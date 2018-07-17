cmake_minimum_required(VERSION 3.0)
project(Aion)

file(GLOB_RECURSE SOURCE_FILES "src/*.cc")
file(GLOB_RECURSE HEADER_FILES "src/*.hh")

add_library(aion STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(aion PUBLIC src/)
if (MSVC)
    target_compile_options(aion PUBLIC /MP)
else()
    target_compile_options(aion PRIVATE -Wall -Werror -fPIC)
    target_compile_options(aion PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)
endif()
