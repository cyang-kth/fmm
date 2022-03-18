cmake_minimum_required( VERSION 3.5.1)

message(STATUS "CMAKE version ${CMAKE_VERSION}")

if (POLICY CMP0074)
    message(STATUS "Set CMP0074 state to NEW")
    cmake_policy(SET CMP0074 NEW)
endif()

if (POLICY CMP0086)
    message(STATUS "Set CMP0086 state to NEW")
    cmake_policy(SET CMP0086 NEW)
endif()

if (POLICY CMP0078)
    message(STATUS "Set CMP0078 state to NEW")
    cmake_policy(SET CMP0078 NEW)
endif()

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(fmm)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Conda)
if(CONDA_PREFIX)
  message(STATUS "Set CONDA_PREFIX ${CONDA_PREFIX}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CONDA_PREFIX})
  set(Python_FIND_STRATEGY LOCATION) # might want to prefer conda over system python
else()
  message(STATUS "Non conda exist, search library in default path")
endif()

find_package(GDAL 2.2 REQUIRED)
if (GDAL_FOUND)
  message(STATUS "GDAL headers found at ${GDAL_INCLUDE_DIR}")
  message(STATUS "GDAL library found at ${GDAL_LIBRARIES}")
  include_directories(${GDAL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "GDAL Not Found!")
endif (GDAL_FOUND)
link_libraries(${GDAL_LIBRARIES})

if (WIN32)
  find_package(Boost 1.56.0 REQUIRED serialization exception)
else ()
  find_package(Boost 1.56.0 REQUIRED serialization)
endif (WIN32)
if (Boost_FOUND)
  message(STATUS "Boost headers found at ${Boost_INCLUDE_DIR}")
  message(STATUS "Boost library found at ${Boost_LIBRARIES}")
  message(STATUS "Boost library version ${Boost_LIB_VERSION}")
else()
  message(FATAL_ERROR "Boost Not Found!")
endif (Boost_FOUND)
link_libraries(${Boost_LIBRARIES})


find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  message(STATUS "OpenMP_HEADERS found at ${OpenMP_CXX_INCLUDE_DIRS}")
  message(STATUS "OpenMP_CXX_LIBRARIES found at ${OpenMP_CXX_LIBRARIES}")
endif()
link_libraries(${OpenMP_CXX_LIBRARIES})

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
