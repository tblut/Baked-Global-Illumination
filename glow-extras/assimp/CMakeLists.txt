cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

add_library(glow-extras-assimp ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(glow-extras-assimp PUBLIC ./)
target_compile_options(glow-extras-assimp PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-assimp PUBLIC glow)
target_link_libraries(glow-extras-assimp PRIVATE assimp)
