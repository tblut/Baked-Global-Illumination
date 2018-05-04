cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

file(GLOB_RECURSE SHADER_FILES "shader/*.*")

add_library(glow-extras-pipeline ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_include_directories(glow-extras-pipeline PUBLIC ./)
target_compile_options(glow-extras-pipeline PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-pipeline PUBLIC glow glow-extras-camera)
target_link_libraries(glow-extras-pipeline PRIVATE glow-extras-geometry)
