#ifndef PROXY_INCLUDE_SERVER_H__
#define PROXY_INCLUDE_SERVER_H__

#include <sys/epoll.h>
#include <netinet/ip.h>

#include "vector.h"
#include "hashmap.h"

#define MAX_EVENTS (254) /* Amount of file descriptor of monitoring */
#define EVENT_TIMEOUT (0) /* EPOLL event timeout as milliseconds */

#define FTP_COMMAND_PORT (21)
#define FTP_DATA_PORT (20)

/* Server info structure */
struct server
{
	int socket_fd;
	int epoll_fd;
	struct sockaddr_in* socket_address;
};
/* */

/* SERVER RETURN CODE DEFINE */
enum server_add_error_type
{
	SERVER_ADD_SUCCESS,
	SERVER_ADD_ALLOC_FAILED,
	SERVER_ADD_INCORRECT_CONNECTION_STRING,
	SERVER_ADD_SOCKET_CREATE_FAILED,
	SERVER_ADD_NO_SERVERS
};

enum server_remove_error_type
{
	SERVER_REMOVE_SUCCESS,
	SERVER_REMOVE_INVALID_SERVER
};

enum servers_polling_error_type
{
	SERVERS_POLLING_SUCCESS,
	SERVERS_POLLING_WAIT_ERROR
};
/* */

int add_servers_from_vector(struct hashmap* dest, const struct vector* connection_strings, int epoll_fd);
int add_server(struct hashmap* dest, const char* connection_string, int epoll_fd);
void reset_server_list(struct hashmap* dest);
void server_packet_received(const struct server* server, unsigned char* packet);
void send_packet_to_server(struct server* dst_server, unsigned char* packet);
int servers_polling(int epoll_fd, const struct hashmap* server_list, struct epoll_event** events);
struct server* get_server_from_fd(const struct hashmap* server_list, int fd);
int get_packet_from_server(const struct server* server, struct iphdr* ip_header, unsigned char** packet);

#endif

