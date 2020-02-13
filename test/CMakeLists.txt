cmake_minimum_required( VERSION 3.5.1)

message(STATUS "Configuring unit tests")

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(fmm_test)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 11)

find_package(GDAL REQUIRED)
if (GDAL_FOUND)
  message(STATUS "GDAL headers found at ${GDAL_INCLUDE_DIR}")
  message(STATUS "GDAL library found at ${GDAL_LIBRARIES}")
  include_directories(${GDAL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "GDAL Not Found!")
endif (GDAL_FOUND)

include_directories(../include)

add_executable(example example.cpp)
target_link_libraries(example ${GDAL_LIBRARIES})

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach( test_file ${TEST_SOURCES} )
  # I used a simple string replace, to cut off .cpp.
  string( REPLACE ".cpp" "" filename ${test_file})
  get_filename_component(test_name ${filename} NAME)
  add_executable(${test_name} ${test_file})
  target_link_libraries(${test_name} ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
    ${OpenMP_CXX_LIBRARIES})
  install(TARGETS ${test_name} DESTINATION bin)
endforeach( test_file ${TEST_SOURCES} )
