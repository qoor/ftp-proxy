#ifndef PROXY_INCLUDE_SESSION_H__
#define PROXY_INCLUDE_SESSION_H__

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

struct session
{
	int client_socket[MAX_SESSION_SOCKETS];
	int server_socket[MAX_SESSION_SOCKETS];
};

int add_session_to_list(struct vector* session_list, int socket_fd, int socket_type, int port_type);
int remove_session_from_list(struct vector* session_list, int socket_fd, int socket_type, int port_type);

struct session* get_session_from_list(const struct vector* session_list, int socket_type, int port_type);

#endif
