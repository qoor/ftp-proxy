cmake_minimum_required (VERSION 2.8)

set (SOURCE_FILE
	"main.c"

	"main.h"
	"StdInc.h"
)

add_executable (test-proxy ${SOURCE_FILE})
