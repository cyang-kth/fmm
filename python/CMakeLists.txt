cmake_minimum_required( VERSION 3.5.1)

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(fmm-python)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
  -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF")
set(CMAKE_CXX_STANDARD 11)

# Make sure the swig package is loaded.
find_package(SWIG REQUIRED)
find_package(GDAL 2.2 REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(Boost 1.54.0 REQUIRED serialization)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

list(APPEND CMAKE_MODULE_PATH
     "${PARENT_DIR}/third_party/libosmium/cmake")

if(NOT OSMIUM_INCLUDE_DIR)
  set(OSMIUM_INCLUDE_DIR "${PARENT_DIR}/third_party/libosmium/include")
endif()
if(NOT PROTOZERO_INCLUDE_DIR)
  set(PROTOZERO_INCLUDE_DIR "${PARENT_DIR}/third_party/protozero/include")
endif()

find_package(Osmium REQUIRED COMPONENTS io)
if(OSMIUM_FOUND)
  message(STATUS "OSMium found at ${OSMIUM_INCLUDE_DIRS}")
  message(STATUS "OSMium library found at ${OSMIUM_LIBRARIES}")
  include_directories(${OSMIUM_INCLUDE_DIR})
else()
  message(WARNING "Libosmium not found!\n")
endif()

include(${SWIG_USE_FILE})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE_DIR})
include_directories(../third_party)
include_directories(../third_party/libosmium/include)
include_directories(../third_party/protozero/include)
include_directories(../src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set the flags.

set(CMAKE_CXX_FLAGS "-O3")

# Set the properties for the interface file.
set_source_files_properties(fmm.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(fmm.i PROPERTIES SWIG_FLAGS "")

file(GLOB CoreGlob ../src/core/*.cpp)
file(GLOB AlgorithmGlob ../src/algorithm/*.cpp)
file(GLOB ConfigGlob ../src/config/*.cpp)
file(GLOB NetworkGlob ../src/network/*.cpp)
file(GLOB UtilGlob ../src/util/*.cpp)
file(GLOB MMGlob ../src/mm/*.cpp)
file(GLOB FMMGlob ../src/mm/fmm/fmm_algorithm.cpp ../src/mm/fmm/ubodt.cpp)
file(GLOB STMATCHGlob ../src/mm/stmatch/stmatch_algorithm.cpp)

add_library(pyfmm SHARED ${CoreGlob}
${AlgorithmGlob}
${ConfigGlob}
${UtilGlob}
${NetworkGlob}
${MMGlob}
${FMMGlob}
${STMATCHGlob})

target_link_libraries(pyfmm ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
${OSMIUM_LIBRARIES})
# Add the target.
if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
  SWIG_ADD_MODULE(fmm python fmm.i)
else()
  SWIG_ADD_LIBRARY(fmm
    LANGUAGE python
    SOURCES fmm.i)
endif()

# -----------------------------------------------------------------------------
# LINK
# -----------------------------------------------------------------------------
message( STATUS "Creating makefiles for system: ${CMAKE_SYSTEM}")

swig_link_libraries(fmm
        ${PYTHON_LIBRARIES} ${GDAL_LIBRARIES}  ${Boost_LIBRARIES}
        ${OSMIUM_LIBRARIES}
        pyfmm)
