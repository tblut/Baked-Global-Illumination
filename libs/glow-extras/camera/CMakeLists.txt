cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

add_library(glow-extras-camera ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(glow-extras-camera PUBLIC ./)
target_compile_options(glow-extras-camera PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-camera PUBLIC glow)
