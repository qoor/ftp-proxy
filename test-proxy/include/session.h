#ifndef PROXY_INCLUDE_SESSION_H__
#define PROXY_INCLUDE_SESSION_H__

#include "list.h"
#include "socket.h"

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
	SESSION_ADD_SUCCESS,
	SESSION_INVALID_LIST,
	SESSION_ALLOC_FAILED,
	SESSION_INVALID_SOCKET,
	SESSION_INVALID_PARAMS,
	SESSION_INVALID_SESSION
};

struct session
{
	struct socket* client_command_socket;
	struct socket* client_data_socket;
	struct socket* server_command_socket;
	struct socket* server_data_socket;
	struct list list;
};

int add_session_to_list(struct list* session_list, int socket_fd, int socket_type, int port_type);
int remove_session_from_list(struct list* session_list, struct session* target_session);

struct session* get_session_from_list(const struct list* session_list, int socket_fd, int socket_type, int port_type);

#endif

