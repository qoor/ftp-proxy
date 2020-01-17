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
	
	printf("파일을 읽었지비?\n");
	
	/* Main logic write in here */

	/* */

	/* Free allocated memories */
	reset_server_list();
	log_free();
	/* */

	return 0;
}

/* Print help */
int print_help(const char* argv)
{
	printf("---------- Help ----------\n");
	printf("Usage: %s \n", argv[0]);
	printf("debugging: %s -c debugging [Developer Only] \n", argv[0]);
	printf("Help: %s -h \n", argv[0]);
	return 0;
}
