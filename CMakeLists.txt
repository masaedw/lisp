cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_C_FLAGS "-Wall -std=c99 -Wno-unknown-attributes")
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(lisp main.c lisp.c parse.c print.c symbol.c primitive.c compile.c vm.c)
target_link_libraries(lisp gc)
