cmake_minimum_required(VERSION 3.0)
project(j.leapmotion)

if(APPLE)
    find_library(LEAPMOTION_LIBRARY  
    			NAMES libLeap.dylib Leap 
    			PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
elseif(WIN32)
    if(CMAKE_CL_64)
    find_library(LEAPMOTION_LIBRARY 
    			NAMES Leap.lib
    			PATHS ${LEAPSDK}/lib/x64)
    else()
    find_library(LEAPMOTION_LIBRARY 
    			NAMES Leap.lib
    			PATHS ${LEAPSDK}/lib/x86)
    endif()
endif()

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
