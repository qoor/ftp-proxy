#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "types.h"
#include "session.h"
#include "packet.h"
#include "log.h"

static int client_command_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		retrun CLIENT_INVALID;
	}

	if (buffer == NULL)
	{
		return CLIENT_INVALID_PARAM;
	}

	/* Command received
	 * TODO: Must implement packet sender function to server
	 * send_packet_to_server(target_session->server_command_socket, buffer, received_bytes, PORT_TYPE_COMMAND);
	*/

	return CLIENT_SUCCESS;
}

static int server_data_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return CLIENT_INVALID;
	}

	if (buffer == NULL)
	{
		return CLIENT_INVALID_PARAM;
	}

	/* File transfer data received
	 * TODO: Must implement packet sender function to server
	 * send_packet_to_server(target_session->server_data_socket, buffer, received_bytes, PORT_TYPE_DATA);
	*/

	return CLIENT_SUCCESS;
}

int client_free(struct client* target_client)
{
	if (target_client == NULL)
	{
		return CLIENT_INVALID;
	}

	if (target_client->command_socket != NULL)
	{
		socket_free(target_client->command_socket);
		target_client->command_socket = NULL;
	}
	if (target_client->data_socket != NULL)
	{
		
	}
}

int send_packet_to_client(struct client* target_client, char* buffer, int received_bytes, int port_type)
{
	int socket_fd = -1;
	int new_buffer_size = received_bytes;

	if (target_client == NULL)
	{
		return CLIENT_INVALID;
	}

	if (buffer == NULL)
	{
		return CLIENT_INVALID_PARAM;
	}

	if (port_type == PORT_TYPE_COMMAND)
	{
		socket_fd = target_client->command_socket->fd;
	}
	else if (port_type == PORT_TYPE_DATA)
	{
		socket_fd = target_client->data_socket->fd;
	}
	else
	{
		return CLIENT_INVALID_PARAM;
	}

	/* Buffer max size is always same as COMMAND_BUFFER_SIZE or DATA_BUFFER_SIZE */
	if ((is_port_command(buffer, received_bytes) == TRUE) && server_listen(target_server) == CLIENT_SUCCESS)
	{
		new_buffer_size = generate_port_command(socket_fd, buffer);
		proxy_error("server", "%s", buffer);
	}

	if (new_buffer_size > 0)
	{
		packet_full_write(socket_fd, buffer, new_buffer_size);
	}

	return CLIENT_SUCCESS;
}