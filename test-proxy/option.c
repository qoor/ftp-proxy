#include "StdInc.h"

#define MAX_CONFIG_KEY_LENGTH 32

/*
모든 종류의 옵션을 가져옴

옵션 종류:
파일
Argument :
-h & --help : 도움말 출력
-c [command] : 커맨드 실행

Command : 
setting : 설정 파일 부분

*/

int get_argument(int argc, char** argv) /* - getopt() 함수를 사용하면 좋겠지만 C89는 미지원이라 이렇게 짰음*/
{
	int option = 0;
	if (argc > 3)
	{
		fprintf(stderr, "Usage: %s -c [command] \n", argv[0]);
		return -1;
	}
	for (option = 1; option < argc; ++option) /*들어온 인수갯수 만큼 검사 argv[0]은 기본이니 1부터 시작*/
	{
		if (strcmp(argv[option], "-c") == 0 && argv[option + 1] != NULL) /*-C Command 파서*/
		{
			if (strcmp(argv[option + 1], "setting") == 0) /*setting 커맨드 처리*/
			{
				print_setting();
				return 1;
			}
			else
			{
				fprintf(stderr, "Unknown command %s. \n", argv[option + 1]);
				return -1;
			}
			return 0;
		}
		else if (strcmp(argv[option], "-h") == 0 || (strcmp(argv[option], "--help") == 0))
		{
			print_help(argv[0]);
			return 0;
		}
		else if (strcmp(argv[option], "-c") == 0 && argv[option + 1] == NULL) /*-c 인수 뒤 argument가 없을시*/
		{
			fprintf(stderr, "option -c requires an argument.\n");
			return -1;
		}
		else
		{
			fprintf(stderr, "Unknown option %s. \n", argv[option]);
			return -1;
		}
	}
	return 0;
}

int print_help(const char* argv)
{
	printf("---------- Help ----------\n");
	printf("Usage: %s \n", argv);
	printf("Setting: %s -c setting \n", argv);
	printf("Help: %s -h \n", argv);
	return 0;
}

int print_setting()
{
	int input_value = 0;
	printf("---------- Setting ----------\n");
	printf("1. Server Registration. \n");
	printf("2. Bind Setting. \n\n");

	printf("Enter a choice number : ");
	scanf("%d", &input_value);
	switch (input_value)
	{
	case 1:
		server_registration();
		break;
	case 2:
		bind_setting();
		break;
	default:
		fprintf(stderr, "Invalid choice number. \n");
		break;
	}
	return 0;
}

int server_registration()
{
	FILE* fp = NULL;
	/* 추후 설정 파일 디렉터리 생성시 사용
	struct stat st = { 0 };
	if (stat("/etc/test", &st) == -1)
	{
		mkdir("/etc/test", 0744);
	}
	*/
	if ((fp = fopen("proxy.cfg", "w")) == NULL)
	{
		fprintf(stderr, "proxy.cfg ERROR\n");
	}
	printf("파일을 만들었지비 \n");
	fclose(fp);
	return 0;
}

int bind_setting()
{
	return 0;
}

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

