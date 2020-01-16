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


/* IMPORTANT: MUST execute this code when program will shutdown */
clean:
	for (i = 0; i < server_count; ++i)
	{
		free(server_list[i]);
	}

	free(server_list);
	return 0;
}

/* Print help */
int print_help(const char* argv)
{
	printf("---------- Help ----------\n");
	printf("Usage: %s \n", argv);
	printf("debugging: %s -c debugging [Developer Only] \n", argv);
	printf("Help: %s -h \n", argv);
	return 0;
}
