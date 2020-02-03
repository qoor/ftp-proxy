#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "client.h"
#include "hashmap.h"
#include "server.h"
#include "log.h"
#include "option.h"
#include "socket.h"
#include "vector.h"
#include "session.h"
#include "client.h"
#include "thread.h"
#include "types.h"

#define THREAD_AMOUNT (10)

static void main_loop(void* arg)
{
	struct option* option = NULL;
	int ret = 0;

	if (arg == NULL)
	{
		return;
	}

	option = (struct option*)arg;
	while (TRUE)
	{
		ret = session_polling(option->epoll_fd, &option->session_list, option->proxy_socket->fd);
		if (ret != SESSION_SUCCESS)
		{
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

	/* Initializing */
	/* LOG INIT */
	if (log_init() != LOG_INIT_SUCCESS) 
	{
		return 0;
	}

	option = option_create(THREAD_AMOUNT);
	if (option == NULL)
	{
		return 0;
	}

	if (get_options(option, argc, argv) != OPTION_GET_SUCCESS)
	{
		main_free(option);

		return 0;
	}

	thread_pool_add_work(option->thread_pool, (void*)main_loop, &option);
	thread_pool_wait(option->thread_pool);

	main_free(option);

	return 0;
}

