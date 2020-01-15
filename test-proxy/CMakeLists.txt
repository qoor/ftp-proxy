cmake_minimum_required (VERSION 2.8)

set (SOURCE_FILE
	"main.c"
	"option.c"
	"server.c"

	"main.h"
	"StdInc.h"
	"option.h"
	"server.h"
)

add_executable (test-proxy ${SOURCE_FILE})
