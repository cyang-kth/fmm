cmake_minimum_required( VERSION 3.5.1)

project(fmm-contrib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

findFMM()
include_directories(${FMM_HEADER})

find_package(FMM REQUIRED)
if (FMM_FOUND)
    message(STATUS "FMM headers found at ${FMM_HEADER}")
    message(STATUS "FMM library found at ${FMM_LIBRARY}")
else()
    message(FATAL_ERROR "FMM Not Found!")
endif (FMM_FOUND)

add_executable(fmm-foo main.cpp)
target_link_libraries(fmm-foo ${FMM_LIBRARY})

install(TARGETS fmm-foo DESTINATION bin)
