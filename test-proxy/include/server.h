#ifndef PROXY_INCLUDE_SERVER_H__
#define PROXY_INCLUDE_SERVER_H__

#include <sys/epoll.h>
#include <netinet/ip.h>

#include "session.h"
#include "socket.h"
#include "list.h"


/* Server addresses for server list from option */
struct server_address
{
	struct sockaddr_in address;
	struct list list;
};
/* */

/* Server info structure */
struct server
{
	struct sockaddr_in address;
	struct socket* command_socket;
	struct socket* data_socket;
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
int send_packet_to_server(struct session* target_session, char* buffer, int received_bytes, int port_type);
int server_accept(struct server* target_server, struct sockaddr_in* client_address);
int server_insert_address(struct list* server_list, char* address);
struct sockaddr_in* server_get_available_address(struct list* server_list);
int server_command_received(struct session* target_session, char* buffer, int received_bytes);
int server_data_received(struct session* target_session, char* buffer, int received_bytes);

#endif

