cmake_minimum_required(VERSION 3.0)
project(j.leapmotion)

find_library(LEAPMOTION_LIBRARY 
			NAMES libLeap.dylib 
			PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/util)

set(PROJECT_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/j.leapmotion.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/util/LeapScene.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/util/LeapUtil.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/CMake/MaxExternalCommon.cmake)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(j.leapmotion ${LEAPMOTION_LIBRARY})
