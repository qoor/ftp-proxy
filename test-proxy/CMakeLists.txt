cmake_minimum_required (VERSION 3.8)

set (SOURCE_FILE
	"src/main.c"
	"src/option.c"
	"src/server.c"
	"src/log.c"
	"src/socket.c"
	"src/client.c"
	"src/vector.c"
	"src/hashmap.c"
	"src/session.c"
	"src/thread.c"
	"src/packet.c"
	"src/utils.c"

	"include/option.h"
	"include/server.h"
	"include/log.h"
	"include/socket.h"
	"include/client.h"
	"include/vector.h"
	"include/hashmap.h"
	"include/session.h"
	"include/thread.h"
	"include/types.h"
	"include/packet.h"
	"include/utils.h"
)

add_executable (test-proxy ${SOURCE_FILE})

target_include_directories (test-proxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries (test-proxy pthread)

