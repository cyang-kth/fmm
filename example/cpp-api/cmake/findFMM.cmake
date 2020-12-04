if (NOT FMM_INCLUDE_DIR)
    find_path(FMM_INCLUDE_DIR fmm/fmm-api.hpp ${CMAKE_PREFIX_PATH} PATH_SUFFIXES include)
else ()
    file(TO_CMAKE_PATH "${FMM_INCLUDE_DIR}" FMM_INCLUDE_DIR)
endif ()

if (NOT FMM_LIBRARY)
    find_library(FMM_LIBRARY NAMES fmmlib NAMES_PER_DIR PATH_SUFFIXES lib)
else ()
    file(TO_CMAKE_PATH "${FMM_LIBRARY}" FMM_LIBRARY)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMM
                                  REQUIRED_VARS FMM_LIBRARY FMM_INCLUDE_DIR)
