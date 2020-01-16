#include "StdInc.h"

#define filename "proxy.cfg"
#define MAX_CONFIG_KEY_LENGTH 32

/*
모든 종류의 옵션을 가져옴

옵션 종류:
파일
Argument :
-h & --help : 도움말 출력
-c [command] : 커맨드 실행

Command : 
debugging : 추후 디버깅용 함수 추가 [Developer Only]

*/

int get_option_from_argument(int argc, const char** argv) /* - getopt() 함수를 사용하면 좋겠지만 C89는 미지원이라 이렇게 짰음*/
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
			if (strcmp(argv[option + 1], "debugging") == 0) /*setting 커맨드 처리*/
			{
				/* 추후 디버깅 함수 추가 바람*/
				return 0;
			}
			else
			{
				fprintf(stderr, "Unknown command %s. \n", argv[option + 1]);
				return -1;
			}
			return 0;
		}
		else if (strcmp(argv[option], "-c") == 0 && argv[option + 1] == NULL) /*-c 인수 뒤 argument가 없을시*/
		{
			fprintf(stderr, "option -c requires an argument.\n");
			return -1;
		}

		if (strcmp(argv[option], "-h") == 0 || (strcmp(argv[option], "--help") == 0)) /*help 파서*/
		{
			print_help(argv[0]);
			return 0;
		}
		else
		{
			fprintf(stderr, "Unknown option %s. \n", argv[option]);
			return -1;
		}
	}
	return 1;
}

/* 옵션 파일 생성 */
int create_option_file()
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

/* 파일에서 옵션을 가져옴 */
int get_option_from_file()
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

