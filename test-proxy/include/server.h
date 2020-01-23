#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/epoll.h>

#include "vector.h"
#include "hashmap.h"

/* Server info structure */
struct server
{
	int socket_fd;
	int epoll_fd;
	struct sockaddr_in* socket_address;
};
/* */

int add_servers_from_vector(struct hashmap* dest, const struct vector* connection_strings, int epoll_fd);
int add_server(struct hashmap* dest, const char* connection_string, int epoll_fd);
void reset_server_list(struct hashmap* dest);
void server_packet_received(const struct server* server, unsigned char* packet);
void send_packet_to_server(struct server* dst_server, unsigned char* packet);
int servers_polling(int epoll_fd, const struct hashmap* server_list, struct epoll_event** events);
struct server* get_server_from_fd(const struct hashmap* server_list, int fd);
int get_packet_from_server(const struct server* server, struct iphdr* ip_header, unsigned char* packet);

#endif
