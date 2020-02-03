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

/* Listen FTP data port */
static struct socket* server_listen(struct server* target_server)
{
	struct sockaddr_in bind_address = { 0, };
	struct socket* new_socket = NULL;
	int ret = 0;

	if (target_server == NULL)
	{
		return NULL;
	}

	bind_address.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_address.sin_family = AF_INET;
	bind_address.sin_port = htons(0); /* Port 0 is ANY */
	new_socket = socket_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, DATA_BUFFER_SIZE, &bind_address);
	if (new_socket == NULL)
	{
		return NULL;
	}

	ret = socket_listen(new_socket, 0);
	if (ret < 0)
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
	proxy_error("server", "Connecting to [%s:%d]...", server_address, ntohs(target_server->address.sin_port));
	free(server_address);

	return new_socket;
}

struct server* server_create(const struct sockaddr_in* address)
{
	struct server* new_server = NULL;
	struct socket* new_command_socket = NULL;
	struct socket* new_data_socket = NULL;

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

	new_data_socket = server_listen(new_server);
	if (new_data_socket == NULL)
	{
		server_free(new_server);

		return NULL;
	}

	new_server->command_socket = new_command_socket;
	new_server->data_socket = new_data_socket;

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

int send_packet_to_server(struct server* target_server, char* buffer, int received_bytes, int port_type)
{
	int socket_fd = -1;
	int new_buffer_size = received_bytes;

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
		socket_fd = target_server->command_socket->fd;
	}
	else if (port_type == PORT_TYPE_DATA)
	{
		socket_fd = target_server->data_socket->fd;
	}
	else
	{
		return SERVER_INVALID_PARAM;
	}

	/* Buffer max size is always same as COMMAND_BUFFER_SIZE or DATA_BUFFER_SIZE */
	if ((is_port_command(buffer, received_bytes) == TRUE) && server_listen(target_server) == SERVER_SUCCESS)
	{
		new_buffer_size = generate_port_command(socket_fd, buffer);
		proxy_error("server", "%s", buffer);
	}

	if (new_buffer_size > 0)
	{
		packet_full_write(socket_fd, buffer, new_buffer_size);
	}

	return SERVER_SUCCESS;
}

/* 
 * Accept connection from FTP server to proxy data port
 * This function return value of  socket file descriptor
*/
int server_accept(struct server* target_server, struct sockaddr_in* client_address)
{
	int client_socket = -1;
	static unsigned int client_address_length = sizeof(struct sockaddr);
	char* client_address_str = NULL;

	if (target_server == NULL || client_address == NULL)
	{
		return -1;
	}
	
	client_socket = accept(target_server->data_socket->fd, (struct sockaddr*)client_address, &client_address_length);
	if (client_socket <= 0)
	{
		return -1;
	}

	if (socket_set_nonblock_mode(client_socket) != SOCKET_SUCCESS)
	{
		return -1;
	}

	client_address_str = inet_ntoa(client_address->sin_addr);
	proxy_error("server", "Data port connection accepted [Address: %s:%d]", client_address_str, ntohs(client_address->sin_port));
	free(client_address_str);

	return client_socket;
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

	current_address = LIST_ELEM(server_list, struct server_address*, list);

	return &current_address->address;
}

