cmake_minimum_required(VERSION 3.0)
project(glow)

# ==================================
# Options

# minimum opengl
# e.g. CORE_42 or COMPATIBILITY_33
set(GLOW_OPENGL_SUPPORT CORE_44 CACHE STRING "Defines the OpenGL version with supported C++ code (higher versions work, but GLOW features are disabled. If you have a lower version, GLOW may crash when using those)")

# libs
set(GLOW_USE_OWN_GLM ON CACHE BOOL "If true, extern/glm is used")
set(GLOW_USE_OWN_GLAD ON CACHE BOOL "If true, extern/glad is used")
set(GLOW_USE_OWN_FMT ON CACHE BOOL "If true, extern/fmt is used")
set(GLOW_USE_OWN_STB ON CACHE BOOL "If true, extern/stb is used")
set(GLOW_USE_OWN_LODEPNG ON CACHE BOOL "If true, extern/lodepng is used")
set(GLOW_USE_OWN_GLI ON CACHE BOOL "If true, extern/gli is used")
set(GLOW_USE_OWN_SNAPPY ON CACHE BOOL "If true, extern/snappy is used")

# build options
set(GLOW_LINK_TYPE SHARED CACHE STRING "Defines the build type of libraries (shared is default)")
set(GLOW_USE_GOLD_LINKER ON CACHE BOOL "If true, ld.gold is used for linking")

# misc
set(GLOW_AUTO_PROFILING ON CACHE BOOL "If true and aion target is present, glow will profile some likely-to-be-costly functions")
set(GLOW_VALIDATION ON CACHE BOOL "If true, checks if glow is initialized and called from the correct thread (very low performance impact)")

# ==================================
# Basics
include(cmake/basic-setup.cmake)
message(STATUS "GLOW CMake Config")
message(STATUS "  Operating System : ${OPERATING_SYSTEM}")
message(STATUS "  Compiler         : ${COMPILER}")
message(STATUS "  OpenGL           : ${GLOW_OPENGL_SUPPORT}")

if(MSVC)
    set(GLOW_LINK_TYPE STATIC CACHE STRING "" FORCE) # unfortunately, we need to link statically here
endif()

if(APPLE)
    set(GLOW_USE_GOLD_LINKER OFF CACHE STRING "" FORCE) # no gold linker on mac
endif()

# DEBUG
# set(GLOW_LINK_TYPE STATIC CACHE STRING "" FORCE)
# set(GLOW_USE_GOLD_LINKER OFF CACHE STRING "" FORCE)

# ==================================
# Collect files
file(GLOB_RECURSE SOURCE_FILES "src/*.cc")
file(GLOB_RECURSE HEADER_FILES "src/*.hh")
file(GLOB_RECURSE TEST_FILES "test/*.cc" "test/*.hh")
file(GLOB_RECURSE SCRIPT_FILES "scripts/*.sh" "scripts/*.py")

# ==================================
# Create GLOW target
add_library(glow ${GLOW_LINK_TYPE}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    )

# ==================================
# Compiler flags
if(NOT MSVC)
    # internally, enable Werror and Wall
    target_compile_options(glow PRIVATE
        -Wall
        -Werror

        # C++11 for C++ files
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>

        # for extra errors
        -Wextra
        -Wpedantic
        # -Wshadow # glm has a lot of these unfortunately
        # -Wstrict-overflow # false positives!
        -Wno-unused-parameter # we don't consider this an error        
        )
    if(GCC)
        target_compile_options(glow PRIVATE -Wdouble-promotion)
    else()
        # clang supports this only at newer version
    endif()
    target_compile_options(glow PUBLIC
        # Force GLM to use C++11 (this must be public because it propagates in the header)
        -DGLM_FORCE_CXX11
        # -Wno-strict-aliasing # Temporary fix for https://github.com/g-truc/glm/issues/473
        )

    # gold linker
    if(GLOW_USE_GOLD_LINKER)
        # WRONG: target_compile_options(glow PUBLIC -fuse-ld=gold )
        target_link_libraries(glow PUBLIC  -fuse-ld=gold)
    endif()

    # arch-dependend optimization
    if(NOT DEPLOY)
        # optimize for native arch in non-deploy
        target_compile_options(glow PUBLIC -march=native)
    else()
        # otherwise at least sse4.2
        target_compile_options(glow PUBLIC -msse4.2)
    endif()
else() # MSVC
    target_compile_options(glow PUBLIC
        /MP # multi processor compilation
        )
    target_compile_definitions(glow PUBLIC
        NOMINMAX            # windows.h ...
        WIN32_LEAN_AND_MEAN # windows.h ...
        )
endif()

# ==================================
# Defines
target_compile_definitions(glow PUBLIC
    # Build Type
    GLOW_${GLOW_BUILD_TYPE}
    GLOW_BUILD_TYPE=${GLOW_BUILD_TYPE}

    # OS
    GLOW_OS_${OPERATING_SYSTEM}

    # Compiler
    GLOW_COMPILER_${COMPILER}

    # OpenGL
    GLOW_OPENGL_SUPPORT_${GLOW_OPENGL_SUPPORT}
    )

