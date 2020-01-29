#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static void server_set_socket_option(int socket_fd)
{
	int flags = 0;

	flags = fcntl(socket_fd, F_GETFL);
	fcntl(socket_fd, F_SETFL, flags & O_NONBLOCK);
}

static int server_free(struct server* target_server)
{
	int socket_fd = -1;
	int i = 0;

	if (target_server == NULL)
	{
		return SERVER_INVALID;
	}

	for ( ; i < MAX_SESSION_SOCKETS; ++i)
	{
		socket_fd = target_server->socket[i];
		if (socket_fd != -1)
		{
			shutdown(socket_fd, SHUT_RDWR);
			close(socket_fd);
		}
	}

	free(target_server);

	return SERVER_SUCCESS;
}

static struct server* server_create()
{
	struct server* new_server = NULL;
	int new_socket = -1;
	int i = 0;

	new_server = (struct server*)malloc(sizeof(struct server));
	if (new_server == NULL)
	{
		return NULL;
	}

	/* Must reset socket before `server_free` */
	for (i = 0; i < MAX_SESSION_SOCKETS; ++i)
	{
		new_server->socket[i] = -1;
	}

	for (i = 0; i < MAX_SESSION_SOCKETS; ++i)
	{
		new_server->socket[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		new_socket = new_server->socket[i];
		if (new_socket < 0)
		{
			server_free(new_server);
			return NULL;
		}	
	}

	return new_server;
}

static int server_connect(int socket_fd, uint32_t address, uint16_t port)
{
	struct sockaddr_in socket_address = { 0, };
	int ret = 0;

	socket_address.sin_addr.s_addr = htonl(address);
	socket_address.sin_port = htons(port);
	socket_address.sin_family = AF_INET;

	server_set_socket_option(socket_fd);
	ret = connect(socket_fd, (struct sockaddr*)&socket_address, sizeof(struct sockaddr));
	if (ret < 0 && errno != EINPROGRESS)
	{
		return SERVER_SOCKET_CREATE_FAILED;
	}

	return SERVER_SUCCESS;
}

static int server_listen(int socket_fd, uint16_t port)
{
	struct sockaddr_in bind_socket = { 0, };
	int ret = 0;

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

static int server_add_to_list(struct vector* server_list, uint32_t address, struct server** created_server)
{
	struct server* new_server = NULL;
	int ret = -1;

	errno = 0;

	if (created_server == NULL)
	{
		errno = SERVER_INVALID;
		return -1;
	}

	*created_server = NULL;

	if (server_list == NULL)
	{
		errno = SERVER_INVALID;
		return -1;
	}

	new_server = server_create();
	if (new_server == NULL)
	{
		errno = SERVER_ALLOC_FAILED;
		return -1;
	}

	ret = server_connect(new_server->socket[PORT_TYPE_COMMAND], address, FTP_COMMAND_PORT);
	if (ret != SERVER_SUCCESS)
	{
		server_free(new_server);
		errno = SERVER_SOCKET_CREATE_FAILED;
		return -1;
	}

	ret = server_listen(new_server->socket[PORT_TYPE_DATA], FTP_DATA_PORT);
	if (ret != SERVER_SUCCESS)
	{
		server_free(new_server);
		errno = SERVER_SOCKET_CREATE_FAILED;
		return -1;
	}

	if (vector_push_back(server_list, new_server) != VECTOR_SUCCESS)
	{
		server_free(new_server);
		errno = SERVER_ALLOC_FAILED;
		return -1;
	}

	*created_server = new_server;

	return (server_list->size - 1);
}

int server_remove_from_list(struct vector* server_list, const struct server* target_server)
{
	int i = 0;

	if (server_list == NULL || target_server == NULL)
	{
		return SERVER_INVALID;
	}

	for ( ; i < server_list->size; ++i)
	{
		if (memcmp(target_server, server_list->container[i], sizeof(struct server)) == 0)
		{
			return server_remove_from_list_index(server_list, i);
		}
	}

	return SERVER_INVALID;
}

int server_remove_from_list_index(struct vector* server_list, int index)
{
	if (server_list == NULL || index < 0 || server_list->size >= index)
	{
		return SERVER_INVALID;
	}

	server_free((struct server*)server_list->container[index]);
	vector_erase(server_list, index);

	return SERVER_SUCCESS;
}

