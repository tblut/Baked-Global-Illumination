cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

add_library(glow-extras-glfw ${GLOW_LINK_TYPE} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(glow-extras-glfw PUBLIC ./)
target_compile_options(glow-extras-glfw PRIVATE ${GLOW_EXTRAS_DEF_OPTIONS})
target_link_libraries(glow-extras-glfw PUBLIC
    glow
    aion
    AntTweakBar
    fmt
    glfw
    glow-extras-timing
    glow-extras-material
    glow-extras-pipeline
    glow-extras-debugging
    glow-extras-camera    
)
