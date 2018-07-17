cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

add_library(glow-extras-shader ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(glow-extras-shader PUBLIC ./)
target_compile_options(glow-extras-shader PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-shader PUBLIC glow)
