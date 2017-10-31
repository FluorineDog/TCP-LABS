cmake_minimum_required(VERSION 3.9)
project(dog_chatter_client)
add_executable(chat_client client.cpp)
target_link_libraries(chat_client pthread)

add_library(client_dataflow client_dataflow.cpp)
target_link_libraries(chat_client client_dataflow)
