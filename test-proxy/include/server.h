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
	struct sockaddr_in socket_address;
	struct list list;
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
	SERVER_POLLING_WAIT_ERROR
};
/* */

int server_remove_from_list(struct server* target_server);
int server_register_servers_to_epoll(struct list* session_list, int target_epoll_fd);
int server_polling(int epoll_fd, const struct list* session_list);

#endif

