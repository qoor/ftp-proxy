# Minimum version requirements of CMake
cmake_minimum_required (VERSION 3.8)

# Export compile_commands.json (for clangd)
set (CMAKE_EXPORT_COMPILE_COMMANDS true)

# Build type check
set (DEFAULT_CMAKE_BUILD_TYPE Release)
if (NOT CMAKE_BUILD_TYPE)
	message (STATUS "Setting build type to '${DEFAULT_CMAKE_BUILD_TYPE}' as none was specified.")
	set (CMAKE_BUILD_TYPE ${DEFAULT_CMAKE_BUILD_TYPE})
endif ()

# Target directories
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/lib)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Child projects
project (ftp-proxy)
add_subdirectory ("ftp-proxy")

set (CMAKE_C_COMPILER	clang)
set (CMAKE_CXX_COMPILER	clang++)

