#include "StdInc.h"

int main(int argc, char** argv)
{
	int i;

	if (!get_option(argc, argv))
	{
		return 0;
	}

/*
프로그램이 종료될 때 실행되어야 하는 코드

도중에 프로그램을 종료할 경우 이 라벨로 goto 해야 함 
*/
clean:
	for (i = 0; i < server_count; ++i)
	{
		free(server_list[i]);
	}

	free(server_list);
	return 0;
}
