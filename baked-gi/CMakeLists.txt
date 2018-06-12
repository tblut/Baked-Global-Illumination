cmake_minimum_required(VERSION 3.0)
project(baked-gi)

file(GLOB SOURCES "*.cc" "*.hh")
file(GLOB SHADERS "shaders/*.*sh" "shaders/*.glsl")
file(GLOB TINY_GLTF_SOURCES "tinygltf/*.h" "tinygltf/*.hpp")

source_group("Source" FILES ${SOURCES})
source_group("Shaders" FILES ${SHADERS})
source_group("Source\\TinyGLTF" FILES ${TINY_GLTF_SOURCES})

FIND_PACKAGE(embree 3.0 REQUIRED)
INCLUDE_DIRECTORIES(${EMBREE_INCLUDE_DIRS})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(baked-gi ${SOURCES} ${SHADERS} ${TINY_GLTF_SOURCES})
target_link_libraries(baked-gi PUBLIC glow glow-extras glfw AntTweakBar assimp ${EMBREE_LIBRARY})
