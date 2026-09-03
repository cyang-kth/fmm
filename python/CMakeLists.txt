# Make sure the swig package is loaded.
find_package(SWIG 4.0 REQUIRED)
include(${SWIG_USE_FILE})
message(STATUS "Swig version is ${SWIG_VERSION}")

# Development.Module locates the headers needed to build an extension module
# without requiring a shared libpython (which is the recommended way of
# building CPython extensions and also works for statically linked Pythons).
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
message(STATUS "Python ${Python3_VERSION} interpreter found at ${Python3_EXECUTABLE}")
message(STATUS "Python header found at ${Python3_INCLUDE_DIRS}")

set(FMM_PYTHON_INSTALL_DIR "${Python3_SITEARCH}" CACHE PATH
  "Directory where the fmm python module is installed")
message(STATUS "Python packages ${FMM_PYTHON_INSTALL_DIR}")

set_property(SOURCE ${PROJECT_SOURCE_DIR}/python/fmm.i
  PROPERTY CPLUSPLUS ON)

swig_add_library(pyfmm
  LANGUAGE python
  SOURCES ${PROJECT_SOURCE_DIR}/python/fmm.i)
set_target_properties(pyfmm PROPERTIES
  OUTPUT_NAME fmm
  SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
target_include_directories(pyfmm PRIVATE ${FMM_INCLUDE_PATH})
target_link_libraries(pyfmm PRIVATE FMMLIB Python3::Module)

install(TARGETS pyfmm LIBRARY DESTINATION ${FMM_PYTHON_INSTALL_DIR})
install(FILES $<TARGET_PROPERTY:pyfmm,SWIG_SUPPORT_FILES>
  DESTINATION ${FMM_PYTHON_INSTALL_DIR})
