cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

file(GLOB_RECURSE SHADER_FILES "shader/*.*")

add_library(glow-extras-material ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_include_directories(glow-extras-material PUBLIC ./)
target_compile_options(glow-extras-material PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-material PUBLIC glow glow-extras-camera)
target_link_libraries(glow-extras-material PRIVATE glow-extras-geometry)
