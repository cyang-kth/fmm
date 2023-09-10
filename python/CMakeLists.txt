# Make sure the swig package is loaded.
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
if (SWIG_FOUND)
  message(STATUS "Swig version is ${SWIG_VERSION}")
else()
  message(FATAL_ERROR "Swig not found!\n")
endif()

find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
if (PYTHONLIBS_FOUND)
  message(STATUS "Python header found at ${PYTHON_INCLUDE_DIRS}")
  message(STATUS "Python library found at ${PYTHON_LIBRARIES}")
  include_directories(${PYTHON_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Python library not found!\n")
endif()

execute_process(
  COMMAND python -c "import site; print(site.getsitepackages())"
  OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Python packages ${PYTHON_SITE_PACKAGES}")

set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES CPLUSPLUS ON)

if (${CMAKE_VERSION} VERSION_LESS 3.13.0)
  message(STATUS "Using swig add module")
  SWIG_ADD_MODULE(fmm python ${PROJECT_SOURCE_DIR}/python/fmm.i)
  swig_link_libraries(fmm FMMLIB ${PYTHON_LIBRARIES})
  install(TARGETS ${SWIG_MODULE_fmm_REAL_NAME} DESTINATION include ${PYTHON_SITE_PACKAGES})
else()
  message(STATUS "Using swig add library ${DESTINATION}")
  SWIG_ADD_LIBRARY(pyfmm
    LANGUAGE python
    SOURCES ${PROJECT_SOURCE_DIR}/python/fmm.i)
  set_property(TARGET pyfmm PROPERTY OUTPUT_NAME fmm)
  swig_link_libraries(pyfmm FMMLIB ${PYTHON_LIBRARIES})
  install(TARGETS pyfmm DESTINATION include ${PYTHON_SITE_PACKAGES})
endif()

install(FILES ${CMAKE_BINARY_DIR}/python/fmm.py DESTINATION include ${PYTHON_SITE_PACKAGES})
