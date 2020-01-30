#include "socket.h"

#include <malloc.h>
#include <errno.h>

#include <sys/socket.h>

#define DEFAULT_BUFFER_SIZE BUFSIZ

struct socket* socket_create(int domain, int type, int protocol, size_t buffer_size)
{
	struct socket* new_socket = NULL;

	errno = 0;
	
	if (buffer_size <= 0)
	{
		buffer_size = DEFAULT_BUFFER_SIZE;
	}

	new_socket = (struct socket*)malloc(sizeof(struct socket));
	if (new_socket == NULL)
	{
		errno = SOCKET_ALLOC_FAILED;
		return NULL;
	}

	new_socket->buffer = (char*)malloc(buffer_size);
	if (new_socket->buffer == NULL)
	{
		free(new_socket);
		errno = SOCKET_ALLOC_FAILED;
		return NULL;
	}

	new_socket->buffer_size = buffer_size;

	new_socket->socket_fd = socket(domain, type, protocol);
	if (new_socket->socket_fd < 0)
	{
		free(new_socket->buffer);
		free(new_socket);
		errno = SOCKET_OPEN_SOCKET_FAILED;
		return NULL;
	}

	return new_socket;
}
