cmake_minimum_required(VERSION 3.0)
project(AntTweakBar)


set(ANTTWEAKBAR_LINK_TYPE SHARED CACHE String "link type for ATB")
if(MSVC)
    set(ANTTWEAKBAR_LINK_TYPE STATIC CACHE String "" FORCE)
endif()

set(ATB_SOURCES
    include/AntTweakBar.h
    src/TwColors.cpp
    src/TwColors.h
    src/TwFonts.cpp
    src/TwFonts.h
    src/TwOpenGL.cpp
    src/TwOpenGL.h
    src/TwOpenGLCore.cpp
    src/TwOpenGLCore.h
    src/TwBar.cpp
    src/TwBar.h
    src/TwMgr.cpp
    src/TwMgr.h
    src/TwPrecomp.cpp
    src/TwPrecomp.h
    src/LoadOGL.cpp
    src/LoadOGL.h
    src/LoadOGLCore.cpp
    src/LoadOGLCore.h
    src/TwEventGLFW.c
    src/TwEventGLUT.c
    src/TwEventSDL.c
    src/TwEventSDL12.c
    src/TwEventSDL13.c
    src/TwEventSFML.cpp
    )

if(UNIX)
    set(ATB_SOURCES ${ATB_SOURCES} src/TwEventX11.c)
else()
    set(ATB_SOURCES ${ATB_SOURCES}
        src/TwEventWin.c
        # NO DX SUPPORT
        #src/TwDirect3D9.cpp
        #src/TwDirect3D9.h
        #src/TwDirect3D10.cpp
        #src/TwDirect3D10.h
        #src/TwDirect3D11.cpp
        #src/TwDirect3D11.h
        )
endif()

add_library(AntTweakBar ${ANTTWEAKBAR_LINK_TYPE} ${ATB_SOURCES})

if (MSVC)
    target_compile_options(AntTweakBar PUBLIC
        /MP
      )
    target_compile_definitions(AntTweakBar PUBLIC TW_STATIC)
else()
target_compile_options(AntTweakBar PRIVATE
    -Wall
    -Werror
  )
target_compile_options(AntTweakBar PUBLIC
    # C++11 for C++ files
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
  )
endif()

target_include_directories(AntTweakBar PUBLIC include)

#target_link_libraries(AntTweakBar PRIVATE GL)

if (UNIX)
    target_compile_definitions(AntTweakBar PUBLIC _UNIX)
    target_link_libraries(AntTweakBar PUBLIC X11)
    target_link_libraries(AntTweakBar PUBLIC GL)

    target_compile_options(AntTweakBar PRIVATE
        -Wno-strict-aliasing
      )

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(AntTweakBar PRIVATE
            -Wno-unused-const-variable
          )
    endif()
endif()

if (WIN32)
  target_compile_definitions(AntTweakBar PUBLIC TW_STATIC )
endif()
