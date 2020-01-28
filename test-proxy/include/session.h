#ifndef PROXY_INCLUDE_SESSION_H__
#define PROXY_INCLUDE_SESSION_H__

#include "vector.h"

#define MAX_SESSION_SOCKETS (2)

enum socket_type
{
    SOCKET_TYPE_SERVER,
    SOCKET_TYPE_CLIENT
};

enum port_type
{
    PORT_TYPE_COMMAND,
    PORT_TYPE_DATA
};

enum session_error_type
{
	SESSION_SUCCESS,
	SESSION_INVALID_LIST,
	SESSION_ALLOC_FAILED,
	SESSION_INVALID_SOCKET,
	SESSION_INVALID_PARAMS,
	SESSION_INVALID_SESSION
};

struct session
{
	int client_socket[MAX_SESSION_SOCKETS];
	int server_socket[MAX_SESSION_SOCKETS];
};

int add_session_to_list(struct vector* session_list, int socket_fd, int socket_type, int port_type);
int remove_session_from_list(struct vector* session_list, struct session* target_session);

struct session* get_session_from_list(const struct vector* session_list, int socket_fd, int socket_type, int port_type);

#endif
