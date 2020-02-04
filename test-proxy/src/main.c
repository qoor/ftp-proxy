#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "client.h"
#include "server.h"
#include "log.h"
#include "option.h"
#include "socket.h"
#include "session.h"
#include "client.h"
#include "thread.h"
#include "types.h"

#define THREAD_AMOUNT (10)

struct option* global_option = NULL;

static void main_loop(void* arg)
{
	struct option* option = NULL;
	int ret = 0;
	struct epoll_event events[MAX_EVENTS];

	if (arg == NULL)
	{
		return;
	}

	option = (struct option*)arg;
	while (TRUE)
	{
		ret = session_polling(option->epoll_fd, &option->session_list, option->proxy_socket->fd, events);
		if (ret != SESSION_SUCCESS)
		{
			proxy_error("main", "Session poll suspended. [Error: %d]", ret);
			break;
		}
	}
}

static void main_free(struct option* option)
{
	log_free();
	option_free(option);
}

int main(int argc, const char** argv)
{
	struct option* option = NULL;
	int ret = 0;

	/* Initializing */
	/* LOG INIT */
	if (log_init() != LOG_INIT_SUCCESS) 
	{
		fprintf(stderr, "[main] log initialize failed\n");

		return 0;
	}

	option = option_create(THREAD_AMOUNT);
	if (option == NULL)
	{
		proxy_error("main", "Not enough memory to initialize option");

		return 0;
	}

	if (get_options(option, argc, argv) != OPTION_GET_SUCCESS)
	{
		proxy_error("main", "Option parse failed");
		main_free(option);

		return 0;
	}

	global_option = option;

	proxy_error("main", "Proxy start working..");
	
	ret = thread_pool_add_work(option->thread_pool, (void*)main_loop, option);
	if (ret != THREAD_SUCCESS)
	{
		proxy_error("main", "Failed to adding work to thread pool");
		main_free(option);

		return 0;
	}

	ret = thread_pool_wait(option->thread_pool);
	if (ret != THREAD_SUCCESS)
	{
		proxy_error("main", "Failed to waiting thread pool");
		main_free(option);

		return 0;
	}

	main_free(option);

	return 0;
}

