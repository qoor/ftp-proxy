#ifndef PROXY_INCLUDE_SERVER_H__
#define PROXY_INCLUDE_SERVER_H__

#include <sys/epoll.h>
#include <netinet/ip.h>

#include "vector.h"
#include "session.h"
#include "socket.h"
#include "list.h"

#define MAX_EVENTS (254) /* Amount of file descriptor of monitoring */
#define EVENT_TIMEOUT (0) /* EPOLL event timeout as milliseconds */

#define FTP_COMMAND_PORT (21)
#define FTP_DATA_PORT (20)

/* Server info structure */
struct server
{
	struct sockaddr_in address;
	struct socket* command_socket;
	struct socket* data_socket;
	int epoll_fd;
};
/* */

/* SERVER RETURN CODE DEFINE */
enum server_error_type
{
	SERVER_SUCCESS,
	SERVER_INVALID,
	SERVER_ALLOC_FAILED,
	SERVER_INCORRECT_CONNECTION_STRING,
	SERVER_SOCKET_CREATE_FAILED,
	SERVER_NO_SERVERS,
	SERVER_POLLING_WAIT_ERROR,
	SERVER_CONNECTION_CLOSED,
	SERVER_CONNECTION_ERROR,
	SERVER_INVALID_PARAM
};
/* */

struct server* server_create(const struct sockaddr_in* address);
int server_free(struct server* target_server);
int server_loop(struct list* session_list);
int server_session_polling(struct session* target_session);
int send_packet_to_server(struct server* target_server, char* buffer, int received_bytes, int port_type);
int server_read_packet(struct session* target_session, int port_type);

#endif

