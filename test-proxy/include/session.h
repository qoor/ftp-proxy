#ifndef PROXY_INCLUDE_SESSION_H__
#define PROXY_INCLUDE_SESSION_H__

#include "list.h"
#include "socket.h"

#define MAX_EVENTS (256) /* Max amount of event until while epoll_wait once */
#define EVENT_TIMEOUT (0) /* EPOLL event timeout as milliseconds */

#define FTP_COMMAND_PORT (21)

enum packet_from_type
{
    FROM_SERVER,
	FROM_CLIENT
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
	SESSION_INVALID_SESSION,
	SESSION_EPOLL_CTL_FAILED,
	SESSION_ADD_FAILED
};

struct session
{
	struct client* client;
	struct server* server;
	struct list list;
};

int session_remove_from_list(struct session* target_session);
struct session* get_session_from_list(const struct list* session_list, int socket_fd);
int session_polling(int epoll_fd, struct list* session_list, int proxy_connect_socket);
int session_read_packet(struct session* target_session, int from, int port_type);

#endif