# other OpenGL defines
if(${GLOW_OPENGL_SUPPORT} MATCHES "CORE_")
    set(GLOW_OPENGL_PROFILE CORE)
else()
    set(GLOW_OPENGL_PROFILE COMPATIBILITY)
endif()
target_compile_definitions(glow PUBLIC GLOW_OPENGL_PROFILE_${GLOW_OPENGL_PROFILE})
target_compile_definitions(glow PUBLIC GLOW_OPENGL_PROFILE=${GLOW_OPENGL_PROFILE})
string(REGEX MATCH "([0-9]+)" GLOW_OPENGL_VERSION ${GLOW_OPENGL_SUPPORT})
target_compile_definitions(glow PUBLIC GLOW_OPENGL_VERSION_${GLOW_OPENGL_VERSION})
target_compile_definitions(glow PUBLIC GLOW_OPENGL_VERSION=${GLOW_OPENGL_VERSION})

if(${GLOW_VALIDATION})
    target_compile_definitions(glow PUBLIC GLOW_PERFORM_VALIDATION)
endif()

# ==================================
# Include Dirs
target_include_directories(glow PUBLIC
    src/
    )

# ==================================
# Libraries

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(glow PUBLIC ${OpenGL_LIBRARIES})
if(WIN32) # Windows
    target_link_libraries(glow PUBLIC Opengl32)
elseif(APPLE) # Probably OSX
    find_library(OPENGL_FRAMEWORK OpenGL)
    target_link_libraries(glow PUBLIC ${OPENGL_FRAMEWORK})
else() # Probably Linux
    target_link_libraries(glow PUBLIC GL)
endif()

# Qt if found
if(Qt5Gui_FOUND)
    message(STATUS "Found Qt GUI, building with Qt support.")
    target_link_libraries(glow PUBLIC ${Qt5Gui_LIBRARIES})
    target_compile_definitions(glow PUBLIC GLOW_USE_QT)
endif()

# GLM
if(GLOW_USE_OWN_GLM)
    target_include_directories(glow PUBLIC extern/glm)
endif()

# GLAD loader
if(GLOW_USE_OWN_GLAD)
    set(GLAD_VERSION ${GLOW_OPENGL_VERSION})
    string(REGEX REPLACE "(.)(.)" "\\1.\\2" GLAD_VERSION ${GLAD_VERSION})
    set(GLAD_PROFILE ${GLOW_OPENGL_PROFILE})
    add_subdirectory(extern/glad)
    target_link_libraries(glow PUBLIC glad)
endif()

# C++ format
if(GLOW_USE_OWN_FMT)
    add_subdirectory(extern/fmt)
    target_link_libraries(glow PRIVATE fmt)
endif()

# STB image
if(GLOW_USE_OWN_STB)
    add_subdirectory(extern/stb)
    target_link_libraries(glow PRIVATE stb)
endif()

# Lodepng
if(GLOW_USE_OWN_LODEPNG)
    add_subdirectory(extern/lodepng)
    target_link_libraries(glow PRIVATE lodepng)
endif()

# GLI
if(GLOW_USE_OWN_GLI)
    add_subdirectory(extern/gli)
    target_link_libraries(glow PRIVATE gli)
endif()

# Snappy
if(GLOW_USE_OWN_SNAPPY)
    add_subdirectory(extern/snappy)
    target_link_libraries(glow PRIVATE snappy)
endif()

# AION
if(GLOW_AUTO_PROFILING)
    if (TARGET aion)
        message(STATUS "Building with aion-profiling ('aion' target found and auto-profiling active)")
        target_link_libraries(glow PUBLIC aion)
        target_compile_definitions(glow PUBLIC GLOW_AION_PROFILING)
    endif()
endif()

# ==================================
# dummy target for additional files
add_custom_target(_dummy_glow SOURCES
    Readme.md
    .clang-format
    ${SCRIPT_FILES}
    )

# ==================================
# Report
message(STATUS "")
message(STATUS "**************************************")
message(STATUS "GLOW CMake Report")
message(STATUS "")
message(STATUS "  Linked Libraries:")
get_target_property(GLOW_LIBS glow INTERFACE_LINK_LIBRARIES)
foreach(v ${GLOW_LIBS})
    message(STATUS "    * ${v}")
endforeach()
message(STATUS "")
message(STATUS "  Defines:")
get_target_property(GLOW_DEFINES glow INTERFACE_COMPILE_DEFINITIONS)
foreach(v ${GLOW_DEFINES})
    message(STATUS "    * ${v}")
endforeach()
message(STATUS "")
message(STATUS "  Compile Options:")
get_target_property(GLOW_COMPILE_OPTIONS glow INTERFACE_COMPILE_OPTIONS)
foreach(v ${GLOW_COMPILE_OPTIONS})
    message(STATUS "    * ${v}")
endforeach()
message(STATUS "")
message(STATUS "  Include directories:")
get_target_property(GLOW_INCLUDE_DIRECTORIES glow INTERFACE_INCLUDE_DIRECTORIES)
foreach(v ${GLOW_INCLUDE_DIRECTORIES})
    message(STATUS "    * ${v}")
endforeach()
message(STATUS "")
message(STATUS "**************************************")
message(STATUS "")
