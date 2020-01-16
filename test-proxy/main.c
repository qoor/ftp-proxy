#include "StdInc.h"

int main(int argc, const char** argv)
{
	int i = 0;

	if (get_option_from_argument(argc, argv) == -1) /*함수 반환 값-1 실패 0 성공*/
	{
		return 0;
	}
	else
	{
		if(!get_option_from_file())
		{
			printf("파일을 못읽었지비\n");
			return 0;
		}
		else
		{
			printf("파일을 읽었지비\n");
		}
	}


	/*메인 로직 작성*/

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
