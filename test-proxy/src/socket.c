#include "socket.h"

#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/epoll.h>

#include "types.h"
#include "log.h"

#define DEFAULT_BUFFER_SIZE BUFSIZ

static int socket_set_buffer_size(struct socket* target_socket, size_t buffer_size)
{
	int ret = 0;

	if (target_socket == NULL)
	{
		return SOCKET_INVALID;
	}

	if (buffer_size <= 0)
	{
		buffer_size = DEFAULT_BUFFER_SIZE;
	}

	ret = setsockopt(target_socket->fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(size_t));
	if (ret < 0)
	{
		return SOCKET_OPTION_FAILED;
	}

	ret = setsockopt(target_socket->fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(size_t));
	if (ret < 0)
	{
		return SOCKET_OPTION_FAILED;
	}

	return SOCKET_SUCCESS;
}

struct socket* socket_create_by_socket(int domain, int type, int protocol, size_t buffer_size, int socket_fd)
{
	struct socket* new_socket = NULL;
	struct sockaddr_in address = { 0, };
	uint32_t address_length = sizeof(struct sockaddr_in);
	int ret = 0;

	if (buffer_size <= 0)
	{
		buffer_size = DEFAULT_BUFFER_SIZE;
	}

	if (socket_fd == -1)
	{
		return NULL;
	}

	new_socket = (struct socket*)malloc(sizeof(struct socket));
	if (new_socket == NULL)
	{
		return NULL;
	}

	memset(new_socket, 0x00, sizeof(struct socket));
	new_socket->buffer = (char*)malloc(buffer_size);
	if (new_socket->buffer == NULL)
	{
		free(new_socket);
		
		return NULL;
	}

	new_socket->buffer_size = buffer_size;

	new_socket->fd = socket_fd;
	ret = socket_set_buffer_size(new_socket, buffer_size);
	if (ret != SOCKET_SUCCESS)
	{
		socket_free(new_socket);

		return NULL;
	}

	getsockname(socket_fd, (struct sockaddr*)&address, &address_length);
	new_socket->address.sin_addr = address.sin_addr;
	new_socket->address.sin_port = address.sin_port;
	new_socket->address.sin_family = AF_INET;

	return new_socket;
}

struct socket* socket_create(int domain, int type, int protocol, size_t buffer_size, const struct sockaddr_in* address)
{
	struct socket* new_socket = NULL;
	int ret = 0;

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

	memset(new_socket, 0x00, sizeof(struct socket));
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

	ret = socket_set_buffer_size(new_socket, buffer_size);
	if (ret != SOCKET_SUCCESS)
	{
		socket_free(new_socket);
		errno = SOCKET_OPTION_FAILED;

		return NULL;
	}

	new_socket->address.sin_addr = address->sin_addr;
	new_socket->address.sin_family = address->sin_family;
	new_socket->address.sin_port = address->sin_port;

	socket_set_nonblock_mode(new_socket->fd);

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
	}

	free(target_socket);

	return SOCKET_SUCCESS;
}

int socket_set_nonblock_mode(int socket_fd)
{
	/*int flags = 0;
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
	}*/

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

	proxy_error("socket", "Socket fd: %d, address: [%d:%d]", target_socket->fd, target_socket->address.sin_addr.s_addr, ntohs(target_socket->address.sin_port));
	if ((ret < 0) && (errno != EINPROGRESS))
	{
		DBX();
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

	ret = bind(target_socket->fd, (struct sockaddr*)&target_socket->address, sizeof(struct sockaddr));
	if (ret < 0)
	{
		return SOCKET_READY_FAILED;
	}

	ret = listen(target_socket->fd, backlog);
	if (ret < 0)
	{
		return SOCKET_READY_FAILED;
	}

	return SOCKET_SUCCESS;
}

int socket_add_to_epoll(int epoll_fd, int socket_fd)
{
	struct epoll_event event = { 0, };
	int ret = 0;

	event.events = EPOLLIN;

	if ((epoll_fd == -1) || (socket_fd == -1))
	{
		return SOCKET_INVALID;
	}

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
	if (ret < 0)
	{
		return SOCKET_EPOLL_CTL_FAILED;
	}

	return SOCKET_SUCCESS;
}

