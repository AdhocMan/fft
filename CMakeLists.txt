cmake_minimum_required(VERSION 3.8)
project(SDDK C CXX)

# allow {module}_ROOT variables to be set
cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

option(CUDA "Enable CUDA compilation" OFF)
option(ROCM "Enable ROCM compilation" OFF)

# TODO: implement cuda
if(CUDA)
	message(FATAL_ERROR "CUDA compilation not yet implemented!")
endif()

if(CUDA AND ROCM)
	message(FATAL_ERROR "CUDA and ROCM must not be activated at the same time!")
endif()

if(CUDA)
	enable_language(CUDA)
	set(LOCAL_DEFINITIONS ${LOCAL_DEFINITIONS} -D__CUDA -D__GPU)
endif()

if(ROCM)
	find_package(ROCM REQUIRED COMPONENTS hip rocfft)
	set(LOCAL_DEFINITIONS ${LOCAL_DEFINITIONS} -D__ROCM -D__GPU)
	set(EXTERNAL_LIBRARIES
		${ROCM_LIBRARIES}
		)
	set(EXTERNAL_INCLUDE_DIRS
		${ROCM_INCLUDE_DIRS}
		)
	# add_definitions(-D__HIP_PLATFORM_HCC__)

	set(SOURCE_FILES ${SOURCE_FILES} SDDK/GPU/fft_kernels.cpp)
endif()


find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)
find_package(HDF5 REQUIRED)
find_package(FFTW REQUIRED)
find_package(Scalapack REQUIRED)

set(EXTERNAL_LIBRARIES
	${EXTERNAL_LIBRARIES}
	${FFTW_LIBRARIES}
	${HDF5_CXX_LIBRARIES}
	${OpenMP_CXX_LIBRARIES}
	${MPI_CXX_LIBRARIES}
	${Scalapack_LIBRARIES}
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
set(EXTERNAL_LINK_FLAGS ${EXTERNAL_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS})

set(LOCAL_INCLUDE_DIRS utils SDDK SDDK/GPU ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test1 test_fft_correctness_1.cpp ${SOURCE_FILES})
target_include_directories(test1 PRIVATE  ${LOCAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS})
target_compile_options(test1 PRIVATE ${EXTERNAL_COMPILE_FLAGS})
target_link_libraries(test1 PRIVATE ${EXTERNAL_LIBRARIES} ${EXTERNAL_LINK_FLAGS})
target_compile_definitions(test1 PRIVATE ${EXTERNAL_DEFINITIONS} ${LOCAL_DEFINITIONS})
