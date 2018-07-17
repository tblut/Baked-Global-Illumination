cmake_minimum_required(VERSION 3.0)
project(fmt)

add_library(fmt ${GLOW_LINK_TYPE}
    fmt/format.cc
    fmt/format.hh
)

if(MSVC)
    target_compile_options(fmt PUBLIC /MP)
else()
    target_compile_options(fmt PRIVATE -Wall -std=c++11)
endif()

target_include_directories(fmt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
