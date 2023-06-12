cmake_minimum_required(VERSION 3.22)
project(use_ACE)
add_subdirectory(./AceTheGame/ACE/engine/)

add_executable(use_ACE main.cpp)

target_link_libraries(use_ACE
    PRIVATE  lib_ACE-static
)
