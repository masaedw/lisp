cmake_minimum_required(VERSION 2.8.0)

FILE(GLOB BASESRCS "*.c" "*.h")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(GNU_SOURCE "-D_GNU_SOURCE")
endif ()

set(CMAKE_C_FLAGS "-Wall -Wextra -overflow -fno-strict-aliasing -std=c99 -march=native -O2 ${GNU_SOURCE}")
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(lisp ${BASESRCS})
target_link_libraries(lisp gc)
