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
  COMMAND python -c "from setup import get_wheel_name;print(get_wheel_name())"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE WHEEL_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)

set(PYTHON_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
  ${CMAKE_CURRENT_SOURCE_DIR}/_version.py
  ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py)
file(COPY ${PYTHON_SOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES CPLUSPLUS ON)

if (${CMAKE_VERSION} VERSION_LESS 3.13.0)
  message(STATUS "Using swig add module")
  SWIG_ADD_MODULE(fmm python ${PROJECT_SOURCE_DIR}/python/fmm.i)
  swig_link_libraries(fmm FMMLIB ${PYTHON_LIBRARIES})
  add_custom_target(${WHEEL_NAME} ALL
    DEPENDS ${SWIG_MODULE_fmm_REAL_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py
    ${CMAKE_CURRENT_BINARY_DIR}/_version.py
    ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
    COMMAND python setup.py bdist_wheel
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building wheel")
else()
  message(STATUS "Using swig add library")
  SWIG_ADD_LIBRARY(pyfmm
    LANGUAGE python
    SOURCES ${PROJECT_SOURCE_DIR}/python/fmm.i)
  set_property(TARGET pyfmm PROPERTY OUTPUT_NAME fmm)
  swig_link_libraries(pyfmm FMMLIB ${PYTHON_LIBRARIES})
  add_custom_target(${WHEEL_NAME} ALL
    DEPENDS pyfmm
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py
    ${CMAKE_CURRENT_BINARY_DIR}/_version.py
    ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
    COMMAND python setup.py bdist_wheel
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building wheel")
endif()

install(CODE "execute_process(COMMAND python -m pip install --force-reinstall ${CMAKE_CURRENT_BINARY_DIR}/dist/${WHEEL_NAME})")
