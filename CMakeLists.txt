cmake_minimum_required(VERSION 2.8)
project(duke)
# add specific modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake/")

# defining build 
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC DUKE_GIT_SHA1)
git_describe(DUKE_VERSION --all)

#
find_package(Threads)

# adding glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Do not build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "Do not build the GLFW tests programs"   FORCE)
add_subdirectory(dependencies/glfw)

# ZeroMQ
find_package(ZeroMQ REQUIRED)
include_directories(${ZEROMQ_INCLUDE_DIRS})

# Protobuf
if(NOT PROTOBUF_ROOT_DIR AND NOT $ENV{PROTOBUF_ROOT_DIR} STREQUAL "")
  set(PROTOBUF_ROOT_DIR $ENV{PROTOBUF_ROOT_DIR})
endif()
if(PROTOBUF_ROOT_DIR)
  find_library(PROTOBUF_LIBRARY NAMES protobuf HINTS ${PROTOBUF_ROOT_DIR} PATH_SUFFIXES lib lib64)
  find_path(PROTOBUF_INCLUDE_DIR google/protobuf/service.h PATHS ${PROTOBUF_ROOT_DIR}/include)
  find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc PATHS ${PROTOBUF_ROOT_DIR}/bin)
endif()
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)

#
include_directories(dependencies/concurrent_utils)
include_directories(dependencies/light_sequence_parser/include)
include_directories(SYSTEM dependencies/glfw/include)
include_directories(SYSTEM dependencies/glm)
include_directories(src)

# clang && macosx
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# c++11
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# clang && debug adds address sanitizer
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND NOT APPLE)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
endif()

# duke
add_subdirectory(src/duke)

# arnold driver
add_subdirectory(src/arnold_driver)

# tests
if(EXISTS "${CMAKE_SOURCE_DIR}/dependencies/gtest-1.7.0/")
add_subdirectory(dependencies/gtest-1.7.0)
add_subdirectory(test)
else()
MESSAGE(STATUS "Could NOT find gtest (missing: 'dependencies/gtest-1.7.0/' folder) : tests disabled
To enable tests
* download https://googletest.googlecode.com/files/gtest-1.7.0.zip
* unzip into 'dependencies/gtest-1.7.0/'")
endif()
