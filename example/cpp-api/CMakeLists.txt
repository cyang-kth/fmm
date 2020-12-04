cmake_minimum_required( VERSION 3.5.1)

project(fmm-contrib)

set(CMAKE_CXX_FLAGS "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Conda)
if(CONDA_PREFIX)
  message(STATUS "Set CONDA_PREFIX ${CONDA_PREFIX}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CONDA_PREFIX})
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

find_package(Boost 1.56.0 REQUIRED serialization)
if (Boost_FOUND)
  message(STATUS "Boost headers found at ${Boost_INCLUDE_DIR}")
  message(STATUS "Boost library found at ${Boost_LIBRARIES}")
  message(STATUS "Boost library version ${Boost_LIB_VERSION}")
else()
  message(FATAL_ERROR "Boost Not Found!")
endif (Boost_FOUND)

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  message(STATUS "OpenMP_HEADERS found at ${OpenMP_CXX_INCLUDE_DIRS}")
  message(STATUS "OpenMP_CXX_LIBRARIES found at ${OpenMP_CXX_LIBRARIES}")
endif()

find_package(FMM REQUIRED)
if (FMM_FOUND)
    message(STATUS "FMM headers found at ${FMM_INCLUDE_DIR}")
    message(STATUS "FMM library found at ${FMM_LIBRARY}")
    include_directories(${FMM_INCLUDE_DIR})
    include_directories(${FMM_INCLUDE_DIR}/fmm)
else()
    message(FATAL_ERROR "FMM Not Found!")
endif (FMM_FOUND)

### Set RPATH properties

set(CMAKE_SKIP_BUILD_RPATH FALSE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(fmm_print_network main.cpp)
target_link_libraries(fmm_print_network
  ${FMM_LIBRARY} ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
  ${OpenMP_CXX_LIBRARIES})

install(TARGETS fmm_print_network DESTINATION bin)
