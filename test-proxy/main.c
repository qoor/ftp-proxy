#include "StdInc.h"

int main(int argc, const char** argv)
{
	int i;

	if (!get_options(argc, argv))
	{
		return 0;
	}
	
	printf("파일을 읽었지비?\n");
	
	/* Main logic write in here */

	/* Free allocated memories */
	reset_server_list();

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
