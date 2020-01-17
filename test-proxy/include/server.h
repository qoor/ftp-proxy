#ifndef __SERVER_H__
#define __SERVER_H__

#define SERVER_ADD_SUCCESS						0
#define SERVER_ADD_ALLOC_FAILED					-1
#define SERVER_ADD_INCORRECT_CONNECTION_STRING	-2
#define SERVER_ADD_SOCKET_CREATE_FAILED			-3

#define SERVER_REMOVE_SUCCESS			0
#define SERVER_REMOVE_INVALID_SERVER	-1

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

