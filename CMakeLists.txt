cmake_minimum_required(VERSION 3.16)

project(ShellDemo)

include_directories(
    .
)

link_directories(
    .
)

aux_source_directory(. SRC_LIST)
# add_library(shell STATIC ${SRC_LIST})

add_executable(test_main 
    ${CMAKE_CURRENT_LIST_DIR}/demos/linux/test_main.c
    ${SRC_LIST}
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/demos/linux/link.lds")
set(CMAKE_CXX_FLAGS   "-std=c++11")             # c++11
set(CMAKE_CXX_FLAGS   "-g")                     # 调试信息
set(CMAKE_CXX_FLAGS   "-Wall")                  # 开启所有警告

# target_link_libraries( test_main
    # shell
# )
