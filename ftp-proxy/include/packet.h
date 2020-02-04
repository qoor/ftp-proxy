#ifndef PROXY_INCLUDE_PACKET_H_
#define PROXY_INCLUDE_PACKET_H_

#include <sys/types.h>

enum packet_error_type
{
	PACKET_SUCCESS,
	PACKET_INVALID_SOCKET,
	PACKET_INVALID_BUFFER,
	PACKET_INVALID_BUFFER_SIZE
};

int packet_read(int fd, void* buffer, size_t size);
int packet_full_read(int socket_fd, void* buffer, size_t size);
int packet_write(int socket_fd, const void* buffer, size_t size);
int packet_full_write(int socket_fd, const void* buffer, size_t size);

#endif

