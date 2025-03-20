cmake_minimum_required(VERSION 3.10)

project(microcomp_emulator VERSION 1.0)

set(CMAKE_C_STANDARD 99)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIRECTORY}")


add_executable(microcomp_emulator microcomp_emulator.c)

target_compile_options(microcomp_emulator PUBLIC -Wall -Wextra -Wpedantic -Werror -Wdouble-promotion)

target_link_libraries(microcomp_emulator)
