#include "StdInc.h"

/* server_list 전역 변수에 서버를 추가함 */
void add_server(const char* connection_string)
{
	char* port_start_pos;
	size_t address_len;

	server_list = (struct server**)realloc(server_list, sizeof(struct server*) * server_count + 1);
	server_list[server_count] = (struct server*)malloc(sizeof(struct server));

	if ((port_start_pos = strchr(connection_string, ':'))) /* 콜론을 발견한 경우 (콜론 앞: IP, 콜론 뒤: Port) */
	{
		address_len = (size_t)(port_start_pos - connection_string);

		server_list[server_count]->address = (char*)malloc(address_len * sizeof(char));
		strncpy(server_list[server_count]->address, connection_string, address_len);
		server_list[server_count]->address[address_len] = '\0';

		server_list[server_count]->port = atoi(port_start_pos + 1);
	}
	printf("서버를 추가했지비 \n");
	++server_count;
}

void reset_server_list(void)
{
	int i;
	
	if (!server_list)
	{
		return;
	}

	for (i = 0; i < server_count; ++i)
	{
		free(server_list[i]);
	}

	free(server_list);
}
