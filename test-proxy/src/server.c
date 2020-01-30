#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static int server_free(struct server* target_server)
{
	if (target_server == NULL)
	{
		return SERVER_INVALID;
	}
	free(target_server);

	return SERVER_SUCCESS;
}

static struct server* server_create()
{
	struct server* new_server = NULL;

	new_server = (struct server*)malloc(sizeof(struct server));
	if (new_server == NULL)
	{
		return NULL;
	}

	return new_server;
}

static int session_listen(struct socket* socket, uint16_t port)
{
	struct sockaddr_in bind_socket = { 0, };
	int ret = 0;
	int socket_fd = -1;

	if (socket == NULL)
	{
		return SERVER_INVALID;
	}

	socket_fd = socket->socket_fd;

	bind_socket.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_socket.sin_family = AF_INET;
	bind_socket.sin_port = htons(port);
	ret = bind(socket_fd, (struct sockaddr*)&bind_socket, sizeof(struct sockaddr));
	if (ret == -1)
	{
		return SERVER_SOCKET_CREATE_FAILED;
	}

	ret = listen(socket_fd, SOMAXCONN);
	if (ret == -1)
	{
		return SERVER_SOCKET_CREATE_FAILED;
	}

	return SERVER_SUCCESS;
}

/* Connect to server and insert connection socket to session */
static struct socket* server_connect(struct session* session, const struct server* server)
{
	int ret = 0;
	struct socket* new_socket = NULL;

	new_socket = socket_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, COMMAND_BUFFER_SIZE, &server->socket_address);
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

	session->server_command_socket = new_socket;

	return new_socket;
}

int server_remove_from_list(struct server* target_server)
{
	if (target_server == NULL)
	{
		return SERVER_INVALID;
	}

	LIST_DEL(target_server->list);	

	return SERVER_SUCCESS;
}

int server_register_servers_to_epoll(struct list* session_list, int target_epoll_fd)
{
	struct socket* server_socket = NULL;
	struct epoll_event event = { 0, };

	event.events = EPOLLIN;

	if (session_list == NULL || target_epoll_fd == -1)
	{
		return SERVER_INVALID;
	}

	list_for_each_entry(server_socket, session_list, list)
	{
		epoll_ctl(target_epoll_fd, EPOLL_CTL_ADD, server_socket->socket_fd, &event);	
	}

	return SERVER_SUCCESS;
}

int server_polling(int epoll_fd, const struct list* session_list)
{
	int active_event_count = 0;
	static struct epoll_event events[MAX_EVENTS] = { { 0, }, };
	int event_id = 0;
	int server_socket = -1;
	int client_socket = -1;
	struct sockaddr_in client_address = { 0, };
	const size_t client_address_length = sizeof(struct sockaddr);

	if (epoll_fd == -1)
	{
		return SERVER_INVALID;
	}

	active_event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, EVENT_TIMEOUT);
	if (active_event_count == -1)
	{
		return SERVER_POLLING_WAIT_ERROR;
	}

	for (event_id = 0; event_id < active_event_count; ++event_id)
	{
		client_socket = events[event_id].data.fd;
		if (client_socket == server_socket)
		{
			/* If session request connecting */
			client_socket = accept(server_socket, (struct sockaddr*)&client_address, (socklen_t*)&client_address_length);
		}
		else
		{
			
		}
	}

	return SERVER_SUCCESS;
}

