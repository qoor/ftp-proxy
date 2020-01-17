#ifndef __OPTION_H__
#define __OPTION_H__

#define OPTION_GET_SUCCESS			0
#define OPTION_FILE_PARSE_ERROR		-1
#define OPTION_ARGS_PARSE_ERROR		-2

struct option
{
	struct server** servers;
};

struct option proxy_option;

int get_options(int argc, const char** argv);
int get_option_from_argument(int argc, const char** argv);
int get_option_from_file(void);
void skip_whitespace(char** data);

int create_option_file(void);

int keycmp(const char* target, const char* original_key);

#endif
