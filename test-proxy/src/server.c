#include "StdInc.h"

static int remove_list_last_server(struct vector* dest);
static void server_free(struct server* target_server);

/* Add server to destination from connection strings */
int add_servers_from_vector(struct vector* dest, const struct vector* connection_strings)
{
	int i = 0;
	int server_count = 0;
	char* current_string = NULL;
	int ret = SERVER_ADD_SUCCESS;

	if (connection_strings == NULL)
	{
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	server_count = connection_strings->size;

	if (server_count == 0)
	{
		return SERVER_ADD_NO_SERVERS;
	}

	for (i = 0; i < server_count; ++i)
	{
		if ((current_string = connection_strings->container[i]) == NULL)
		{
			continue;
		}

		if ((ret = add_server(dest, current_string)))
		{
			break;
		}
	}

	return ret;
}

int add_server(struct vector* dest, const char* connection_string)
{
	struct server* server_object = NULL;
	char* colon_pos = NULL;
	char* server_ip = NULL;
	size_t address_length = 0;

	if (dest == NULL)
	{
		return SERVER_ADD_ALLOC_FAILED;
	}

	if (connection_string == NULL)
	{
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}
	
	if ((server_object = (struct server*)malloc(sizeof(struct server))) == NULL)
	{
		return SERVER_ADD_ALLOC_FAILED;
	}

	server_object->socket_fd = -1;
	memset(&server_object->socket_address, 0x00, sizeof(struct sockaddr_in));

	if ((colon_pos = strchr(connection_string, ':')) == NULL)
	{
		server_free(server_object);
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	address_length = colon_pos - connection_string;

	if ((server_ip = (char*)malloc((address_length + 1) * sizeof(char))) == NULL)
	{
		server_free(server_object);
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	strncpy(server_ip, connection_string, address_length + 1);
	server_ip[address_length] = '\0';

	server_object->socket_address.sin_addr.s_addr = htonl(atoi(server_ip));
	server_object->socket_address.sin_port = htons(atoi(colon_pos + 1));

	free(server_ip);

	if (vector_push_back(dest, server_object) != VECTOR_SUCCESS)
	{
		server_free(server_object);
		return SERVER_ADD_ALLOC_FAILED;
	}

	return SERVER_ADD_SUCCESS;
}

/*
 * Remove the back of listed server from list (Internal)
 * target_server MUST be in server_list
*/
int remove_list_last_server(struct vector* dest)
{
	struct server* target_server = NULL;

	if (dest == NULL || (target_server = vector_get(dest, dest->size - 1)) == NULL)
	{
		return SERVER_REMOVE_INVALID_SERVER;
	}

	server_free(target_server);
	vector_pop_back(dest);

	return SERVER_REMOVE_SUCCESS;
}

static void server_free(struct server* target_server)
{
	if (target_server->socket_fd != -1)
	{
		close(target_server->socket_fd);
		target_server->socket_fd = -1;
	}

	memset(&target_server->socket_address, 0x00, sizeof(struct sockaddr_in));
}

void reset_server_list(struct vector* dest)
{
	int i = 0;
	int server_count = 0;
	
	if (dest == NULL)
	{
		return;
	}

	server_count = dest->size;

	for (i = 0; i < server_count; ++i)
	{
		server_free(dest->container[i]);
	}

	vector_clear(dest);
}

void send_packet_to_server(struct server* dst_server, unsigned char* packet)
{
	change_packet_address(packet, &dst_server->socket_address, PACKET_ADDR_CHANGE_TYPE_DEST);
	sendto(dst_server->socket_fd, packet, ((struct iphdr*)packet)->tot_len, 0x0, (struct sockaddr*)&dst_server->socket_address, sizeof(struct sockaddr));
}

