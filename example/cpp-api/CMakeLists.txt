cmake_minimum_required( VERSION 3.5.1)

project(fmm-contrib)

add_executable(fmm-foo main.cpp)
target_link_libraries(fmm-foo FMMLIB)

install(TARGETS fmm-foo DESTINATION bin)
