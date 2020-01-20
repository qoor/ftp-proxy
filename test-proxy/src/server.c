#include "StdInc.h"

static int remove_list_last_server(struct server* target_server);
static void server_free(struct server* target_server);

/* Add server data to server_list */
int add_server(const char* connection_string)
{
	char* port_start_pos = NULL;
	size_t address_len = 0;
	size_t server_list_size = sizeof(struct server*) * server_count;
	struct server* new_server = NULL;
	int socket_fd = -1;

	server_list = (struct server**)realloc(server_list, server_list_size + sizeof(struct server*));
	new_server = server_list[server_count] = (struct server*)malloc(sizeof(struct server));

	if (new_server == NULL)
	{
		server_list = (struct server**)realloc(server_list, server_list_size);
		return SERVER_ADD_ALLOC_FAILED;
	}

	new_server->address = NULL;
	new_server->port = 0;
	new_server->socket_fd = -1;
	memset(&(new_server->socket_address), 0x00, sizeof(struct sockaddr_in));

	if ((port_start_pos = strchr(connection_string, ':')) == NULL) /* If colon not found from connection string (Before colon: IP, After colon: Port) */
	{
		remove_list_last_server(new_server);
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	address_len = (size_t)(port_start_pos - connection_string);
	new_server->address = (char*)malloc(address_len * sizeof(char));

	if (new_server->address == NULL)
	{
		remove_list_last_server(new_server);
		return SERVER_ADD_ALLOC_FAILED;
	}

	strncpy(server_list[server_count]->address, connection_string, address_len);
	server_list[server_count]->address[address_len] = '\0';
	server_list[server_count]->port = atoi(port_start_pos + 1);

	if ((socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
	{
		remove_list_last_server(new_server);
		return SERVER_ADD_SOCKET_CREATE_FAILED;
	}

	++server_count;
	printf("Server %s:%d listed successfully.\n", new_server->address, new_server->port);

	return SERVER_ADD_SUCCESS;
}

/*
 * Remove the back of listed server from list (Internal)
 * target_server MUST be in server_list
*/
static int remove_list_last_server(struct server* target_server)
{
	if (target_server == NULL)
	{
		return SERVER_REMOVE_INVALID_SERVER;
	}

	server_free(target_server);

	server_list = (struct server**)realloc(server_list, sizeof(struct server*) * (server_count - 1));
	--server_count;

	return SERVER_REMOVE_SUCCESS;
}

static void server_free(struct server* target_server)
{
	if (target_server->address != NULL)
	{
		free(target_server->address);
		target_server->address = NULL;
	}

	target_server->port = 0;

	if (target_server->socket_fd != -1)
	{
		close(target_server->socket_fd);
		target_server->socket_fd = -1;
	}
}

void reset_server_list(void)
{
	int i = 0;
	
	if (server_list == NULL)
	{
		return;
	}

	for (i = 0; i < server_count; ++i)
	{
		server_free(server_list[i]);
	}

	free(server_list);
	server_count = 0;
}

void send_packet_to_server(struct server* dst_server, unsigned char* packet)
{
	change_packet_dst_address(packet, &dst_server->socket_address);
	sendto(dst_server->socket_fd, packet, ((struct iphdr*)packet)->tot_len, 0x0, (struct sockaddr*)&dst_server->socket_address, sizeof(struct sockaddr));
}
