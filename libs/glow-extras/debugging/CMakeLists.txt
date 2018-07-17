cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")
file(GLOB_RECURSE SHADER_FILES "*.*sh")

add_library(glow-extras-debugging ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_include_directories(glow-extras-debugging PUBLIC ./)
target_compile_options(glow-extras-debugging PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-debugging PUBLIC
    glow
    glow-extras-geometry
    glow-extras-camera    
    glow-extras-pipeline
)
