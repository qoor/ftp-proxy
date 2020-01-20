#ifndef __OPTION_H__
#define __OPTION_H__

int get_options(int argc, const char** argv);
int get_option_from_argument(int argc, const char** argv);
int get_option_from_file(void);
void skip_whitespace(char** data);

int create_option_file(void);

int keycmp(const char* target, const char* original_key);

#endif
