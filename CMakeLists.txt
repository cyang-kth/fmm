cmake_minimum_required( VERSION 2.5.1)

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
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(fmm)

set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_C_FLAGS "-std=c11")
set(CMAKE_CXX_FLAGS "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake"
     "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libosmium/cmake")

find_package(Conda)
if(CONDA_PREFIX)
  message(STATUS "Set CONDA_PREFIX ${CONDA_PREFIX}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CONDA_PREFIX})
else()
  message(STATUS "Non conda exist, search library in default path")
endif()

if(NOT OSMIUM_INCLUDE_DIR)
  set(OSMIUM_INCLUDE_DIR "third_party/libosmium/include")
endif()
if(NOT PROTOZERO_INCLUDE_DIR)
  set(PROTOZERO_INCLUDE_DIR "third_party/protozero/include")
endif()

find_package(Osmium REQUIRED COMPONENTS io)
if(OSMIUM_FOUND)
  message(STATUS "OSMium found at ${OSMIUM_INCLUDE_DIRS}")
  message(STATUS "OSMium library found at ${OSMIUM_LIBRARIES}")
  include_directories(${OSMIUM_INCLUDE_DIR})
else()
  message(FATAL_ERROR "Libosmium not found!\n")
endif()

find_package(GDAL 2.2 REQUIRED)
if (GDAL_FOUND)
  message(STATUS "GDAL headers found at ${GDAL_INCLUDE_DIR}")
  message(STATUS "GDAL library found at ${GDAL_LIBRARIES}")
  include_directories(${GDAL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "GDAL Not Found!")
endif (GDAL_FOUND)

find_package(Boost 1.56.0 REQUIRED serialization)
if (Boost_FOUND)
  message(STATUS "Boost headers found at ${Boost_INCLUDE_DIR}")
  message(STATUS "Boost library found at ${Boost_LIBRARIES}")
  message(STATUS "Boost library version ${Boost_LIB_VERSION}")
else()
  message(FATAL_ERROR "Boost Not Found!")
endif (Boost_FOUND)

if(APPLE)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang\$")
      set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_C_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY omp)
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang\$")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_CXX_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY omp)
  endif()
endif()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  message(STATUS "OpenMP_HEADERS found at ${OpenMP_CXX_INCLUDE_DIRS}")
  message(STATUS "OpenMP_CXX_LIBRARIES found at ${OpenMP_CXX_LIBRARIES}")
endif()

include_directories(third_party)
include_directories(third_party/libosmium/include)
include_directories(third_party/protozero/include)
include_directories(src)

file(GLOB CoreGlob src/core/*.cpp)
file(GLOB AlgorithmGlob src/algorithm/*.cpp)
file(GLOB ConfigGlob src/config/*.cpp)
file(GLOB IOGlob src/io/*.cpp)
file(GLOB NetworkGlob src/network/*.cpp)
file(GLOB UtilGlob src/util/*.cpp)
file(GLOB MMGlob src/mm/*.cpp)
file(GLOB FMMGlob src/mm/fmm/*.cpp)
file(GLOB STMATCHGlob src/mm/stmatch/*.cpp)

add_library(CORE OBJECT ${CoreGlob})
add_library(ALGORITHM OBJECT ${AlgorithmGlob})
add_library(CONFIG OBJECT ${ConfigGlob})
add_library(IO OBJECT ${IOGlob})
add_library(UTIL OBJECT ${UtilGlob})
add_library(NETWORK OBJECT ${NetworkGlob})
add_library(MM_OBJ OBJECT ${MMGlob})
add_library(FMM_OBJ OBJECT ${FMMGlob})
add_library(STMATCH_OBJ OBJECT ${STMATCHGlob})

# Build h3 objects

set(UNCONFIGURED_API_HEADER third_party/h3/src/h3lib/include/h3api.h.in)
set(CONFIGURED_API_HEADER third_party/h3/src/h3lib/include/h3api.h)
configure_file(${UNCONFIGURED_API_HEADER} ${CONFIGURED_API_HEADER})
message(STATUS "Copy h3 header to ${CONFIGURED_API_HEADER}")
set(H3_INCLUDE_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/third_party/h3/src/h3lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/h3/src/h3lib/include)

file(GLOB h3Glob third_party/h3/src/h3lib/lib/*.c)
add_library(H3_OBJ OBJECT ${h3Glob})
target_include_directories(H3_OBJ PUBLIC ${H3_INCLUDE_PATH})

### Set RPATH properties

set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Creation of FMMLIB
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
target_link_libraries(FMMLIB
          ${OSMIUM_LIBRARIES}
          ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
          ${OpenMP_CXX_LIBRARIES})
install(TARGETS FMMLIB DESTINATION lib)

# Add executables

add_executable(fmm src/app/fmm.cpp)
target_link_libraries(fmm FMMLIB)

add_executable(ubodt_gen src/app/ubodt_gen_app.cpp)
target_link_libraries(ubodt_gen FMMLIB)

add_executable(stmatch src/app/stmatch.cpp)
target_link_libraries(stmatch FMMLIB)

add_executable(h3mm src/app/h3mm.cpp)
target_include_directories(
  h3mm PUBLIC ${H3_INCLUDE_PATH})

target_link_libraries(h3mm FMMLIB)

message(STATUS "Installation folder ${CMAKE_INSTALL_PREFIX}")

install(TARGETS fmm ubodt_gen stmatch h3mm DESTINATION bin)

add_subdirectory(python)
add_subdirectory(test EXCLUDE_FROM_ALL)
