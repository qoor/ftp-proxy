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
#include "thread.h"

static void main_free(struct hashmap* server_list, struct vector* session_list, struct thread_pool* thpool)
{
	reset_server_list(server_list);
	vector_free(session_list);
	log_free();
	thread_pool_free(thpool);
}

int main(int argc, const char** argv)
{
	struct thread_pool* thpool = NULL;
	struct hashmap* server_list = NULL;
	struct vector* session_list = NULL;
	struct option option = { .connection_strings = NULL };
	struct epoll_event server_events[MAX_EVENTS] = { { 0, }, };
	int server_epoll_fd = -1;

/* Initializing */
	/* THREAD INIT */
	thpool = thread_pool_create(10);

	/* LOG INIT */
	if (log_init() != LOG_INIT_SUCCESS) 
	{
		return 0;
	}

	/* OPTION INIT */
	if ((option.connection_strings = vector_init(0)) == NULL)
	{
		main_free(server_list, session_list, thpool);
		return 0;
	}

	if (get_options(&option, argc, argv) != OPTION_GET_SUCCESS)
	{
		main_free(server_list, session_list, thpool);
		return 0;
	}

	/* SERVER INIT */
	if ((server_list = hashmap_init(option.connection_strings->size, hashmap_hash_int, hashmap_equals_int)) == NULL)
	{
		main_free(server_list, session_list, thpool);
		return 0;
	}

	if ((server_epoll_fd = epoll_create(MAX_EVENTS)) == -1)
	{
		main_free(server_list, session_list, thpool);
		return 0;
	}

	if (add_servers_from_vector(server_list, option.connection_strings, server_epoll_fd) != SERVER_ADD_SUCCESS)
	{
		main_free(server_list, session_list, thpool);
		return 0;
	}
	
	/* SESSION_LIST INIT */
	if ((session_list = vector_init(0)) == NULL)
	{
		main_free(server_list, session_list, thpool);
		return 0;
	}
	
/* Main logic write in here */
	void thread_client_polling()
	{
		client_polling(session_list);
	}
	
	if (thread_pool_add_work(thpool, (void*)thread_client_polling, NULL) != THREAD_SUCCESS )
	{
		int temp_result = 0;
		temp_result = thread_pool_add_work(thpool, (void*)thread_client_polling, NULL);
		printf("%d \n",temp_result);
	}

	while (1)
	{
		servers_polling(server_epoll_fd, server_list, (struct epoll_event**)&server_events);
	}

/* Free allocated memories */
	main_free(server_list, session_list, thpool);

	return 0;
}
