# Make sure the swig package is loaded.
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
if (SWIG_FOUND)
  message(STATUS "Swig version is ${SWIG_VERSION}")
else()
  message(FATAL_ERROR "Swig not found!\n")
endif()

find_package(Python COMPONENTS Interpreter Development)
if (Python_FOUND)
  message(STATUS "Python header found at ${Python_INCLUDE_DIRS}")
  message(STATUS "Python library found at ${Python_LIBRARIES}")
  message(STATUS "Python packages ${Python_SITELIB}")
  include_directories(${Python_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Python library not found!\n")
endif()

set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES CPLUSPLUS ON)

if (${CMAKE_VERSION} VERSION_LESS 3.13.0)
  message(STATUS "Using swig add module")
  SWIG_ADD_MODULE(fmm python ${PROJECT_SOURCE_DIR}/python/fmm.i)
  swig_link_libraries(fmm FMMLIB ${Python_LIBRARIES})
  install(TARGETS ${SWIG_MODULE_fmm_REAL_NAME} DESTINATION ${Python_SITELIB})
else()
  message(STATUS "Using swig add library")
  SWIG_ADD_LIBRARY(pyfmm
    LANGUAGE python
    SOURCES ${PROJECT_SOURCE_DIR}/python/fmm.i)
  set_property(TARGET pyfmm PROPERTY OUTPUT_NAME fmm)
  target_link_libraries(pyfmm FMMLIB Python::Module)
  install(TARGETS pyfmm DESTINATION ${Python_SITELIB})
endif()

install(FILES ${CMAKE_BINARY_DIR}/python/fmm.py DESTINATION ${Python_SITELIB})
