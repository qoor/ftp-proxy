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

static void main_free(struct thread_pool* thpool)
{
	log_free();
	thread_pool_free(thpool);
}

int main(int argc, const char** argv)
{
	struct thread_pool* thpool = NULL;
	struct option option;
	int server_epoll_fd = -1;

	option.server_list.n = &option.server_list;
	option.server_list.p = &option.server_list;

	/* Initializing */
	/* THREAD INIT */
	thpool = thread_pool_create(10);

	/* LOG INIT */
	if (log_init() != LOG_INIT_SUCCESS) 
	{
		return 0;
	}

	if (get_options(&option, argc, argv) != OPTION_GET_SUCCESS)
	{
		main_free(thpool);
		return 0;
	}

	/* Main logic write in here */
	while(TRUE)
	{
		/* 메인 로직 : 사용자 옵션 선택 등 구현 */
	}
	return 0;
}

