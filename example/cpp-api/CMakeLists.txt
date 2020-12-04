cmake_minimum_required( VERSION 3.5.1)

project(fmm-contrib)

set(CMAKE_CXX_FLAGS "-O3 -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE")
set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(FMM REQUIRED)
if (FMM_FOUND)
    message(STATUS "FMM headers found at ${FMM_INCLUDE_DIR}")
    message(STATUS "FMM library found at ${FMM_LIBRARY}")
    include_directories(${FMM_INCLUDE_DIR})
    include_directories(${FMM_INCLUDE_DIR}/fmm)
else()
    message(FATAL_ERROR "FMM Not Found!")
endif (FMM_FOUND)

add_executable(fmm_print_network main.cpp)
target_link_libraries(fmm_print_network ${FMM_LIBRARY})

install(TARGETS fmm_print_network DESTINATION bin)
