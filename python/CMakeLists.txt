cmake_minimum_required( VERSION 3.5.1)

if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
if (POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
endif()
if (POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
endif()

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(fmm-python)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
  -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF")
set(CMAKE_CXX_STANDARD 11)

# Make sure the swig package is loaded.
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
if (SWIG_FOUND)
  message(STATUS "Swig version is ${SWIG_VERSION}")
else()
  message(FATAL_ERROR "Python library not found!\n")
endif()

find_package(GDAL 2.2 REQUIRED)
if (GDAL_FOUND)
  message(STATUS "GDAL headers found at ${GDAL_INCLUDE_DIR}")
  message(STATUS "GDAL library found at ${GDAL_LIBRARIES}")
  include_directories(${GDAL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "GDAL Not Found!")
endif (GDAL_FOUND)

find_package(PythonLibs 2.7 REQUIRED)
if (PYTHONLIBS_FOUND)
  message(STATUS "Python header found at ${PYTHON_INCLUDE_DIRS}")
  message(STATUS "Python library found at ${PYTHON_LIBRARIES}")
  include_directories(${PYTHON_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Python library not found!\n")
endif()

find_package(Boost 1.54.0 REQUIRED serialization)
if (Boost_FOUND)
  message(STATUS "Boost headers found at ${Boost_INCLUDE_DIR}")
  message(STATUS "Boost library found at ${Boost_LIBRARIES}")
else()
  message(FATAL_ERROR "Boost Not Found!")
endif (Boost_FOUND)

include_directories(../third_party)
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

target_link_libraries(pyfmm ${GDAL_LIBRARIES} ${Boost_LIBRARIES})
# Add the target.
if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
  message(STATUS "Using swig add module")
  SWIG_ADD_MODULE(fmm python fmm.i)
else()
  message(STATUS "Using swig add library")
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
        pyfmm)
