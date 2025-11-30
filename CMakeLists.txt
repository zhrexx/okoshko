cmake_minimum_required(VERSION 4.0)
project(okoshko)

set(CMAKE_C_STANDARD 23)

add_library(okoshko STATIC okoshko.c
        helpers/log.c)

add_executable(example example.c)
target_link_libraries(example PUBLIC okoshko X11)