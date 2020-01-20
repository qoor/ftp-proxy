#include "StdInc.h"

int main(int argc, const char** argv)
{
	int i;

	/* Initializing */
	if (log_init() != LOG_INIT_SUCCESS)
	{
		return 0;
	}

	if (get_options(argc, argv) != OPTION_GET_SUCCESS)
	{
		log_free();
		return 0;
	}
	/* */
	
	/* Main logic write in here */

	/* */

	/* Free allocated memories */
	reset_server_list();
	log_free();
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
