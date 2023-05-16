cmake_minimum_required(VERSION 3.16)

project(shell)

include_directories(
    .
)

link_directories(
    .
)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE 
    -Wl,--start-group
    main
    shell
    -Wl,--end-group
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/project/linux)
