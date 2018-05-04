cmake_minimum_required(VERSION 3.0)
project(gli)

file(GLOB ROOT_SOURCE gli/*.cpp)
file(GLOB ROOT_INLINE gli/*.inl)
file(GLOB ROOT_HEADER gli/*.hpp)

file(GLOB_RECURSE CORE_SOURCE gli/core/*.cpp)
file(GLOB_RECURSE CORE_INLINE gli/core/*.inl)
file(GLOB_RECURSE CORE_HEADER gli/core/*.hpp)

file(GLOB_RECURSE GTX_SOURCE gli/gtx/*.cpp)
file(GLOB_RECURSE GTX_INLINE gli/gtx/*.inl)
file(GLOB_RECURSE GTX_HEADER gli/gtx/*.hpp)

source_group("KMG Spec" FILES  ${ROOT_KMG})
source_group("Text Files" FILES ${ROOT_TEXT} ${ROOT_MD})
source_group("Core Files" FILES ${CORE_SOURCE})
source_group("Core Files" FILES ${CORE_INLINE})
source_group("Core Files" FILES ${CORE_HEADER})
source_group("GTX Files" FILES ${GTX_SOURCE})
source_group("GTX Files" FILES ${GTX_INLINE})
source_group("GTX Files" FILES ${GTX_HEADER})


add_library(gli ${GLOW_LINK_TYPE} gli.cc
	${ROOT_SOURCE}    ${ROOT_INLINE}    ${ROOT_HEADER}
	${CORE_SOURCE}    ${CORE_INLINE}    ${CORE_HEADER}
	${GTX_SOURCE}     ${GTX_INLINE}     ${GTX_HEADER})
target_include_directories(gli PUBLIC ./)
target_include_directories(gli PRIVATE ../glm)
if (MSVC)
    target_compile_options(gli PUBLIC /MP)
else()
    target_compile_options(gli PRIVATE -Wall -Werror -Wno-unused-variable -std=c++11 -Wno-strict-aliasing)
endif()
