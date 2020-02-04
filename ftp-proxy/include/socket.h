#ifndef PROXY_INCLUDE_SOCKET_H_
#define PROXY_INCLUDE_SOCKET_H_

#include <sys/types.h>
#include <netinet/in.h>

#define COMMAND_BUFFER_SIZE (4096)
#define DATA_BUFFER_SIZE (32768)

enum socket_error_type
{
	SOCKET_SUCCESS,
	SOCKET_INVALID,
	SOCKET_ALLOC_FAILED,
	SOCKET_OPEN_SOCKET_FAILED,
	SOCKET_FLAG_CONTROL_FAILED,
	SOCKET_READY_FAILED,
	SOCKET_OPTION_FAILED,
	SOCKET_EPOLL_CTL_FAILED
};

struct socket
{
	int fd;
	char* buffer;
	size_t buffer_size;
	struct sockaddr_in address;
};

struct socket* socket_create_by_socket(int socket_fd, size_t buffer_size);
struct socket* socket_create(int domain, int type, int protocol, size_t buffer_size, const struct sockaddr_in* address);
int socket_free(struct socket* target_socket);
int socket_set_nonblock_mode(int socket_fd);
int socket_connect(struct socket* target_socket);
int socket_listen(struct socket* target_socket, int backlog);
int socket_add_to_epoll(int epoll_fd, int socket_fd);

#endif

