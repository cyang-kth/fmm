if(DEFINED ENV{CONDA_PREFIX})
	set(CONDA_PREFIX $ENV{CONDA_PREFIX} CACHE PATH "")
else()
	# Assuming the active conda environment is on PATH, this finds the path of bin/ in the environment
	find_program(CONDA_EXECUTABLE conda
			PATHS ${CONDA_PREFIX}/bin ${CONDA_PREFIX}/Scripts ENV PATH
			)
	if(CONDA_EXECUTABLE)
		execute_process(COMMAND ${CONDA_EXECUTABLE} info --root
				OUTPUT_VARIABLE out_var
				OUTPUT_STRIP_TRAILING_WHITESPACE
				)
		execute_process(COMMAND ${CONDA_EXECUTABLE} info --envs
				)
	else()
		message(STATUS "\nNo conda environment found in PATH!\nPATH=$ENV{PATH}\n")
	endif()
  set(CONDA_PREFIX ${out_var})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Conda
		REQUIRED_VARS CONDA_PREFIX
		)
