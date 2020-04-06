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

include(${SWIG_USE_FILE})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE_DIR})
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
        pyfmm)
