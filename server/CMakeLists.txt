cmake_minimum_required(VERSION 3.9)
project(dog_chat_server)
add_executable(chat_server server.cpp)
target_link_libraries(chat_server pthread)

add_library(server_dataflow server_dataflow.cpp)
target_link_libraries(chat_server server_dataflow)
target_link_libraries(chat_server vsqlitepp)