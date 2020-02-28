# Minimum version requirements of CMake
cmake_minimum_required (VERSION 3.8)

# Build type and export compile_commands.json (for clangd)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_EXPORT_COMPILE_COMMANDS true)

# Child projects
project (ftp-proxy)
add_subdirectory ("ftp-proxy") 
 
# Target directories
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib)

