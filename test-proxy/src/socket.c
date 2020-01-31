#include "socket.h"

#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>

#define DEFAULT_BUFFER_SIZE BUFSIZ

struct socket* socket_create(int domain, int type, int protocol, size_t buffer_size, const struct sockaddr_in* address)
{
	struct socket* new_socket = NULL;

	errno = 0;
	
	if (buffer_size <= 0)
	{
		buffer_size = DEFAULT_BUFFER_SIZE;
	}

	if (address == NULL)
	{
		errno = SOCKET_INVALID;
		return NULL;
	}

	new_socket = (struct socket*)malloc(sizeof(struct socket));
	if (new_socket == NULL)
	{
		errno = SOCKET_ALLOC_FAILED;
		return NULL;
	}

	memset(&new_socket->address, 0x00, sizeof(struct sockaddr_in));
	new_socket->buffer = (char*)malloc(buffer_size);
	if (new_socket->buffer == NULL)
	{
		free(new_socket);
		errno = SOCKET_ALLOC_FAILED;
		return NULL;
	}

	new_socket->buffer_size = buffer_size;

	new_socket->fd = socket(domain, type, protocol);
	if (new_socket->fd < 0)
	{
		socket_free(new_socket);
		errno = SOCKET_OPEN_SOCKET_FAILED;
		return NULL;
	}

	new_socket->address.sin_addr = address->sin_addr;
	new_socket->address.sin_family = address->sin_family;
	new_socket->address.sin_port = address->sin_port;

	return new_socket;
}

int socket_free(struct socket* target_socket)
{
	if (target_socket == NULL)
	{
		return SOCKET_INVALID;
	}

	if (target_socket->buffer != NULL)
	{
		free(target_socket->buffer);
		target_socket->buffer = NULL;
	}

	if (target_socket->fd >= 0)
	{
		shutdown(target_socket->fd, SHUT_RDWR);
		close(target_socket->fd);
		free(target_socket);
	}

	return SOCKET_SUCCESS;
}

int socket_set_nonblock_mode(int socket_fd)
{
	int flags = 0;
	int ret = 0;

	if (socket_fd == -1)
	{
		return SOCKET_INVALID;
	}

	flags = fcntl(socket_fd, F_GETFL);
	if (flags < 0)
	{
		return SOCKET_FLAG_CONTROL_FAILED;
	}

	ret = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0)
	{
		return SOCKET_FLAG_CONTROL_FAILED;
	}

	return SOCKET_SUCCESS;
}

int socket_connect(struct socket* target_socket)
{
	int ret = 0;

	if (target_socket == NULL)
	{
		return SOCKET_INVALID;
	}

	ret = connect(target_socket->fd, (struct sockaddr*)&target_socket->address, sizeof(struct sockaddr));
	if ((ret < 0) && (errno != EINPROGRESS))
	{
		return SOCKET_READY_FAILED;
	}

	return SOCKET_SUCCESS;
}

int socket_listen(struct socket* target_socket, int backlog)
{
	int ret = 0;

	if (target_socket == NULL)
	{
		return SOCKET_INVALID;
	}

	if (backlog < 0)
	{
		backlog = 0;
	}

	ret = listen(target_socket->fd, backlog);
	if (ret < 0)
	{
		return SOCKET_READY_FAILED;
	}

	return SOCKET_SUCCESS;
}

