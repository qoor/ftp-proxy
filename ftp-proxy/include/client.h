#ifndef PROXY_INCLUDE_CLIENT_H_
#define PROXY_INCLUDE_CLIENT_H_

#include "socket.h"
#include "session.h"

/* Client info structure */
struct client
{
	struct sockaddr_in address;
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
	CLIENT_INVALID_PARAM,
	CLIENT_PORT_PARSE_FAILED
};

struct client* client_create(struct session* parent_session, int connected_socket);
int client_free(struct client* target_client);
int send_packet_to_client(struct client* target_client, char* buffer, int received_bytes, int port_type);
int client_command_received(struct session* target_session, char* buffer, int received_bytes);
int client_data_received(struct session* target_session, char* buffer, int received_bytes);
struct socket* client_connect(struct client* target_client, struct sockaddr_in* target_address);

#endif
