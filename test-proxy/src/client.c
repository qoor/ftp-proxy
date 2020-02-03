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
#include "packet.h"
#include "log.h"
#include "utils.h"
#include "server.h"

/* Connect to FTP client data port */
static struct socket* client_connect(struct client* target_client, struct sockaddr_in* target_address)
{
	struct socket* new_socket = NULL;
	char* client_address = NULL;
	int ret = 0;

	if (target_client == NULL)
	{
		return NULL;
	}

	new_socket = socket_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, DATA_BUFFER_SIZE, target_address);
	if (new_socket == NULL)
	{
		return NULL;
	}

	ret = socket_connect(new_socket);
	if (ret != CLIENT_SUCCESS)
	{
		socket_free(new_socket);
		return NULL;
	}
	
	target_client->data_socket = new_socket;

	client_address = inet_ntoa(target_address->sin_addr);
	proxy_error("client", "Connecting to [%s:%d]...", client_address, ntohs(target_address->sin_port));
	free(client_address);

	return new_socket;
}

int client_command_received(struct session* target_session, char* buffer, int received_bytes)
{
	struct sockaddr_in* client_data_address = NULL;
	struct socket* new_data_socket = NULL;

	if (target_session == NULL)
	{
		return CLIENT_INVALID;
	}

	if (buffer == NULL)
	{
		return CLIENT_INVALID_PARAM;
	}
	
	/* PORT 커맨드 읽어서 그 데이터로 CONNECT 하는 부분 -> DATA 소켓 */
	client_data_address = get_address_from_port_command(buffer, received_bytes);
	if (client_data_address != NULL)
	{
		new_data_socket = client_connect(target_session->client, client_data_address);
		if (new_data_socket == NULL)
		{
			free(client_data_address);
			session_remove_from_list(target_session);

			return CLIENT_PORT_PARSE_FAILED;
		}

		free(client_data_address);
	}
	
	send_packet_to_server(target_session->server, buffer, received_bytes, PORT_TYPE_COMMAND);

	return CLIENT_SUCCESS;
}

int client_data_received(struct session* target_session, char* buffer, int received_bytes)
{
	if (target_session == NULL)
	{
		return CLIENT_INVALID;
	}

	if (buffer == NULL)
	{
		return CLIENT_INVALID_PARAM;
	}
	send_packet_to_server(target_session->server, buffer, received_bytes, PORT_TYPE_DATA);
	return CLIENT_SUCCESS;
}

struct client* client_create(int connected_socket)
{
	struct client* new_client = NULL;
	struct socket* new_command_socket = NULL;
	struct sockaddr_in client_address = { 0, };
	uint32_t address_length = sizeof(struct sockaddr);

	new_client = (struct client*)malloc(sizeof(struct client));
	if (new_client == NULL)
	{
		return NULL;
	}

	getsockname(connected_socket, (struct sockaddr*)&client_address, &address_length);
	new_client->address.sin_addr = client_address.sin_addr;
	new_client->address.sin_port = client_address.sin_port;
	new_client->address.sin_family = AF_INET;

	new_command_socket = socket_create_by_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP, COMMAND_BUFFER_SIZE, connected_socket);
	if (new_command_socket == NULL)
	{
		client_free(new_client);

		return NULL;
	}

	new_client->command_socket = new_command_socket;
	new_client->data_socket = NULL;

	return new_client;
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
		socket_free(target_client->data_socket);
		target_client->data_socket = NULL;
	}

	free (target_client);
	return CLIENT_SUCCESS;
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

	if (new_buffer_size > 0)
	{
	packet_full_write(socket_fd, buffer, new_buffer_size);
	}
	return CLIENT_SUCCESS;
}