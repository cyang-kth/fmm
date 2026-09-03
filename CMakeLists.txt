cmake_minimum_required(VERSION 3.16...4.4)

message(STATUS "CMAKE version ${CMAKE_VERSION}")

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(fmm LANGUAGES C CXX)

# Default to an optimised build. An unknown build type would silently drop
# the optimisation flags, so anything but the standard types is reset too.
set(FMM_KNOWN_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
if(NOT CMAKE_BUILD_TYPE IN_LIST FMM_KNOWN_BUILD_TYPES)
  if(CMAKE_BUILD_TYPE)
    message(STATUS "Unknown build type '${CMAKE_BUILD_TYPE}', using Release")
  endif()
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type ${CMAKE_BUILD_TYPE}")

# Boost.Geometry >= 1.75 requires C++14 and GDAL >= 3.9 headers require
# C++17, so C++17 is the floor for building against a current toolchain.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compile-time log level of spdlog. Messages below this level are removed
# from the binary entirely (SPDLOG_LEVEL_TRACE keeps everything and lets the
# CLI --log_level flag switch them on at run time).
set(FMM_SPDLOG_ACTIVE_LEVEL "SPDLOG_LEVEL_TRACE" CACHE STRING
  "Compile-time spdlog level (SPDLOG_LEVEL_TRACE, SPDLOG_LEVEL_DEBUG, SPDLOG_LEVEL_INFO, ...)")
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=${FMM_SPDLOG_ACTIVE_LEVEL})

# Optimise for the CPU of the build machine. The resulting binaries only run
# on CPUs supporting the same instruction set extensions, so this is OFF by
# default.
option(FMM_NATIVE_ARCH "Compile with -march=native" OFF)
if(FMM_NATIVE_ARCH)
  add_compile_options(-march=native)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Conda)
if(CONDA_PREFIX)
  message(STATUS "Set CONDA_PREFIX ${CONDA_PREFIX}")
  list(APPEND CMAKE_PREFIX_PATH ${CONDA_PREFIX})
else()
  message(STATUS "Non conda exist, search library in default path")
endif()

find_package(GDAL 2.2 REQUIRED)
message(STATUS "GDAL ${GDAL_VERSION} headers found at ${GDAL_INCLUDE_DIRS}")
message(STATUS "GDAL library found at ${GDAL_LIBRARIES}")

if (WIN32)
  find_package(Boost 1.70 REQUIRED COMPONENTS serialization exception)
else ()
  find_package(Boost 1.70 REQUIRED COMPONENTS serialization)
endif (WIN32)
message(STATUS "Boost ${Boost_VERSION} headers found at ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library found at ${Boost_LIBRARIES}")
if(TARGET Boost::headers)
  set(FMM_BOOST_HEADERS Boost::headers)
else()
  set(FMM_BOOST_HEADERS Boost::boost)
endif()

find_package(OpenMP REQUIRED)
message(STATUS "OpenMP ${OpenMP_CXX_VERSION} flags ${OpenMP_CXX_FLAGS}")

# Every target below (object libraries, FMMLIB, executables and the python
# module in the subdirectory) picks up include directories, compile flags and
# link libraries from these imported targets.
link_libraries(GDAL::GDAL Boost::serialization ${FMM_BOOST_HEADERS}
  OpenMP::OpenMP_CXX)
if (WIN32)
  link_libraries(Boost::exception)
endif (WIN32)

### Set RPATH properties

