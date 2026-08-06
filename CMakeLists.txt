cmake_minimum_required(VERSION 3.20)
project(chessInC LANGUAGES C)
CMAKE_POLICY(SET CMP0111 NEW)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()
# find_package(raylib REQUIRED)

add_executable(chessInC 
src/main.c
src/pawn.c
src/position.c
src/move.c
)
# target_link_libraries(chessThree PUBLIC raylib)
