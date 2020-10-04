cmake_minimum_required (VERSION 2.6)

project (virtual_switch)

find_package(Threads)



add_executable(virtual_switch_client virtual_switch_client.cpp utils.cpp)
add_executable(virtual_switch_server virtual_switch_server.cpp utils.cpp)

target_link_libraries(virtual_switch_server ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(virtual_switch_client ${CMAKE_THREAD_LIBS_INIT})
