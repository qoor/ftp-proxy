#include "StdInc.h"

#define MAX_CONFIG_KEY_LENGTH 32

/*
모든 종류의 옵션을 가져옴

옵션 종류:
파일
Argument

추후 Argument에서 옵션을 가져오는 get_option_from_argument 함수 추가해야 함
*/
int get_option(int argc, char** argv)
{
	char filename[128] = "proxy.cfg";

	if (!get_option_from_file(filename))
	{
		return 0;
	}

	return 1;
}

/* 파일에서 옵션을 가져옴 */
int get_option_from_file(const char* filename)
{
	FILE* file = fopen(filename, "r");
	char data[4096];
	char* current_pos;
	char config_key[MAX_CONFIG_KEY_LENGTH];
	int key_len;
	int option_mode = 0;

	if (!file)
	{
		return 0;
	}

	while (fgets(data, sizeof(data), file))
	{
		key_len = 0;
		current_pos = data;

		skip_whitespace(&current_pos);

		while (*current_pos > ' ')
		{
			++current_pos;
			config_key[key_len++] = *current_pos;
		}

		config_key[key_len] = '\0';

		if (strncmp(config_key, "server_address", 7) == 0)
		{
			skip_whitespace(&current_pos);
			add_server(current_pos);
		}
		else
		{
			printf("Config key '%s' is not valid.", config_key);
			return 0;
		}
	}
}

/* 문자열에서 공백 건너뜀 */
void skip_whitespace(char** data)
{
	assert(data);

	while (**data != '\0' && (**data == '\t' || **data == ' '))
	{
		++(*data);
	}
}

