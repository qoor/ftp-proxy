cmake_minimum_required (VERSION 3.8)

set (SOURCE_FILE
	"src/main.c"
	"src/option.c"
	"src/server.c"
	"src/log.c"
	"src/packet.c"
	"src/client.c"
	"src/vector.c"

	"include/StdInc.h"
	"include/option.h"
	"include/server.h"
	"include/log.h"
	"include/packet.h"
	"include/client.h"
	"include/proxy.h"
	"include/vector.h"
)

add_executable (test-proxy ${SOURCE_FILE})

target_include_directories (test-proxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
