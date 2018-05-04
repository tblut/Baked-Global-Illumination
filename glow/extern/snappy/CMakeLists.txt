cmake_minimum_required(VERSION 3.0)
project(snappy)

add_library(snappy ${GLOW_LINK_TYPE}
    snappy/snappy.cc
    snappy/snappy-sinksource.cc
    snappy/snappy-stubs-internal.cc
    )
target_include_directories(snappy PUBLIC ./)
if(MSVC)
    target_compile_options(snappy PUBLIC /MP)
else()
    target_compile_options(snappy PRIVATE -Wall -Werror -Wno-sign-compare -std=c++11)
endif()
