#ifndef __OPTION_H__
#define __OPTION_H__

struct option
{
	struct vector connection_strings;
};

int get_options(struct option* dest, int argc, const char** argv);
int get_option_from_argument(struct option* dest, int argc, const char** argv);
int get_option_from_file(struct option* dest);
void skip_whitespace(char** data);

int create_option_file(void);
void add_server_to_option(struct option* dest, const char* connection_string);

int keycmp(const char* target, const char* original_key);

#endif
