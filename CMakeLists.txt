cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_C_FLAGS "-Wall -Wextra -overflow -fno-strict-aliasing -std=c99 -Wno-unknown-attributes -march=native -O2")
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(lisp main.c lisp.c parse.c print.c symbol.c primitive.c compile.c vm.c syntax.c bytevector.c port.c system.c)
target_link_libraries(lisp gc)
