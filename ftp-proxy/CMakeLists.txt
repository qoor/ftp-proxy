cmake_minimum_required (VERSION 3.8)

# ftp-proxy binary version
set (PROJECT_VERSION_MAJOR 1)
set (PROJECT_VERSION_MINOR 01)

# Binary name
set (BINARY_NAME
	${CMAKE_PROJECT_NAME}-${PROJECT_VERSION_MAJOR}-${PROJECT_VERSION_MINOR}.out)

# Source files
set (SOURCE_FILE
	"src/main.c"
	"src/option.c"
	"src/server.c"
	"src/log.c"
	"src/socket.c"
	"src/client.c"
	"src/session.c"
	"src/thread.c"
	"src/packet.c"
	"src/utils.c"

	"include/option.h"
	"include/server.h"
	"include/log.h"
	"include/socket.h"
	"include/client.h"
	"include/session.h"
	"include/thread.h"
	"include/types.h"
	"include/packet.h"
	"include/utils.h"
)

# Register the executable binary
add_executable (${BINARY_NAME} ${SOURCE_FILE})

# Include path of headers directory
target_include_directories (${BINARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link libraries
target_link_libraries (${BINARY_NAME} pthread)

# Set compile options
target_compile_options (${BINARY_NAME} PUBLIC
	-pedantic -Wall -Wextra -Werror -std=c90)

