cmake_minimum_required( VERSION 3.5.1)

# Prevent in source build
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Skip resetting RPATH to empty
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

project(fmm)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
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
  message(STATUS "OpenMP_CXX_LIBRARIES found at ${OpenMP_CXX_LIBRARIES}")
endif()

include_directories(third_party)
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

add_executable(fmm src/app/fmm.cpp
        $<TARGET_OBJECTS:MM_OBJ>
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:CONFIG>
        $<TARGET_OBJECTS:ALGORITHM>
        $<TARGET_OBJECTS:UTIL>
        $<TARGET_OBJECTS:IO>
        $<TARGET_OBJECTS:NETWORK>
        $<TARGET_OBJECTS:FMM_OBJ>)
target_link_libraries(fmm ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
        ${OpenMP_CXX_LIBRARIES})

add_executable(ubodt_gen src/app/ubodt_gen_app.cpp
        $<TARGET_OBJECTS:MM_OBJ>
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:CONFIG>
        $<TARGET_OBJECTS:ALGORITHM>
        $<TARGET_OBJECTS:UTIL>
        $<TARGET_OBJECTS:IO>
        $<TARGET_OBJECTS:NETWORK>
        $<TARGET_OBJECTS:FMM_OBJ>)
target_link_libraries(ubodt_gen ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
        ${OpenMP_CXX_LIBRARIES})

add_executable(stmatch src/app/stmatch.cpp
        $<TARGET_OBJECTS:MM_OBJ>
        $<TARGET_OBJECTS:STMATCH_OBJ>
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:CONFIG>
        $<TARGET_OBJECTS:ALGORITHM>
        $<TARGET_OBJECTS:UTIL>
        $<TARGET_OBJECTS:IO>
        $<TARGET_OBJECTS:NETWORK>)
target_link_libraries(stmatch ${GDAL_LIBRARIES} ${OpenMP_CXX_LIBRARIES})

install(TARGETS fmm ubodt_gen stmatch DESTINATION bin)
