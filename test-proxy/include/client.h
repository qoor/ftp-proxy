#ifndef PROXY_INCLUDE_CLIENT_H_
#define PROXY_INCLUDE_CLIENT_H_

#include "socket.h"

/* Client info structure */
struct client
{
	struct socket* command_socket;
	struct socket* data_socket;
};

/* CLIENT RETURN CODE DEFINE */
enum client_error_type
{
	CLIENT_SUCCESS,
	CLIENT_INVALID,
	CLIENT_ALLOC_FAILED,
	CLIENT_INCORRECT_CONNECTION_STRING,
	CLIENT_SOCKET_CREATE_FAILED,
	CLIENT_NO_CLIENTS,
	CLIENT_POLLING_WAIT_ERROR,
	CLIENT_CONNECTION_CLOSED,
	CLIENT_CONNECTION_ERROR,
	CLIENT_INVALID_PARAM
};

int send_packet_to_client(struct client* target_client, char* buffer, int received_bytes, int port_type);

#endif
