add_executable(algorithm_test algorithm_test.cpp
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:ALGORITHM>)
target_link_libraries(algorithm_test ${GDAL_LIBRARIES} ${Boost_LIBRARIES})

add_executable(fmm_test fmm_test.cpp
        $<TARGET_OBJECTS:MM_OBJ>
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:CONFIG>
        $<TARGET_OBJECTS:ALGORITHM>
        $<TARGET_OBJECTS:UTIL>
        $<TARGET_OBJECTS:IO>
        $<TARGET_OBJECTS:NETWORK>
        $<TARGET_OBJECTS:FMM_OBJ>)
target_link_libraries(fmm_test ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
        ${OpenMP_CXX_LIBRARIES} ${OSMIUM_LIBRARIES})

add_executable(network_graph_test network_graph_test.cpp
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:CONFIG>
        $<TARGET_OBJECTS:ALGORITHM>
        $<TARGET_OBJECTS:UTIL>
        $<TARGET_OBJECTS:IO>
        $<TARGET_OBJECTS:NETWORK>)
target_link_libraries(network_graph_test ${GDAL_LIBRARIES} ${Boost_LIBRARIES}
        ${OpenMP_CXX_LIBRARIES} ${OSMIUM_LIBRARIES})

add_executable(network_test network_test.cpp
        $<TARGET_OBJECTS:CORE>
        $<TARGET_OBJECTS:CONFIG>
        $<TARGET_OBJECTS:ALGORITHM>
        $<TARGET_OBJECTS:UTIL>
        $<TARGET_OBJECTS:IO>
        $<TARGET_OBJECTS:NETWORK>)
target_link_libraries(network_test ${GDAL_LIBRARIES} ${OpenMP_CXX_LIBRARIES}
${OSMIUM_LIBRARIES})

add_custom_target(tests
	DEPENDS algorithm_test network_test network_graph_test fmm_test)
