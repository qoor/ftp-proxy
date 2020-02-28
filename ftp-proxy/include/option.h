#ifndef PROXY_INCLUDE_OPTION_H_
#define PROXY_INCLUDE_OPTION_H_

#include <sys/epoll.h>

#include "list.h"
#include "session.h"

#define OPTION_FILE_NAME "proxy"
#define OPTION_FILE_EXT "cfg"
#define OPTION_FILE_PATH ("" OPTION_FILE_NAME "." OPTION_FILE_EXT)
#define MAX_CONFIG_KEY_LENGTH (32)

struct option
{
	struct list server_list; /* List of server addresses */
	struct list session_list;
	struct thread_pool* thread_pool;
	int epoll_fd;
	struct socket* proxy_socket;
};

/* OPTION RETURN CODE DEFINE */
enum log_option_get_error_type
{
	OPTION_GET_SUCCESS,
	OPTION_GET_FILE_PARSE_ERROR,
	OPTION_GET_ARGS_PARSE_ERROR
};
/* */

/* ARGUMENT RETURN CODE DEFINE */
enum get_argument_error_type
{
	ARGUMENT_SUCCESS,
	ARGUMENT_FAIL,
	ARGUMENT_NOT_FOUND
};
/* */

struct option* option_create(int num_threads);
int option_free(struct option* target_option);
int get_options(struct option* dest, int argc, const char** argv);
int get_option_from_argument(int argc, const char** argv);
int get_option_from_file(struct option* dest);

int create_option_file(void);
void add_server_to_option(struct option* dest, const char* connection_string);

int keycmp(const char* target, const char* original_key);
int print_help(const char* argv);

extern struct option* global_option; /* For debugging */

#endif

