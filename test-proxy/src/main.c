#include "StdInc.h"

static void main_free(struct hashmap* server_list);

int main(int argc, const char** argv)
{
	struct hashmap* server_list = NULL;

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
		servers_polling(server_epoll_fd, server_list, (struct epoll_event**)&server_events);
	}
	/* */

	/* Free allocated memories */
	main_free(server_list);
	/* */

	return 0;
}

void main_free(struct hashmap* server_list)
{
	reset_server_list(server_list);
	log_free();
}
