cmake_minimum_required(VERSION 3.0)
project(glow-extras)

if(MSVC)
    set(GLOW_EXTRAS_DEF_OPTIONS /MP)
else()
    set(GLOW_EXTRAS_DEF_OPTIONS -Wall -Werror -std=c++11)
endif()

# all glow-extras
add_library(glow-extras ${GLOW_LINK_TYPE} glow-extras.cc)

# Camera
add_subdirectory(camera)
target_link_libraries(glow-extras PUBLIC glow-extras-camera)

# Shader
add_subdirectory(shader)
target_link_libraries(glow-extras PUBLIC glow-extras-shader)

# Geometry
add_subdirectory(geometry)
target_link_libraries(glow-extras PUBLIC glow-extras-geometry)

# Rendering Pipeline
add_subdirectory(pipeline)
target_link_libraries(glow-extras PUBLIC glow-extras-pipeline)

# Debug
add_subdirectory(debugging)
target_link_libraries(glow-extras PUBLIC glow-extras-debugging)

# Material shading library
add_subdirectory(material)
target_link_libraries(glow-extras PUBLIC glow-extras-material)

# Timing
# requires 'aion'
if (TARGET aion)
    add_subdirectory(timing)
    target_link_libraries(glow-extras PUBLIC glow-extras-timing)
else()
    message(STATUS "target 'aion' not found, disabling glow-extras-timing")
endif()

# Assimp
# requires 'assimp'
if (TARGET assimp)
    add_subdirectory(assimp)
    target_link_libraries(glow-extras PUBLIC glow-extras-assimp)
else()
    message(STATUS "target 'assimp' not found, disabling glow-extras-assimp")
endif()

# GLFW App
# requires 'aion' and 'AntTweakBar'
if (TARGET aion)
    if (TARGET AntTweakBar)
        add_subdirectory(glfw)
        target_link_libraries(glow-extras PUBLIC glow-extras-glfw)
    else()
        message(STATUS "target 'AntTweakBar' not found, disabling glow-extras-glfw")
    endif()
else()
    message(STATUS "target 'aion' not found, disabling glow-extras-glfw")
endif()
