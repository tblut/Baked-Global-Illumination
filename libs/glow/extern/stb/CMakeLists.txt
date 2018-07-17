cmake_minimum_required(VERSION 3.0)
project(stb)

add_library(stb ${GLOW_LINK_TYPE} stb-impl.cc)
target_include_directories(stb PUBLIC ./)

if(MSVC)
    target_compile_options(stb PUBLIC /MP)
else()
    target_compile_options(stb PRIVATE -Wall -std=c++11)
endif()

if(GCC)
    # workaround for https://github.com/nothings/stb/issues/280
    target_link_libraries(stb PUBLIC gcc_s gcc)
endif()
