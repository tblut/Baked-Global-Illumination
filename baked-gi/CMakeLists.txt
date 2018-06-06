cmake_minimum_required(VERSION 3.0)
project(baked-gi)

file(GLOB SOURCES "*.cc" "*.hh")
file(GLOB SHADERS "shaders/*.*sh")
file(GLOB TINY_GLTF_SOURCES "tinygltf/*.h" "tinygltf/*.hpp")

source_group("Source" FILES ${SOURCES})
source_group("Shaders" FILES ${SHADERS})
source_group("Source\\TinyGLTF" FILES ${TINY_GLTF_SOURCES})

FIND_PACKAGE(embree 3.0 REQUIRED)
INCLUDE_DIRECTORIES(${EMBREE_INCLUDE_DIRS})

add_executable(baked-gi ${SOURCES} ${SHADERS} ${TINY_GLTF_SOURCES})
target_link_libraries(baked-gi PUBLIC glow glow-extras glfw AntTweakBar assimp ${EMBREE_LIBRARY})