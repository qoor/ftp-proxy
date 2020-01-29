#ifndef PROXY_INCLUDE_SERVER_H__
#define PROXY_INCLUDE_SERVER_H__

#include <sys/epoll.h>
#include <netinet/ip.h>

#include "vector.h"
#include "session.h"

#define MAX_EVENTS (254) /* Amount of file descriptor of monitoring */
#define EVENT_TIMEOUT (0) /* EPOLL event timeout as milliseconds */

#define FTP_COMMAND_PORT (21)
#define FTP_DATA_PORT (20)

/* Server info structure */
struct server
{
	int socket[MAX_SESSION_SOCKETS];
	struct sockaddr_in socket_address;
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

int server_remove_from_list(struct vector* server_list, const struct server* target_server);
int server_remove_from_list_index(struct vector* server_list, int index);
int server_register_servers_to_epoll(struct vector* server_list, int target_epoll_fd);

#endif

