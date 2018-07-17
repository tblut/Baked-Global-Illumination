cmake_minimum_required(VERSION 3.1)
project(Assimp)

CONFIGURE_FILE(
    "include/assimp/config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/assimp/config.h"
)

file(GLOB_RECURSE SOURCE_FILES "*.c" "*.cpp" "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh" "*.h" "*.hpp" "*.inl")

add_subdirectory(contrib/zlib)
add_subdirectory(contrib/irrXML)

if(MSVC)
    set(ASSIMP_LINK_TYPE STATIC)
else()
    set(ASSIMP_LINK_TYPE SHARED)
endif()

add_library(assimp ${ASSIMP_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(assimp PUBLIC
    code/BoostWorkaround
)

target_include_directories(assimp PUBLIC
    include/
    contrib/openddlparser/include
    contrib/rapidjson/include
)
target_include_directories(assimp PRIVATE 
    . # for <contrib/zlib/zlib.h>
)

if(NOT MSVC)
        target_compile_options(assimp PRIVATE
		$<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
	)
else()
    target_compile_options(assimp PUBLIC
        /MP # multi processor compilation
    )
    target_compile_options(assimp PRIVATE
        /bigobj # due to IFCReaderGen1
    )
endif()

target_compile_definitions(assimp PUBLIC
    ASSIMP_BUILD_NO_C4D_IMPORTER
)

target_compile_definitions(assimp PRIVATE
	OPENDDLPARSER_BUILD
)

target_link_libraries(assimp PRIVATE zlibstatic IrrXML)
