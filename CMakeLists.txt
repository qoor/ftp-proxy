cmake_minimum_required (VERSION 3.8)

project ("test-proxy")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c90 -Wall")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory ("test-proxy")

