cmake_minimum_required(VERSION 3.8)
project(SDDK C CXX)

# allow {module}_ROOT variables to be set
cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

option(CUDA "Enable CUDA compilation" OFF)
option(ROCM "Enable ROCM compilation" OFF)

# TODO: implement cuda

if(CUDA AND ROCM)
	message(FATAL_ERROR "CUDA and ROCM must not be activated at the same time!")
endif()

if(CUDA)
	enable_language(CUDA)
	set(LOCAL_DEFINITIONS ${LOCAL_DEFINITIONS} -D__CUDA -D__GPU)
endif()

if(CUDA)
	set(LOCAL_SOURCES SDDK/GPU/fft_kernels.cu SDDK/GPU/checksum.cu SDDK/GPU/checksum.cu SDDK/GPU/scale_matrix.cu)
endif()

if(ROCM)
	set(LOCAL_SOURCES ${LOCAL_SOURCES} SDDK/GPU/rocfft_interface.cpp)
	set(LOCAL_SOURCES_HIP ${LOCAL_SOURCES_HIP} SDDK/GPU/fft_kernels.cpp)
endif()



find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)
find_package(HDF5 REQUIRED)
find_package(FFTW REQUIRED)
find_package(Scalapack REQUIRED)
find_package(BLAS REQUIRED)

add_executable(test1 test_fft_correctness_1.cpp ${LOCAL_SOURCES})

set(EXTERNAL_LIBRARIES
	${EXTERNAL_LIBRARIES}
	${FFTW_LIBRARIES}
	${HDF5_CXX_LIBRARIES}
	${OpenMP_CXX_LIBRARIES}
	${MPI_CXX_LIBRARIES}
	${Scalapack_LIBRARIES}
	${BLAS_LIBRARIES}
	)

set(EXTERNAL_INCLUDE_DIRS
	${EXTERNAL_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS}
	${FFTW_INCLUDE_DIRS}
	${MPI_CXX_INCLUDE_PATH}
	)

set(EXTERNAL_DEFINITIONS
	${EXTERNAL_DEFINITIONS}
	${HDF5_DEFINITIONS}
	)

set(EXTERNAL_COMPILE_FLAGS ${EXTERNAL_COMPILE_FLAGS} ${OpenMP_CXX_FLAGS})
set(EXTERNAL_LINK_FLAGS ${EXTERNAL_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS} ${BLAS_LINKER_FLAGS})

set(LOCAL_INCLUDE_DIRS utils SDDK SDDK/GPU ${CMAKE_CURRENT_SOURCE_DIR})

if(ROCM)
	find_package(ROCM REQUIRED COMPONENTS hip rocfft)
	set(LOCAL_DEFINITIONS ${LOCAL_DEFINITIONS} -D__ROCM -D__GPU)
	set(EXTERNAL_LIBRARIES
		${EXTERNAL_LIBRARIES}
		${ROCM_LIBRARIES}
		)
	set(EXTERNAL_INCLUDE_DIRS
		${EXTERNAL_INCLUDE_DIRS}
		${ROCM_INCLUDE_DIRS}
		)
	add_definitions(-D__HIP_PLATFORM_HCC__)

	# generate include flags
	set(FULL_PATH_INCLUDE_FLAGS)
	foreach(dir IN LISTS EXTERNAL_INCLUDE_DIRS LOCAL_INCLUDE_DIRS)
		if(NOT IS_ABSOLUTE ${dir})
			get_filename_component(dir ${dir} ABSOLUTE)
		endif()
		set(FULL_PATH_INCLUDE_FLAGS ${FULL_PATH_INCLUDE_FLAGS} -I${dir})
	endforeach()

	# generate full path to source files
	set(FULL_PATH_HIP_SOURCES)
	foreach(source IN LISTS LOCAL_SOURCES_HIP)
		if(NOT IS_ABSOLUTE ${source})
			get_filename_component(source ${source} ABSOLUTE)
		endif()
		set(FULL_PATH_HIP_SOURCES ${FULL_PATH_HIP_SOURCES} ${source})
	endforeach()

	# generate flags to use
	set(CUSTOM_COMMAND_FLAGS ${CMAKE_CXX_FLAGS})
	set(BUILD_TYPES DEBUG RELEASE RELWITHDEBINFO)
	if(CMAKE_BUILD_TYPE)
		foreach(type IN LISTS BUILD_TYPES)
			string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
			if(${type} MATCHES ${BUILD_TYPE_UPPER})
				set(CUSTOM_COMMAND_FLAGS ${CUSTOM_COMMAND_FLAGS} ${CMAKE_CXX_FLAGS_${type}})
			endif()
		endforeach()
	endif()

	if(NOT ROCM_HIPCC_EXECUTABLE)
		message(FATAL_ERROR "HIPCC executable not found!")
	endif()

	# compile GPU kernels with hipcc compiler
	add_custom_target(fft_kernels COMMAND ${ROCM_HIPCC_EXECUTABLE} ${FULL_PATH_HIP_SOURCES} --shared -fPIC -o ${PROJECT_BINARY_DIR}/libfft_kernels.so ${FULL_PATH_INCLUDE_FLAGS} ${CUSTOM_COMMAND_FLAGS}
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR} SOURCES ${FULL_PATH_HIP_SOURCES})
	add_library(fft_kernels_lib SHARED IMPORTED)
	set_target_properties(fft_kernels_lib PROPERTIES IMPORTED_LOCATION ${PROJECT_BINARY_DIR}/libfft_kernels.so)
	add_dependencies(fft_kernels_lib fft_kernels)
	set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} fft_kernels_lib)
endif()

target_include_directories(test1 PRIVATE  ${LOCAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS})
target_compile_options(test1 PRIVATE ${EXTERNAL_COMPILE_FLAGS})
target_link_libraries(test1 PRIVATE ${EXTERNAL_LIBRARIES})
# target_link_options(test1 PRIVATE ${EXTERNAL_LINK_FLAGS})
target_compile_definitions(test1 PRIVATE ${EXTERNAL_DEFINITIONS} ${LOCAL_DEFINITIONS})
