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

# target_link_libraries( test_main
    # shell
# )
