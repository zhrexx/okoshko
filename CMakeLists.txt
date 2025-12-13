cmake_minimum_required(VERSION 4.0)
project(okoshko)

set(CMAKE_C_STANDARD 23)

add_library(okoshko STATIC okoshko.c
        helpers/log.c)
target_link_libraries(okoshko PUBLIC X11 asound m) # TODO: change make linux only

add_executable(example example.c platform/linux_x11.c)
target_link_libraries(example PUBLIC okoshko)