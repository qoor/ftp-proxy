#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "client.h"
#include "hashmap.h"
#include "server.h"
#include "log.h"
#include "option.h"
#include "packet.h"
#include "vector.h"
#include "session.h"
#include "client.h"


static void main_free(struct hashmap* server_list);

void main_free(struct hashmap* server_list)
{
	reset_server_list(server_list);
	log_free();
}

int main(int argc, const char** argv)
{
	struct hashmap* server_list = NULL;
	struct vector* session_list = NULL;

	struct option option = {
		.connection_strings = NULL
	};

	struct epoll_event server_events[MAX_EVENTS] = { { 0, }, };
	int server_epoll_fd = -1;

	/* Initializing */
	if (log_init() != LOG_INIT_SUCCESS)
	{
		return 0;
	}

	if ((option.connection_strings = vector_init(0)) == NULL)
	{
		main_free(server_list);
		return 0;
	}


	if (get_options(&option, argc, argv) != OPTION_GET_SUCCESS)
	{
		main_free(server_list);
		return 0;
	}

	if ((server_list = hashmap_init(option.connection_strings->size, hashmap_hash_int, hashmap_equals_int)) == NULL)
	{
		main_free(server_list);
		return 0;
	}


	if ((session_list = vector_init(0)) == NULL)
	{
		return 0;
	}

	if ((server_epoll_fd = epoll_create(MAX_EVENTS)) == -1)
	{
		main_free(server_list);
		return 0;
	}

	if (add_servers_from_vector(server_list, option.connection_strings, server_epoll_fd) != SERVER_ADD_SUCCESS)
	{
		main_free(server_list);
		return 0;
	}
	/* */
	
	/* Main logic write in here */
	while (1)
	{
		/* 추후 멀티 쓰레드로 각각 함수 돌려야함 지금은 둘다 동시에 작동 안함*/
		client_polling(session_list);
		servers_polling(server_epoll_fd, server_list, (struct epoll_event**)&server_events);
	}
	/* */

	/* Free allocated memories */
	main_free(server_list);
	/* */

	return 0;
}
