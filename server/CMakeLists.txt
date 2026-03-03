cmake_minimum_required(VERSION 3.14)
project(gaijin_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.70 REQUIRED)

add_executable(server
    src/Main.cpp
    src/Storage.cpp
    src/Persistence.cpp
    src/CommandHandler.cpp
    src/ConnectionHandler.cpp
    src/Server.cpp
)

target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})

add_custom_command(TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
        $<TARGET_FILE_DIR:server>/
)
add_custom_command(TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/server_config.txt
        $<TARGET_FILE_DIR:server>/
)