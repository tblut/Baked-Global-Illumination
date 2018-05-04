cmake_minimum_required(VERSION 3.0)
project(baked-gi)

file(GLOB_RECURSE SOURCES "*.cc" "*.hh" "*.*sh")

add_executable(baked-gi ${SOURCES})
target_link_libraries(baked-gi PUBLIC glow glow-extras glfw AntTweakBar assimp)
#target_compile_options(baked-gi PUBLIC ${GLOW_SAMPLES_DEF_OPTIONS})