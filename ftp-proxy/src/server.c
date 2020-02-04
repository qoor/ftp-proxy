#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "packet.h"
#include "utils.h"
#include "types.h"
#include "log.h"
#include "client.h"
#include "option.h"

int server_command_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	if (buffer == NULL)
	{
		return SERVER_INVALID_PARAM;
	}
	
	send_packet_to_client(target_session->client, buffer, received_bytes, PORT_TYPE_COMMAND);

	return SERVER_SUCCESS;
}

int server_data_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	if (buffer == NULL)
	{
		return SERVER_INVALID_PARAM;
	}
	
	send_packet_to_client(target_session->client, buffer, received_bytes, PORT_TYPE_DATA);

	return SERVER_SUCCESS;
}

int server_data_connection_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	if (buffer == NULL)
	{
		return SERVER_INVALID_PARAM;
	}
	
	send_packet_to_client(target_session->client, buffer, received_bytes, PORT_TYPE_DATA);

	return SERVER_SUCCESS;
}

/* Listen FTP data port */
static struct socket* server_listen(struct server* target_server, uint32_t net_host)
{
	struct sockaddr_in bind_address = { 0, };
	struct socket* new_socket = NULL;
	int ret = 0;
	int reuse_addr = TRUE;

	bind_address.sin_addr.s_addr = net_host;
	bind_address.sin_family = AF_INET;
	bind_address.sin_port = htons(0); /* Port 0 is ANY */
	new_socket = socket_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, DATA_BUFFER_SIZE, &bind_address);
	if (new_socket == NULL)
	{
		return NULL;
	}

	ret = setsockopt(new_socket->fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int));
	if (ret < 0)
	{
		socket_free(new_socket);

		return NULL;
	}

	ret = socket_listen(new_socket, 0);
	if (ret != SERVER_SUCCESS)
	{
		socket_free(new_socket);

		return NULL;
	}

	ret = socket_add_to_epoll(global_option->epoll_fd, new_socket->fd);
	if (ret != SOCKET_SUCCESS)
	{
		socket_free(new_socket);

		return NULL;
	}

	target_server->data_socket = new_socket;

	proxy_error("server", "Listening data port...");

	return new_socket;
}

/* Connect to FTP server command port */
static struct socket* server_connect(struct server* target_server)
{
	struct socket* new_socket = NULL;
	char* server_address = NULL;
	int ret = 0;

	if (target_server == NULL)
	{
		return NULL;
	}

	new_socket = socket_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, COMMAND_BUFFER_SIZE, &target_server->address);
	if (new_socket == NULL)
	{
		return NULL;
	}

	ret = socket_connect(new_socket);
	if (ret != SERVER_SUCCESS)
	{
		socket_free(new_socket);

		return NULL;
	}
	
	target_server->command_socket = new_socket;

	server_address = inet_ntoa(target_server->address.sin_addr);
	proxy_error("server", "Connected to [%s:%d] socket fd: %d", server_address, ntohs(target_server->address.sin_port), new_socket->fd);

	return new_socket;
}

struct server* server_create(const struct sockaddr_in* address)
{
	struct server* new_server = NULL;
	struct socket* new_command_socket = NULL;
	int ret = 0;

	new_server = (struct server*)malloc(sizeof(struct server));
	if (new_server == NULL)
	{
		return NULL;
	}

	new_server->address.sin_addr = address->sin_addr;
	new_server->address.sin_port = address->sin_port;
	new_server->address.sin_family = AF_INET;

	new_command_socket = server_connect(new_server);
	if (new_command_socket == NULL)
	{
		server_free(new_server);

		return NULL;
	}

	new_server->command_socket = new_command_socket;
	ret = socket_add_to_epoll(global_option->epoll_fd, new_command_socket->fd);
	if (ret != SOCKET_SUCCESS)
	{
		server_free(new_server);
		
		return NULL;
	}

	new_server->data_socket = NULL;
	new_server->connection_socket = NULL;

	return new_server;
}

int server_free(struct server* target_server)
{
	if (target_server == NULL)
	{
		return SERVER_INVALID;
	}

	if (target_server->command_socket != NULL)
	{
		socket_free(target_server->command_socket);
		target_server->command_socket = NULL;
	}
	if (target_server->data_socket != NULL)
	{
		socket_free(target_server->data_socket);
		target_server->data_socket = NULL;
	}

	free(target_server);

	return SERVER_SUCCESS;
}

