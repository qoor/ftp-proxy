#ifndef PROXY_INCLUDE_SESSION_H__
#define PROXY_INCLUDE_SESSION_H__

#include <sys/epoll.h>

#include "list.h"
#include "socket.h"

#define MAX_EVENTS (256) /* Max amount of event until while epoll_wait once */
#define EVENT_TIMEOUT (0) /* EPOLL event timeout as milliseconds */

#define FTP_COMMAND_PORT (7676)

enum packet_from_type
{
    FROM_SERVER,
	FROM_CLIENT
};

enum port_type
{
    PORT_TYPE_COMMAND,
    PORT_TYPE_DATA,
	PORT_TYPE_DATA_CONNECTION
};

enum session_error_type
{
	SESSION_SUCCESS,
	SESSION_INVALID_LIST,
	SESSION_ALLOC_FAILED,
	SESSION_INVALID_SOCKET,
	SESSION_INVALID_PARAMS,
	SESSION_INVALID_SESSION,
	SESSION_EPOLL_CTL_FAILED,
	SESSION_ADD_FAILED,
	SESSION_POLLING_WAIT_ERROR,
	SESSION_CONNECTION_CLOSED,
	SESSION_CONNECTION_ERROR
};

struct session
{
	struct client* client;
	struct server* server;
	struct list list;
	struct sockaddr_in host_address; /* For alias to outside my IP */
};

int session_remove_from_list(struct session* target_session);
struct session* get_session_from_list(const struct list* session_list, int socket_fd);
int session_polling(int epoll_fd, struct list* session_list, int proxy_connect_socket, struct epoll_event* events);
int session_read_packet(struct session* target_session, int event_socket);
int session_buffer_write(struct socket* target_socket, char* buffer, size_t buffer_size);

#endif

