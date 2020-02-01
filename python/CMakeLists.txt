cmake_minimum_required( VERSION 3.5.1)

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(fmm-python)

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -fPIC")

# Make sure the swig package is loaded.
find_package(SWIG REQUIRED)
find_package(GDAL 2.2 REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(Boost 1.54.0 REQUIRED serialization)

include(${SWIG_USE_FILE})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE_DIR})
include_directories(../include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set the flags.

set(CMAKE_CXX_FLAGS "-O3")

# Set the properties for the interface file.
set_source_files_properties(fmm.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(fmm.i PROPERTIES SWIG_FLAGS "")

# Add the target.
swig_add_module(fmm python fmm.i)
# -----------------------------------------------------------------------------
# LINK
# -----------------------------------------------------------------------------
message( STATUS "Creating makefiles for system: ${CMAKE_SYSTEM}")

swig_link_libraries(fmm ${PYTHON_LIBRARIES} ${GDAL_LIBRARIES}  ${Boost_LIBRARIES})
