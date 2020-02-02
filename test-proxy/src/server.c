#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "packet.h"
#include "utils.h"
#include "types.h"

static int server_command_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	if (buffer == NULL)
	{
		return SERVER_INVALID_PARAM;
	}

	/* Command received
	 * TODO: Must implement packet sender function to client 
	 * send_packet_to_client(target_session->client_command_socket, buffer, received_bytes, PORT_TYPE_COMMAND);
	*/

	return SERVER_SUCCESS;
}

static int server_data_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	if (buffer == NULL)
	{
		return SERVER_INVALID_PARAM;
	}

	/* File transfer data received
	 * TODO: Must implement packet sender function to client
	 * send_packet_to_client(target_session->client_data_socket, buffer, received_bytes, PORT_TYPE_DATA);
	*/

	return SERVER_SUCCESS;
}

int server_read_packet(struct session* target_session, int port_type)
{
	int received_bytes = 0;
	int socket_fd = -1;
	static char buffer[COMMAND_BUFFER_SIZE] = { 0, };

	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	if (port_type == PORT_TYPE_COMMAND)
	{
		socket_fd = target_session->server->command_socket->fd;
	}
	else if (port_type == PORT_TYPE_DATA)
	{
		socket_fd = target_session->server->data_socket->fd;
	}
	else
	{
		return SERVER_INVALID_PARAM;
	}

	memset(buffer, 0x00, sizeof(buffer));
	received_bytes = packet_full_read(socket_fd, buffer, sizeof(buffer));
	if (received_bytes <= 0)
	{
		/* Socket error */
		if (received_bytes == 0)
		{
			/* If connection closed result is not error */
			return SERVER_CONNECTION_CLOSED;
		}

		return SERVER_CONNECTION_ERROR;
	}

	if (port_type == PORT_TYPE_COMMAND)
	{
		server_command_received(target_session, buffer, received_bytes);
	}
	else
	{
		server_data_received(target_session, buffer, received_bytes);
	}

	return SERVER_SUCCESS;
}

/* Accept connection from FTP server to proxy data port
 * This function return value of  socket file descriptor */
static int server_accept(struct server* target_server, struct sockaddr_in* client_address)
{
	int client_socket = -1;
	static unsigned int client_address_length = sizeof(struct sockaddr);
	static struct epoll_event event = { 0, };

	if (target_server == NULL || client_address == NULL)
	{
		return -1;
	}
	
	client_socket = accept(target_server->data_socket->fd, (struct sockaddr*)client_address, &client_address_length);
			/* If connection accept failed */
	if (client_socket <= 0)
	{
		return -1;
	}

	if (socket_set_nonblock_mode(client_socket) != SOCKET_SUCCESS)
	{
		return -1;
	}

	event.events = EPOLLIN;
	epoll_ctl(target_server->epoll_fd, EPOLL_CTL_ADD, client_socket, &event);
	
	return client_socket;
}

/* Listen FTP data port */
static struct socket* server_listen(struct server* target_server)
{
	struct sockaddr_in bind_address = { 0, };
	struct socket* new_socket = NULL;
	struct epoll_event event = { 0, };
	int ret = 0;

	event.events = EPOLLIN;

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

	ret = epoll_ctl(target_server->epoll_fd, EPOLL_CTL_ADD, new_socket->fd, &event);
	if (ret < 0)
	{
		socket_free(new_socket);
		return NULL;
	}

	target_server->data_socket = new_socket;

	return new_socket;
}

/* Connect to FTP server command port */
static struct socket* server_connect(struct server* target_server)
{
	struct socket* new_socket = NULL;
	struct epoll_event event = { 0, };
	int ret = 0;

	event.events = EPOLLIN;

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
	
	ret = epoll_ctl(target_server->epoll_fd, EPOLL_CTL_ADD, new_socket->fd, &event);
	if (ret < 0)
	{
		socket_free(new_socket);
		return NULL;
	}

	target_server->command_socket = new_socket;

	return new_socket;
}

struct server* server_create_and_insert(struct session* session, const struct sockaddr_in* address)
{
	struct server* new_server = NULL;
	struct socket* new_command_socket = NULL;

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
		free(new_server);
		return NULL;
	}

	new_server->data_socket = NULL;
	new_server->epoll_fd = -1;

	session->server = new_server;

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

	if (target_server->epoll_fd >= 0)
	{
		close(target_server->epoll_fd);
		target_server->epoll_fd = -1;
	}

	free(target_server);

	return SERVER_SUCCESS;
}

int server_loop(struct list* session_list)
{
	struct session* current_session = NULL;
	struct session* temp_session = NULL;

	if (session_list == NULL)
	{
		return SERVER_INVALID;
	}

	list_for_each_entry_safe(current_session, temp_session, session_list, list, struct session*)
	{
		server_session_polling(current_session);
	}

	return SERVER_SUCCESS;
}

int server_session_polling(struct session* target_session)
{
	static struct epoll_event events[MAX_EVENTS] = { { 0, }, };
	int active_event_count = 0;
	int event_id = 0;
	int proxy_command_socket = -1;
	int proxy_data_socket = -1;
	int client_socket = -1;
	struct sockaddr_in client_address = { 0, };
	int epoll_fd = -1;
	struct server* target_server = NULL;
	int ret = 0;

	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	target_server = target_session->server;
	proxy_command_socket = target_server->command_socket->fd;
	proxy_data_socket = target_server->data_socket->fd;
	epoll_fd = target_server->epoll_fd;

	active_event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, EVENT_TIMEOUT);
	if (active_event_count == -1)
	{
		return SERVER_POLLING_WAIT_ERROR;
	}

	for (event_id = 0; event_id < active_event_count; ++event_id)
	{
		client_socket = events[event_id].data.fd;
		if (client_socket == proxy_command_socket)
		{
			ret = server_read_packet(target_session, PORT_TYPE_COMMAND);
			if (ret != SERVER_SUCCESS)
			{
				server_close_client_socket(epoll_fd, proxy_command_socket);
				remove_session(target_session);

				return ret;
			}
		}
		else if (client_socket == proxy_data_socket)
		{
			/* If FTP server request connect to proxy data socket */
			ret = server_accept(target_server, &client_address);
			if (ret <= 0)
			{
				continue;
			}
		}
		else
		{
			/* Connection of FTP server data socket */
			ret = server_read_packet(target_session, PORT_TYPE_DATA);
			if (ret != SERVER_SUCCESS)
			{
				server_close_client_socket(epoll_fd, client_socket);
				remove_session(target_session);
				
				return ret;
			}
		}
	}

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
	if (strncmp(buffer, "PORT", 4) == 0 && strlen(buffer) > 4)
	{
		/* TODO: Must modify new PORT command packet */
		if (is_port_command(buffer, received_bytes) == FALSE)
		{
			new_buffer_size = generate_port_command(socket_fd, buffer);
		}
	}

	if (new_buffer_size > 0)
	{
		packet_full_write(socket_fd, buffer, new_buffer_size);
	}

	return SERVER_SUCCESS;
}

