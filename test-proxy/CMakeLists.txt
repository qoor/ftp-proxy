cmake_minimum_required (VERSION 3.8)

set (SOURCE_FILE
	"main.c"
	"option.c"
	"server.c"
	"global_variables.c"
	"log.c"

	"main.h"
	"StdInc.h"
	"option.h"
	"server.h"
	"global_variables.h"
	"log.c"
)

add_executable (test-proxy ${SOURCE_FILE})