int send_packet_to_server(struct session* target_session, char* buffer, int received_bytes, int port_type)
{
	int socket_fd = -1;
	int new_buffer_size = received_bytes;
	struct server* target_server = NULL;
	struct socket* data_listen_socket = NULL;
	struct socket* target_socket = NULL;

	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	target_server = target_session->server;
	if (target_server == NULL)
	{
		return SERVER_INVALID;
	}

	if (buffer == NULL)
	{
		return SERVER_INVALID_PARAM;
	}
	
	if (port_type == PORT_TYPE_COMMAND)
	{
		target_socket = target_server->command_socket;
	}
	else if (port_type == PORT_TYPE_DATA)
	{
		target_socket = target_server->data_socket;
	}
	else if (port_type == PORT_TYPE_DATA_CONNECTION)
	{
		target_socket = target_server->connection_socket;
	}
	else
	{
		return SERVER_INVALID_PARAM;
	}

	/* Buffer max size is always same as COMMAND_BUFFER_SIZE or DATA_BUFFER_SIZE */
	if (is_port_command(buffer, received_bytes) == TRUE)
	{
		data_listen_socket = server_listen(target_session->server, target_session->host_address.sin_addr.s_addr);
		if (data_listen_socket != NULL)
		{
			new_buffer_size = generate_port_command(data_listen_socket->fd, buffer);
			proxy_error("server", "Proxy data listen port: %s", buffer);
		}
	}

	if (new_buffer_size > 0)
	{
		session_buffer_write(target_socket, buffer, new_buffer_size);
	}

	return SERVER_SUCCESS;
}

/* 
 * Accept connection from FTP server to proxy data port
 * This function return value of  socket file descriptor
*/
struct socket* server_accept(struct server* target_server, struct sockaddr_in* client_address)
{
	int new_socket_fd = -1;
	static unsigned int client_address_length = sizeof(struct sockaddr);
	struct socket* new_socket = NULL;
	char* client_address_str = NULL;
	int ret = 0;

	if (target_server == NULL || client_address == NULL)
	{
		return NULL;
	}
	
	new_socket_fd = accept(target_server->data_socket->fd, (struct sockaddr*)client_address, &client_address_length);
	if ((new_socket_fd < 0) && (errno != EINPROGRESS))
	{
		return NULL;
	}

	new_socket = socket_create_by_socket(new_socket_fd, DATA_BUFFER_SIZE);
	if (new_socket == NULL)
	{
		return NULL;
	}

	ret = socket_add_to_epoll(global_option->epoll_fd, new_socket->fd);
	if (ret != SOCKET_SUCCESS)
	{
		return NULL;
	}

	target_server->connection_socket = new_socket;

	client_address_str = inet_ntoa(client_address->sin_addr);
	proxy_error("server", "Data port connection accepted [Address: %s:%d]", client_address_str, ntohs(client_address->sin_port));

	return new_socket;
}

int server_insert_address(struct list* server_list, char* address)
{
	char* colon_pos = NULL;
	char host[32] = { 0, };
	size_t address_length = 0;
	uint32_t address_net = 0;
	uint16_t port = 0;
	struct server_address* server_address = NULL;

	if ((server_list == NULL) || (address == NULL))
	{
		return SERVER_INVALID_PARAM;
	}

	colon_pos = strchr(address, ':');
	if ((colon_pos == NULL) || (*(colon_pos + 1) == '\0'))
	{
		return SERVER_INVALID_PARAM;
	}

	port = atoi(colon_pos + 1);

	address_length = colon_pos - address;
	strncpy(host, address, address_length);
	host[address_length] = '\0';
	address_net = inet_addr(host);
	if (address_net == INADDR_NONE)
	{
		return SERVER_INVALID_PARAM;
	}

	server_address = (struct server_address*)malloc(sizeof(struct server_address));
	if (server_address == NULL)
	{
		return SERVER_ALLOC_FAILED;
	}

	memset(&server_address->address, 0x00, sizeof(struct sockaddr_in));
	server_address->address.sin_addr.s_addr = address_net;
	server_address->address.sin_port = htons(port);
	server_address->address.sin_family = AF_INET;

	LIST_INIT(&server_address->list);
	LIST_ADD(server_list, &server_address->list);

	proxy_error("server", "Address [%s:%d] listed", host, port);

	return SERVER_SUCCESS;
}

struct sockaddr_in* server_get_available_address(struct list* server_list)
{
	struct server_address* current_address = NULL;

	if (server_list == NULL)
	{
		return NULL;
	}

	current_address = LIST_ELEM(server_list->n, struct server_address*, list);

	return &current_address->address;
}

int server_data_closed(struct server* target_server)
{
	if (target_server == NULL)
	{
		return SERVER_INVALID;
	}

	if (target_server->connection_socket != NULL)
	{
		socket_free(target_server->connection_socket);
		target_server->connection_socket = NULL;
	}

	if (target_server->data_socket != NULL)
	{
		socket_free(target_server->connection_socket);
		target_server->connection_socket = NULL;
	}
	return SERVER_SUCCESS;
}
