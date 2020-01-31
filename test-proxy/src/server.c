#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "packet.h"

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

	free(target_server);

	return SERVER_SUCCESS;
}

int server_polling(struct session* target_session)
{
	static struct epoll_event events[MAX_EVENTS] = { { 0, }, };
	static char command_buffer[COMMAND_BUFFER_SIZE];
	static char data_buffer[DATA_BUFFER_SIZE];
	static struct epoll_event event = { 0, };
	int active_event_count = 0;
	int event_id = 0;
	int proxy_command_socket = -1;
	int proxy_data_socket = -1;
	int client_socket = -1;
	struct sockaddr_in client_address = { 0, };
	const size_t client_address_length = sizeof(struct sockaddr);
	int epoll_fd = -1;
	ssize_t received_bytes = 0;
	struct server* target_server = NULL;

	if (target_session == NULL)
	{
		return SERVER_INVALID;
	}

	memset(&event, 0x00, sizeof(struct epoll_event));

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
			received_bytes = packet_full_read(proxy_command_socket, command_buffer, sizeof(command_buffer));
			if (received_bytes <= 0)
			{
				/* Socket error */
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, proxy_command_socket, &event);
				socket_free(target_server->command_socket);
				target_server->command_socket = NULL;
				continue;
			}

			if (strncmp(command_buffer, "PORT", 4) == 0 && strlen(command_buffer) > 4)
			{
				/* TODO: Must modify new PORT command packet */
				command_buffer[4] = '\0';
				packet_full_write(proxy_command_socket, command_buffer, 5);
				continue;
			}

			/* Command received
			 * TODO: Must implement packet sender function to client 
			 * send_apcket_to_client(target_session->client_command_socket, buffer, received_bytes);
			*/
		}
		else if (client_socket == proxy_data_socket)
		{
			/* If session request connecting */
			client_socket = accept(proxy_data_socket, (struct sockaddr*)&client_address, (socklen_t*)&client_address_length);
			/* If connection accept failed */
			if (client_socket <= 0)
			{
				continue;
			}

			if (socket_set_nonblock_mode(client_socket) != SOCKET_SUCCESS)
			{
				shutdown(client_socket, SHUT_RDWR);
				close(client_socket);
				continue;
			}

			epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event);
		}
		else
		{
			/* Connection of FTP server data socket */
			received_bytes = packet_full_read(client_socket, data_buffer, sizeof(data_buffer));
			if (received_bytes <= 0)
			{
				/* Socket error */
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, &event);
				socket_free(target_server->data_socket);
				target_server->data_socket = NULL;
				continue;
			}
			/* Command received
			 * TODO: Must implement packet sender function to client 
			 * send_apcket_to_client(target_session->client_command_socket, buffer, received_bytes);
			*/
		}
	}

	return SERVER_SUCCESS;
}

