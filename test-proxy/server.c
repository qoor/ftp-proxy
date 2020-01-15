#include "StdInc.h"

struct server** server_list = NULL;
int server_count = 0;

void add_server(const char* connection_string)
{
	char* port_start_pos;
	size_t address_len;

	server_list = (server**)realloc(server_list, sizeof(server*) * server_count + 1);
	server_list[server_count] = (server*)malloc(sizeof(server));

	if ((port_start_pos = strchr(connection_string, ':')))
	{
		address_len = (size_t)(connection_string - port_start_pos);

		server_list[server_count]->address = (char*)malloc(address_len * sizeof(char));
		strncpy(server_list[server_count]->address, address_len);
		server_list[server_count]->address[address_len] = '\0';

		server_list[server_count]->port = atoi(port_start_pos + 1);
	}

	++server_count;
}
