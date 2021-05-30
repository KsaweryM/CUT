cmake_minimum_required(VERSION 3.18.4)
project(CUT C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)
add_compile_options(-Wall -Wextra -pedantic)
add_executable(CUT src/main.c src/integer-buffer.c src/string-buffer.c src/test/integer-buffer-test.c src/test/string-buffer-test.c src/test/tests.c)