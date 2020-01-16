#ifndef __OPTION_H__
#define __OPTION_H__

struct option
{
	struct server** servers;
};

struct option proxy_option;

int get_options(int argc, const char** argv);
int get_option_from_argument(int argc, const char** argv);
int get_option_from_file();
void skip_whitespace(char** data);

int create_option_file();

int keycmp(const char* target, const char* original_key);

#endif
