#ifndef __SERVER_H__
#define __SERVER_H__

/* Server info structure */
struct server
{
	char* address;
	unsigned short port;
	int socket_fd;
	struct sockaddr_in socket_address;
};

int add_server(const char* connection_string);
void reset_server_list(void);

#endif
