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

file(GLOB PYTHONFMMGlob
${PROJECT_SOURCE_DIR}/src/mm/fmm/fmm_algorithm.cpp
${PROJECT_SOURCE_DIR}/src/mm/fmm/ubodt.cpp)
file(GLOB PYTHONSTMATCHGlob
${PROJECT_SOURCE_DIR}/src/mm/stmatch/stmatch_algorithm.cpp)

add_library(PYTHONFMM_OBJ OBJECT ${PYTHONFMMGlob})
add_library(PYTHONSTMATCH_OBJ OBJECT ${PYTHONSTMATCHGlob})

set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES SWIG_FLAGS "")

add_library(pyfmmlib SHARED
$<TARGET_OBJECTS:MM_OBJ>
$<TARGET_OBJECTS:PYTHONFMM_OBJ>
$<TARGET_OBJECTS:PYTHONSTMATCH_OBJ>
$<TARGET_OBJECTS:CORE>
$<TARGET_OBJECTS:CONFIG>
$<TARGET_OBJECTS:ALGORITHM>
$<TARGET_OBJECTS:UTIL>
$<TARGET_OBJECTS:NETWORK>)

# set_property(TARGET pyfmmlib PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(pyfmmlib ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
${OSMIUM_LIBRARIES})
# Add the target.
if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
  message(STATUS "Using swig add module")
  SWIG_ADD_MODULE(fmm python ${PROJECT_SOURCE_DIR}/python/fmm.i)
else()
  message(STATUS "Using swig add library")
  SWIG_ADD_LIBRARY(fmm
    LANGUAGE python
    SOURCES ${PROJECT_SOURCE_DIR}/python/fmm.i)
endif()

swig_link_libraries(fmm
        ${PYTHON_LIBRARIES}
        ${GDAL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${OSMIUM_LIBRARIES}
        pyfmmlib)
