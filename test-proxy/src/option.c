#include "option.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "client.h"
#include "vector.h"
#include "types.h"
#include "server.h"
#include "utils.h"
#include "thread.h"
#include "session.h"
#include "log.h"

/*
 * Get all of types of option from arguments
 * Options:
 *	File,
 *	Argument:
 *		-h or --help: Print help
 *		-c [command]: Execute command
 *			Command : 
 *				debugging: Used to debugging [Developer Only]
*/

struct option* option_create(int num_threads)
{
	struct option* new_option = NULL;
	struct thread_pool* new_thread_pool = NULL;
	struct socket* new_proxy_socket = NULL;
	struct sockaddr_in address = { 0, };
	int new_epoll = -1;
	struct epoll_event event = { 0, };
	int ret = 0;

	event.events = EPOLLIN;

	new_option = (struct option*)malloc(sizeof(struct option));
	if (new_option == NULL)
	{
		return NULL;
	}

	memset(new_option, 0x00, sizeof(struct option));
	LIST_INIT(&new_option->server_list);
	LIST_INIT(&new_option->session_list);

	new_thread_pool = thread_pool_create(num_threads);
	if (new_thread_pool == NULL)
	{
		option_free(new_option);

		return NULL;
	}

	new_option->thread_pool = new_thread_pool;

	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(FTP_COMMAND_PORT);
	address.sin_family = AF_INET;
	new_proxy_socket = socket_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, COMMAND_BUFFER_SIZE, &address);
	if (new_proxy_socket == NULL)
	{
		option_free(new_option);

		return NULL;
	}

	socket_listen(new_proxy_socket, 0);
	new_option->proxy_socket = new_proxy_socket;

	new_epoll = epoll_create(SOMAXCONN);
	if (new_epoll < 0)
	{
		option_free(new_option);

		return NULL;
	}

	new_option->epoll_fd = new_epoll;
	event.data.fd = new_proxy_socket->fd;
	event.events = EPOLLIN;
	ret = epoll_ctl(new_epoll, EPOLL_CTL_ADD, new_proxy_socket->fd, &event);
	if (ret < 0)
	{
		option_free(new_option);

		return NULL;
	}

	proxy_error("option", "Proxy listen socket fd: %d", new_proxy_socket->fd);

	return new_option;
}

int option_free(struct option* target_option)
{
	struct server_address* current_server = NULL;
	struct server_address* tmp_server = NULL;
	struct session* current_session = NULL;
	struct session* tmp_session = NULL;

	if (target_option == NULL)
	{
		return FALSE;
	}

	/* Finishing all jobs of thread pool first */
	if (target_option->thread_pool != NULL)
	{
		thread_pool_free(target_option->thread_pool);
		target_option->thread_pool = NULL;
	}

	list_for_each_entry_safe(current_server, tmp_server, &target_option->server_list, list, struct server_address*)
	{
		free(current_server);
	}

	list_for_each_entry_safe(current_session, tmp_session, &target_option->session_list, list, struct session*)
	{
		session_remove_from_list(current_session);
	}

	if (target_option->epoll_fd != -1)
	{
		close(target_option->epoll_fd);
		target_option->epoll_fd = -1;
	}

	if (target_option->proxy_socket != NULL)
	{
		socket_free(target_option->proxy_socket);
		target_option->proxy_socket = NULL;
	}

	free(target_option);

	return TRUE;
}

int get_options(struct option* dest, int argc, const char** argv)
{
	if (get_option_from_argument(dest, argc, argv) != ARGUMENT_NOT_FOUND)
	{
		return OPTION_GET_ARGS_PARSE_ERROR;
	}
	if (get_option_from_file(dest) == FALSE)
	{
		return OPTION_GET_FILE_PARSE_ERROR;
	}

	return OPTION_GET_SUCCESS;
}

/*
 * Get options from arguments
 * C90 does not support getopt() then require this function
 *
 * Return values
 *  0: success
 *  1: fail
 *  2: No sub argument
*/
int get_option_from_argument(struct option* dest, int argc, const char** argv)
{
	int option_index = 0;
	if (argc > 3)
	{
		fprintf(stderr, "Usage: %s -c [command] \n", argv[0]);
		return ARGUMENT_FAIL;
	}
	for (option_index = 1; option_index < argc; ++option_index) /* argv[0] is file name */
	{
		if (strcmp(argv[option_index], "-c") == 0 && argv[option_index + 1] != NULL) /* Command parser */
		{
			if (strcmp(argv[option_index + 1], "debugging") == 0) /* Proccessing `setting` command */
			{
				/* Functions for debugging */
				return ARGUMENT_SUCCESS;
			}
			else
			{
				fprintf(stderr, "Unknown command %s. \n", argv[option_index + 1]);
				return ARGUMENT_FAIL;
			}
		}
		else if (strcmp(argv[option_index], "-c") == 0 && argv[option_index + 1] == NULL) /* If argument does not have after -c argument */
		{
			fprintf(stderr, "option -c requires an argument.\n");
			return ARGUMENT_FAIL;
		}

		if (strcmp(argv[option_index], "-h") == 0 || (strcmp(argv[option_index], "--help") == 0)) /* `help` parser*/
		{
			print_help(argv[0]);
			return ARGUMENT_SUCCESS;
		}
		else
		{
			fprintf(stderr, "Unknown option %s. \n", argv[option_index]);
			return ARGUMENT_FAIL;
		}
	}
	return ARGUMENT_NOT_FOUND;
}

/* Create option file if option file is not exists */
int create_option_file(void)
{
	FILE* fp = NULL;
	
	if ((fp = fopen("proxy.cfg", "w")) == NULL)
	{
		fprintf(stderr, "proxy.cfg ERROR\n");
	}

	printf("Option file create success \n");
	fclose(fp);
	return 0;
}

/* Get options from file (OPTION_FILE_PATH) */
int get_option_from_file(struct option* dest)
{
	FILE* file = fopen(OPTION_FILE_PATH, "r");
	char data[4096];
	char* current_pos;
	char config_key[MAX_CONFIG_KEY_LENGTH];
	int key_len;
	int ret = TRUE;

	if (!file)
	{
		/* 
		 * Do not pass NULL file pointer to fclose.
		 * It cause Segmentation fault
		*/
		return FALSE;
	}

	while (fgets(data, sizeof(data), file))
	{
		key_len = 0;
		current_pos = data;

		skip_whitespace(&current_pos);
		while (*current_pos > ' ')
		{
			config_key[key_len++] = *current_pos;
			++current_pos;
		}

		config_key[key_len] = '\0';
		if (keycmp(config_key, "server_address") == 0)
		{
			skip_whitespace(&current_pos);
			server_insert_address(&dest->server_list, current_pos);
		}
		else
		{
			printf("Config key '%s' is not valid.", config_key);
			
			ret = FALSE;
			break;
		}
	}

	fclose(file);
	return ret;
}

/* Compare target string as same as original key? */
int keycmp(const char* target, const char* original_key)
{
	return strncmp(target, original_key, strlen(original_key));
}

/* Print help */
/*
Do not use argv[0] as a factor.
Argv[0] should not be used as a printf factor because argv[0] is already passed as a factor when calling the print_help function.
It cause Segmentation fault
*/
int print_help(const char* argv)
{
	printf("---------- Help ----------\n");
	printf("Usage: %s \n", argv);
	printf("debugging: %s -c debugging [Developer Only] \n", argv);
	printf("Help: %s -h \n", argv);
	return 0;
}