set(CMAKE_SKIP_BUILD_RPATH FALSE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Build h3 objects

set(UNCONFIGURED_API_HEADER
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/h3/include/h3api.h.in)
set(CONFIGURED_API_HEADER
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/h3/include/h3api.h)
configure_file(${UNCONFIGURED_API_HEADER} ${CONFIGURED_API_HEADER})

# Creation of FMMLIB

set(FMM_INCLUDE_PATH
${CMAKE_CURRENT_SOURCE_DIR}/src
${CMAKE_CURRENT_SOURCE_DIR}/third_party
${CMAKE_CURRENT_SOURCE_DIR}/third_party/h3/include)

include_directories(${FMM_INCLUDE_PATH})

file(GLOB CoreGlob src/core/*.cpp)
file(GLOB AlgorithmGlob src/algorithm/*.cpp)
file(GLOB ConfigGlob src/config/*.cpp)
file(GLOB IOGlob src/io/*.cpp)
file(GLOB NetworkGlob src/network/*.cpp)
file(GLOB UtilGlob src/util/*.cpp)
file(GLOB MMGlob src/mm/*.cpp)
file(GLOB FMMGlob src/mm/fmm/*.cpp)
file(GLOB STMATCHGlob src/mm/stmatch/*.cpp)
file(GLOB h3Glob third_party/h3/lib/*.c)

add_library(CORE OBJECT ${CoreGlob})
add_library(ALGORITHM OBJECT ${AlgorithmGlob})
add_library(CONFIG OBJECT ${ConfigGlob})
add_library(IO OBJECT ${IOGlob})
add_library(UTIL OBJECT ${UtilGlob})
add_library(NETWORK OBJECT ${NetworkGlob})
add_library(MM_OBJ OBJECT ${MMGlob})
add_library(FMM_OBJ OBJECT ${FMMGlob})
add_library(STMATCH_OBJ OBJECT ${STMATCHGlob})
add_library(H3_OBJ OBJECT ${h3Glob})

add_library(FMMLIB SHARED
  $<TARGET_OBJECTS:MM_OBJ>
  $<TARGET_OBJECTS:FMM_OBJ>
  $<TARGET_OBJECTS:STMATCH_OBJ>
  $<TARGET_OBJECTS:CORE>
  $<TARGET_OBJECTS:CONFIG>
  $<TARGET_OBJECTS:ALGORITHM>
  $<TARGET_OBJECTS:UTIL>
  $<TARGET_OBJECTS:NETWORK>
  $<TARGET_OBJECTS:IO>
  $<TARGET_OBJECTS:H3_OBJ>)

# Add executables

add_executable(fmm src/app/fmm.cpp)
target_link_libraries(fmm FMMLIB)

add_executable(ubodt_gen src/app/ubodt_gen_app.cpp)
target_link_libraries(ubodt_gen FMMLIB)

add_executable(stmatch src/app/stmatch.cpp)
target_link_libraries(stmatch FMMLIB)

add_executable(h3mm src/app/h3mm.cpp)
target_link_libraries(h3mm FMMLIB)

message(STATUS "Installation folder ${CMAKE_INSTALL_PREFIX}")

install(TARGETS FMMLIB LIBRARY DESTINATION lib)

install(TARGETS fmm ubodt_gen stmatch h3mm DESTINATION bin)

if(FMM_INSTALL_HEADER)
  message(STATUS "Install fmm headers")
  set(FMM_DIST_INCLUDE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/src/io
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mm
  ${CMAKE_CURRENT_SOURCE_DIR}/src/network
  ${CMAKE_CURRENT_SOURCE_DIR}/src/python
  ${CMAKE_CURRENT_SOURCE_DIR}/src/util
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cxxopts
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fiboheap
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/h3
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog)
  set(FMM_API_HEADER
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fmm-api.hpp
  )
  install(
      DIRECTORY
      ${FMM_DIST_INCLUDE_PATH}
      DESTINATION
      "include/fmm"
      FILES_MATCHING
      PATTERN "*.hpp"
      PATTERN "*.h"
  )
  install(
      FILES
      ${FMM_API_HEADER}
      DESTINATION
      "include/fmm"
  )
else()
  message(STATUS "Not install fmm headers")
endif()

message(STATUS "Add python cmake information")

add_subdirectory(python)
# add_subdirectory(test EXCLUDE_FROM_ALL)
