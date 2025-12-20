cmake_minimum_required(VERSION 3.10)
project(okoshko C)

set(CMAKE_C_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ftree-vectorize")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")

    # breaks IEEE 754 compliance
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffast-math")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -funroll-loops")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fomit-frame-pointer")

    # set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -msse2 -mavx2 -mfma")
elseif(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /Oi /GL /arch:AVX2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

add_library(okoshko STATIC
        okoshko.c
        helpers/log.c
)

if(LINUX)
    target_link_libraries(okoshko PUBLIC X11 asound m pthread)
elseif(WIN32)
    target_link_libraries(okoshko PUBLIC gdi32 winmm)
elseif(APPLE)
    target_link_libraries(okoshko PUBLIC "-framework Cocoa" "-framework OpenGL")
endif()

target_include_directories(okoshko PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(example
        example.c
        platform/linux_x11.c
)
target_link_libraries(example PUBLIC okoshko)

# FOR PROFILING UNCOMMENT
# NOTE: 2 pass
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-generate")
# set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fprofile-generate")
#
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fprofile-use -fprofile-correction")
# set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fprofile-use")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C Flags (Release): ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")