#include "StdInc.h"

#define OPTION_FILE_NAME "proxy.cfg"
#define MAX_CONFIG_KEY_LENGTH 32

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

int get_options(int argc, const char** argv)
{
	if (get_option_from_argument(argc, argv) != 1)
	{
		return OPTION_ARGS_PARSE_ERROR;
	}

	if (!get_option_from_file())
	{
		return OPTION_FILE_PARSE_ERROR;
	}

	return OPTION_GET_SUCCESS;
}

/*
 * Get options from arguments
 * C90 does not support getopt() then require this function
 *
 * Return values
 * -1: fail
 *  0: success
 *  1: No sub argument
*/
int get_option_from_argument(int argc, const char** argv)
{
	int option = 0;
	if (argc > 3)
	{
		fprintf(stderr, "Usage: %s -c [command] \n", argv[0]);
		return -1;
	}
	for (option = 1; option < argc; ++option) /* argv[0] is file name */
	{
		if (strcmp(argv[option], "-c") == 0 && argv[option + 1] != NULL) /* Command parser */
		{
			if (strcmp(argv[option + 1], "debugging") == 0) /* Proccessing `setting` command */
			{
				/* Functions for debugging */
				return 0;
			}
			else
			{
				fprintf(stderr, "Unknown command %s. \n", argv[option + 1]);
				return -1;
			}
			return 0;
		}
		else if (strcmp(argv[option], "-c") == 0 && argv[option + 1] == NULL) /* If argument does not have after -c argument */
		{
			fprintf(stderr, "option -c requires an argument.\n");
			return -1;
		}

		if (strcmp(argv[option], "-h") == 0 || (strcmp(argv[option], "--help") == 0)) /* `help` parser*/
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

/* Get options from file (OPTION_FILE_NAME) */
int get_option_from_file(void)
{
	FILE* file = fopen(OPTION_FILE_NAME, "r");
	char data[4096];
	char* current_pos;
	char config_key[MAX_CONFIG_KEY_LENGTH];
	int key_len;
	int option_mode = 0;
	int ret = 1;

	if (!file)
	{
		/* 
		 * Do not pass NULL file pointer to fclose.
		 * It cause Segmentation fault
		*/
		return 0;
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
			add_server(current_pos);
		}
		else
		{
			printf("Config key '%s' is not valid.", config_key);
			
			ret = 0;
			break;
		}
	}

	fclose(file);
	return ret;
}

/* Skipping whitespace of C string pointer */
void skip_whitespace(char** data)
{
	assert(data);

	while (**data != '\0' && (**data == '\t' || **data == ' '))
	{
		++(*data);
	}
}

/* Compare target string as same as original key?  */
int keycmp(const char* target, const char* original_key)
{
	return strncmp(target, original_key, strlen(original_key));
}

