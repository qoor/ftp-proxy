#ifndef PROXY_INCLUDE_CLIENT_H_
#define PROXY_INCLUDE_CLIENT_H_

#include <sys/epoll.h>
#include <netinet/ip.h>

#include <vector.h>
#include <hashmap.h>

#define MAX_CLIENT_EVENTS 254
#define BIND_CLIENT_PORT 7777

/* Client info structure */
struct client
{
	struct sockaddr_in* client_address;
	int socket_fd;
};

#endif
