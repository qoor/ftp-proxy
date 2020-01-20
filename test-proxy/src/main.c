#include "StdInc.h"

static void main_free();

int main(int argc, const char** argv)
{
	struct vector server_list = { 0, };
	int server_count = 0;

	struct option option = {
		.connection_strings = { 0, }
	};

	/* Initializing */
	if (log_init() != LOG_INIT_SUCCESS)
	{
		return 0;
	}

	if (vector_init(&option.connection_strings, 0) != VECTOR_SUCCESS || vector_init(&server_list, 0) != VECTOR_SUCCESS)
	{
		main_free(&server_list);
		return 0;
	}

	if (get_options(&option, argc, argv) != OPTION_GET_SUCCESS)
	{
		main_free(&server_list);
		return 0;
	}

	if (add_servers_from_vector(&server_list, &option.connection_strings) != SERVER_ADD_SUCCESS)
	{
		main_free(&server_list);
		return 0;
	}
	/* */
	
	/* Main logic write in here */

	/* */

	/* Free allocated memories */
	main_free(&server_list);
	/* */

	return 0;
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

void main_free(struct vector* server_list)
{
	reset_server_list(server_list);
	log_free();
}
