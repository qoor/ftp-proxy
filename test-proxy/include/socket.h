#ifndef PROXY_INCLUDE_SOCKET_H_
#define PROXY_INCLUDE_SOCKET_H_

#include <sys/types.h>

#define COMMAND_BUFFER_SIZE (4096)
#define DATA_BUFFER_SIZE (32768)

enum socket_error_type
{
	SOCKET_SUCCESS,
	SOCKET_INVALID,
	SOCKET_ALLOC_FAILED,
	SOCKET_OPEN_SOCKET_FAILED
};

struct socket
{
	int socket_fd;
	char* buffer;
	size_t buffer_size;
};

struct socket* socket_create(int domain, int type, int protocol, size_t buffer_size);

#endif

