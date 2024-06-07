cmake_minimum_required(VERSION 3.24)
project(Unix_Shell)

set(CMAKE_CXX_STANDARD 17)

find_library(READLINE_LIBRARY readline)

add_executable(Unix_Shell main.cpp shelpers.cpp)

target_link_libraries(Unix_Shell ${READLINE_LIBRARY})
