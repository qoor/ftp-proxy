#ifndef PROXY_INCLUDE_OPTION_H_
#define PROXY_INCLUDE_OPTION_H_

#define OPTION_FILE_NAME "proxy"
#define OPTION_FILE_EXT "cfg"
#define OPTION_FILE_PATH ("" OPTION_FILE_NAME "." OPTION_FILE_EXT)
#define MAX_CONFIG_KEY_LENGTH (32)

struct option
{
	struct vector* connection_strings;
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

int get_options(struct option* dest, int argc, const char** argv);
int get_option_from_argument(struct option* dest, int argc, const char** argv);
int get_option_from_file(struct option* dest);
void skip_whitespace(char** data);

int create_option_file(void);
void add_server_to_option(struct option* dest, const char* connection_string);

int keycmp(const char* target, const char* original_key);
int print_help(const char* argv);

#endif

