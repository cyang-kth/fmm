cmake_minimum_required( VERSION 3.5.1)

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Skip resetting RPATH to empty
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

project(fmm)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 11)

find_package(GDAL 2.2 REQUIRED)
if (GDAL_FOUND)
  message(STATUS "GDAL headers found at ${GDAL_INCLUDE_DIR}")
  message(STATUS "GDAL library found at ${GDAL_LIBRARIES}")
  include_directories(${GDAL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "GDAL Not Found!")
endif (GDAL_FOUND)

find_package(Boost 1.54.0 REQUIRED serialization)
if (Boost_FOUND)
  message(STATUS "Boost headers found at ${Boost_INCLUDE_DIR}")
  message(STATUS "Boost library found at ${Boost_LIBRARIES}")
else()
  message(FATAL_ERROR "Boost Not Found!")
endif (Boost_FOUND)

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(include)

file(GLOB APP_SOURCES app/*.cpp)
foreach( source_file ${APP_SOURCES} )
  # I used a simple string replace, to cut off .cpp.
  string( REPLACE ".cpp" "" filename ${source_file})
  get_filename_component(appname ${filename} NAME)
  add_executable(${appname} ${source_file})
  target_link_libraries(${appname} ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
    OpenMP::OpenMP_CXX)
  install(TARGETS ${appname} DESTINATION bin)
endforeach( source_file ${APP_SOURCES} )
