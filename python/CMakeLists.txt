file(GLOB PYTHONFMMGlob
${PROJECT_SOURCE_DIR}/src/mm/fmm/fmm_algorithm.cpp
${PROJECT_SOURCE_DIR}/src/mm/fmm/ubodt.cpp)
file(GLOB PYTHONSTMATCHGlob
${PROJECT_SOURCE_DIR}/src/mm/stmatch/stmatch_algorithm.cpp)

add_library(PYTHONFMM_OBJ OBJECT ${PYTHONFMMGlob})
add_library(PYTHONSTMATCH_OBJ OBJECT ${PYTHONSTMATCHGlob})

set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(
${PROJECT_SOURCE_DIR}/python/fmm.i PROPERTIES SWIG_FLAGS "")
add_library(pyfmm SHARED
$<TARGET_OBJECTS:MM_OBJ>
$<TARGET_OBJECTS:PYTHONFMM_OBJ>
$<TARGET_OBJECTS:PYTHONSTMATCH_OBJ>
$<TARGET_OBJECTS:CORE>
$<TARGET_OBJECTS:CONFIG>
$<TARGET_OBJECTS:ALGORITHM>
$<TARGET_OBJECTS:UTIL>
$<TARGET_OBJECTS:NETWORK>)

target_link_libraries(pyfmm ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
${OSMIUM_LIBRARIES})
# Add the target.
if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
  SWIG_ADD_MODULE(fmm python ${PROJECT_SOURCE_DIR}/python/fmm.i)
else()
  SWIG_ADD_LIBRARY(fmm
    LANGUAGE python
    SOURCES ${PROJECT_SOURCE_DIR}/python/fmm.i)
endif()

swig_link_libraries(fmm
        ${PYTHON_LIBRARIES}
        ${GDAL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${OSMIUM_LIBRARIES}
        pyfmm)
