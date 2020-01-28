#include "option.h"

#include <stdio.h>
#include <string.h>

#include "client.h"
#include "vector.h"

#include "types.h"

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

int get_options(struct option* dest, int argc, const char** argv)
{
	if (get_option_from_argument(dest, argc, argv) != ARGUMENT_NOT_FOUND)
	{
		return OPTION_GET_ARGS_PARSE_ERROR;
	}
	if (get_option_from_file(dest) == FALSE)
	{
		return OPTION_GET_FILE_PARSE_ERROR;
	}

	return OPTION_GET_SUCCESS;
}

/*
 * Get options from arguments
 * C90 does not support getopt() then require this function
 *
 * Return values
 *  0: success
 *  1: fail
 *  2: No sub argument
*/
int get_option_from_argument(struct option* dest, int argc, const char** argv)
{
	int option_index = 0;
	if (argc > 3)
	{
		fprintf(stderr, "Usage: %s -c [command] \n", argv[0]);
		return ARGUMENT_FAIL;
	}
	for (option_index = 1; option_index < argc; ++option_index) /* argv[0] is file name */
	{
		if (strcmp(argv[option_index], "-c") == 0 && argv[option_index + 1] != NULL) /* Command parser */
		{
			if (strcmp(argv[option_index + 1], "debugging") == 0) /* Proccessing `setting` command */
			{
				/* Functions for debugging */
				polling_client();
				return ARGUMENT_SUCCESS;
			}
			else
			{
				fprintf(stderr, "Unknown command %s. \n", argv[option_index + 1]);
				return ARGUMENT_FAIL;
			}
		}
		else if (strcmp(argv[option_index], "-c") == 0 && argv[option_index + 1] == NULL) /* If argument does not have after -c argument */
		{
			fprintf(stderr, "option -c requires an argument.\n");
			return ARGUMENT_FAIL;
		}

		if (strcmp(argv[option_index], "-h") == 0 || (strcmp(argv[option_index], "--help") == 0)) /* `help` parser*/
		{
			print_help(argv[0]);
			return ARGUMENT_SUCCESS;
		}
		else
		{
			fprintf(stderr, "Unknown option %s. \n", argv[option_index]);
			return ARGUMENT_FAIL;
		}
	}
	return ARGUMENT_NOT_FOUND;
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

/* Get options from file (OPTION_FILE_PATH) */
int get_option_from_file(struct option* dest)
{
	FILE* file = fopen(OPTION_FILE_PATH, "r");
	char data[4096];
	char* current_pos;
	char config_key[MAX_CONFIG_KEY_LENGTH];
	int key_len;
	int ret = TRUE;

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
			vector_push_back(dest->connection_strings, current_pos);
		}
		else
		{
			printf("Config key '%s' is not valid.", config_key);
			
			ret = FALSE;
			break;
		}
	}

	fclose(file);
	return ret;
}

/* Skipping whitespace of C string pointer */
void skip_whitespace(char** data)
{
	if (data == NULL || *data == NULL)
	{
		return;
	}

	while (**data != '\0' && (**data == '\t' || **data == ' '))
	{
		++(*data);
	}
}

/* Compare target string as same as original key? */
int keycmp(const char* target, const char* original_key)
{
	return strncmp(target, original_key, strlen(original_key));
}

/* Print help */
/*
Do not use argv[0] as a factor.
Argv[0] should not be used as a printf factor because argv[0] is already passed as a factor when calling the print_help function.
It cause Segmentation fault
*/
int print_help(const char* argv)
{
	printf("---------- Help ----------\n");
	printf("Usage: %s \n", argv);
	printf("debugging: %s -c debugging [Developer Only] \n", argv);
	printf("Help: %s -h \n", argv);
	return 0;
}

