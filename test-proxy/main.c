#include "StdInc.h"

int main(int argc, char** argv)
{
	int i;

	if (!get_option(argc, argv))
	{
		return 0;
	}

clean:
	for (i = 0; i < server_count; ++i)
	{
		free(server_list[i]);
	}

	free(server_list);
	return 0;
}
