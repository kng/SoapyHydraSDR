###################################################
# Build Soapy SDR support module for HydraSDR Devices
###################################################

cmake_minimum_required(VERSION 2.8.12)

project(SoapyHydraSDR CXX)

find_package(SoapySDR "0.4.0" NO_MODULE REQUIRED)
if(NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(LibHYDRASDR REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LibHYDRASDR_INCLUDE_DIRS})

#enable c++11 features
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    #C++11 is a required language feature for this project
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_STD_CXX11)
    if(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()
    #Thread support enabled (not the same as -lpthread)
    list(APPEND HYDRASDR_LIBS -pthread)
    #disable warnings for unused parameters
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wc++11-extensions")
endif(APPLE)

# IF (APPLE)
#  ADD_DEFINITIONS(
#    -D__MACOSX_CORE__
#  )
# FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
#SET (HYDRASDR_LIBS ${COREFOUNDATION_LIBRARY} ${HYDRASDR_LIBS} )
# ENDIF (APPLE)

list(APPEND HYDRASDR_LIBS ${LibHYDRASDR_LIBRARIES})

SOAPY_SDR_MODULE_UTIL(
    TARGET hydrasdrSupport
    SOURCES
    SoapyHydraSDR.hpp
        Registration.cpp
        Settings.cpp
        Streaming.cpp
    LIBRARIES
    ${HYDRASDR_LIBS}
)
