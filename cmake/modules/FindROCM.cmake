# - Find the ROCM library
#
# Usage:
#   find_package(ROCM [REQUIRED] [QUIET] COMPONENTS [components ...] )
#
# Compnents available:
#  - hipblas
#  - hipsparse
#  - rocfft
#  - rocblas
#  - rocsparse
#
# It sets the following variables:
#   ROCM_FOUND ------------------- true if ROCM is found on the system
#   ROCM_LIBRARIES --------------- full path to ROCM
#   ROCM_INCLUDE_DIRS ------------ ROCM include directories
#   ROCM_DEFINITIONS ------------- ROCM definitions
#   ROCM_HCC_EXECUTABLE ---------- ROCM HCC compiler
#   ROCM_HCC-CONFIG_EXECUTABLE --- ROCM HCC config
#   ROCM_HIPCC_EXECUTABLE -------- HIPCC compiler
#   ROCM_HIPCONFIG_EXECUTABLE ---- hip config
#   ROCM_HIPIFY-PERL_EXECUTABLE -- hipify
#   ROCM_HIP_PLATFORM ------------ Platform identifier: hcc or nvcc
#
# The following variables can be set to specify a search location
#   ROCM_ROOT ------------ if set, the libraries are exclusively searched under this path
#   <COMPONENT>_ROOT ------ if set, search for component specific libraries at given path. Takes precedence over ROCM_ROOT

#If environment variable ROCM_ROOT is specified
if(NOT ROCM_ROOT AND ENV{ROCM_ROOT})
    file(TO_CMAKE_PATH "$ENV{ROCM_ROOT}" ROCM_ROOT)
    set(ROCM_ROOT "${ROCM_ROOT}" CACHE PATH "Root directory for ROCM installation.")
endif()

set(ROCM_FOUND FALSE)
unset(ROCM_LIBRARIES)
unset(ROCM_INCLUDE_DIRS)
unset(ROCM_DEFINITIONS)
unset(ROCM_HCC_EXECUTABLE)
unset(ROCM_HCC-CONFIG_EXECUTABLE)
unset(ROCM_HIPCC_EXECUTABLE)
unset(ROCM_HIPCONFIG_EXECUTABLE)
unset(ROCM_HIPFIY-PERL-EXECUTABLE)
unset(ROCM_HIP_PLATFORM)

include(FindPackageHandleStandardArgs)


# Finds libraries and include path for rocm modules
# IN:
#   - module_name: name of a module (e.g. hcc)
#   - following arguments: name of libraries required
# OUT:
#   - ROCM_LIBRARIES: Appends to list of libraries
#   - ROCM_INCLUDE_DIRS: Appends to include dirs
function(find_rcm_module module_name)
    # convert module name to upper case for consistent variable naming
    string(TOUPPER ${module_name} MODULE_NAME_UPPER)
    set(ROOT_DIR ${${MODULE_NAME_UPPER}_ROOT})

    set(LIBRARIES_${MODULE_NAME_UPPER})

    # get abosolute path to avoid issues with tilde
    if(ROCM_ROOT)
        get_filename_component(ROCM_ROOT ${ROCM_ROOT} ABSOLUTE)
    endif()
    if(ROOT_DIR)
        get_filename_component(ROOT_DIR ${ROOT_DIR} ABSOLUTE)
    endif()

    # remove module name from input arguments
    set(LIBRARY_NAMES ${ARGV})
    list(REMOVE_AT LIBRARY_NAMES 0)

    if(${ROCM_FIND_REQUIRED})
	set(ROCM_${MODULE_NAME_UPPER}_FIND_REQUIRED TRUE)
    else()
	set(ROCM_${MODULE_NAME_UPPER}_FIND_REQUIRED FALSE)
    endif()
    if(${ROCM_FIND_QUIETLY})
	set(ROCM_${MODULE_NAME_UPPER}_FIND_QUIETLY TRUE)
    else()
	set(ROCM_${MODULE_NAME_UPPER}_FIND_QUIETLY FALSE)
    endif()

    if(ROOT_DIR)
        # Root directory set for individual module

        # find libraries
        foreach(library_name IN LISTS LIBRARY_NAMES)
            set(LIBRARIES_${library_name})
            find_library(
                LIBRARIES_${library_name}
                NAMES ${library_name}
                PATHS ${ROOT_DIR}
                PATH_SUFFIXES "lib" "lib64"
                NO_DEFAULT_PATH
            )
	find_package_handle_standard_args(ROCM_${MODULE_NAME_UPPER} FAIL_MESSAGE
                "For ROCM module ${module_name}, library ${library_name} could not be found. Please specify ROCM_ROOT or ${MODULE_NAME_UPPER}_ROOT." 
                REQUIRED_VARS LIBRARIES_${library_name})
                set(LIBRARIES_${MODULE_NAME_UPPER} ${LIBRARIES_${MODULE_NAME_UPPER}} ${LIBRARIES_${library_name}})
        endforeach()

        # find include directory
        find_path(
            INCLUDE_DIRS_${MODULE_NAME_UPPER}
            NAMES include
            PATHS ${ROOT_DIR}
            NO_DEFAULT_PATH
        )
        # set include directory for module if found
        if(INCLUDE_DIRS_${MODULE_NAME_UPPER})
            set(INCLUDE_DIRS_${MODULE_NAME_UPPER} ${INCLUDE_DIRS_${MODULE_NAME_UPPER}}/include)
        endif()
    elseif(ROCM_ROOT)
        #Root directory set for rocm

        # find libraries
        foreach(library_name IN LISTS LIBRARY_NAMES)
            set(LIBRARIES_${library_name})
            find_library(
                LIBRARIES_${library_name}
                NAMES ${library_name}
                PATHS ${ROCM_ROOT}
                PATH_SUFFIXES "lib" "lib64" "${module_name}/lib" "${module_name}/lib64"
                NO_DEFAULT_PATH
            )
	find_package_handle_standard_args(ROCM_${MODULE_NAME_UPPER} FAIL_MESSAGE
                "For ROCM module ${module_name}, library ${library_name} could not be found. Please specify ROCM_ROOT or ${MODULE_NAME_UPPER}_ROOT." 
                REQUIRED_VARS LIBRARIES_${library_name})
                set(LIBRARIES_${MODULE_NAME_UPPER} ${LIBRARIES_${MODULE_NAME_UPPER}} ${LIBRARIES_${library_name}})
        endforeach()
        find_path(
            INCLUDE_DIRS_${MODULE_NAME_UPPER}
            NAMES ${module_name}/include
            PATHS ${ROCM_ROOT}
            NO_DEFAULT_PATH
        )
        # set include directory for module if found
        if(INCLUDE_DIRS_${MODULE_NAME_UPPER})
            set(INCLUDE_DIRS_${MODULE_NAME_UPPER} ${INCLUDE_DIRS_${MODULE_NAME_UPPER}}/${module_name}/include)
        endif()
    else()
        # No root directory set

        foreach(library_name IN LISTS LIBRARY_NAMES)
            set(LIBRARIES_${library_name})
            find_library(
                LIBRARIES_${MODULE_NAME_UPPER}
                NAMES ${LIBRARY_NAMES}
                PATHS "/opt/rocm/"
                PATH_SUFFIXES "lib" "lib64" "${module_name}/lib" "${module_name}/lib64"
            )
	find_package_handle_standard_args(ROCM_${MODULE_NAME_UPPER} FAIL_MESSAGE
                "For ROCM module ${module_name}, library ${library_name} could not be found. Please specify ROCM_ROOT or ${MODULE_NAME_UPPER}_ROOT." 
                REQUIRED_VARS LIBRARIES_${library_name})
                set(LIBRARIES_${MODULE_NAME_UPPER} ${LIBRARIES_${MODULE_NAME_UPPER}} ${LIBRARIES_${library_name}})
        endforeach()

        # find include directory
        find_path(
            INCLUDE_DIRS_${MODULE_NAME_UPPER}
            NAMES ${module_name}/include
            PATHS "/opt/rocm/"
        )
        # set include directory for module if found
        if(INCLUDE_DIRS_${MODULE_NAME_UPPER})
            set(INCLUDE_DIRS_${MODULE_NAME_UPPER} ${INCLUDE_DIRS_${MODULE_NAME_UPPER}}/${module_name}/include)
        endif()
    endif()


    # check if all required parts found
    find_package_handle_standard_args(ROCM_${MODULE_NAME_UPPER} FAIL_MESSAGE
        "ROCM module ${module_name} could not be found. Please specify ROCM_ROOT or ${MODULE_NAME_UPPER}_ROOT." 
        REQUIRED_VARS INCLUDE_DIRS_${MODULE_NAME_UPPER})

    # set global variables
    if(LIBRARIES_${MODULE_NAME_UPPER})
        set(ROCM_LIBRARIES ${ROCM_LIBRARIES} ${LIBRARIES_${MODULE_NAME_UPPER}} PARENT_SCOPE)
    endif()
    if(INCLUDE_DIRS_${MODULE_NAME_UPPER})
        set(ROCM_INCLUDE_DIRS ${ROCM_INCLUDE_DIRS} ${INCLUDE_DIRS_${MODULE_NAME_UPPER}} PARENT_SCOPE)
    endif()

endfunction()

function(find_rocm_executable module_name executable_name)
    string(TOUPPER ${module_name} MODULE_NAME_UPPER)
    string(TOUPPER ${executable_name} EXECUTABLE_NAME_UPPER)
    unset(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE PARENT_SCOPE)
    if(${MODULE_NAME_UPPER}_ROOT)
            find_path(
                ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE
                NAMES bin/${executable_name}
                PATHS ${${MODULE_NAME_UPPER}_ROOT}
                NO_DEFAULT_PATH
            )
	if(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE)
	    set(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE ${ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE}/bin/${executable_name} PARENT_SCOPE)
	endif()
    elseif(ROCM_ROOT)
            find_path(
                ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE
                NAMES ${module_name}/bin/${executable_name}
                PATHS ${ROCM_ROOT}
                NO_DEFAULT_PATH
            )
	if(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE)
	    set(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE ${ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE}/${module_name}/bin/${executable_name} PARENT_SCOPE)
	endif()
    else()
            find_path(
                ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE
                NAMES ${module_name}/bin/${executable_name}
                PATHS "/opt/rocm"
            )
	if(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE)
	    set(ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE ${ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE}/${module_name}/bin/${executable_name} PARENT_SCOPE)
	endif()
    endif()
    if(${ROCM_FIND_REQUIRED})
	set(ROCM_${MODULE_NAME_UPPER}_${EXECUTABLE_NAME_UPPER}_FIND_REQUIRED TRUE)
    else()
	set(ROCM_${MODULE_NAME_UPPER}_${EXECUTABLE_NAME_UPPER}_FIND_REQUIRED FALSE)
    endif()
    if(${ROCM_FIND_QUIETLY})
	set(ROCM_${MODULE_NAME_UPPER}_${EXECUTABLE_NAME_UPPER}_FIND_QUIETLY TRUE)
    else()
	set(ROCM_${MODULE_NAME_UPPER}_${EXECUTABLE_NAME_UPPER}_FIND_QUIETLY FALSE)
    endif()
    find_package_handle_standard_args(ROCM FAIL_MESSAGE
	"ROCM_${MODULE_NAME_UPPER}_${EXECUTABLE_NAME_UPPER} ${executable_name} executable could not be found. Please specify ROCM_ROOT or ${MODULE_NAME_UPPER}_ROOT."
        REQUIRED_VARS ROCM_${EXECUTABLE_NAME_UPPER}_EXECUTABLE)
endfunction()




# find compilers
find_rocm_executable(hcc hcc)
find_rocm_executable(hip hipcc)

if(ROCM_HIPCC_EXECUTABLE AND ROCM_HCC_EXECUTABLE)
    set(ROCM_FOUND TRUE)
else()
    set(ROCM_FOUND FALSE)
    return()
endif()


# find other executables and libraries
find_rocm_executable(hcc hcc-config)
find_rocm_executable(hip hipconfig)
find_rocm_executable(hip hipify-perl)
find_rcm_module(hcc LTO OptRemarks mcwamp mcwamp_cpu mcwamp_hsa hc_am)
find_rcm_module(hip hip_hcc)


# parse hip config
execute_process(COMMAND ${ROCM_HIPCONFIG_EXECUTABLE} -P OUTPUT_VARIABLE ROCM_HIP_PLATFORM RESULT_VARIABLE RESULT_VALUE)
if(NOT ${RESULT_VALUE} EQUAL 0)
    message(FATAL_ERROR "Error parsing platform identifier from hipconfig! Code: ${RESULT_VALUE}")
endif()
if(NOT ROCM_HIP_PLATFORM)
    message(FATAL_ERROR "Empty platform identifier from hipconfig!")
endif()

# set definitions
if(${ROCM_HIP_PLATFORM} STREQUAL hcc)
    set(ROCM_DEFINITIONS -D__HIP_PLATFORM_HCC__)
elseif(${ROCM_HIP_PLATFORM} STREQUAL nvcc)
    set(ROCM_DEFINITIONS -D__HIP_PLATFORM_NVCC__)
else()
    message(FATAL_ERROR "Could not parse platform identifier from hipconfig! Value: ${ROCM_HIP_PLATFORM}")
endif()

# find libraries for each specified components
foreach(module_name IN LISTS ROCM_FIND_COMPONENTS)
    # set required libaries for each module
    if(${module_name} STREQUAL hipblas)
        find_rcm_module(hipblas hipblas)
    elseif(${module_name} STREQUAL hipsparse)
        find_rcm_module(hipsparse hipsparse)
    elseif(${module_name} STREQUAL rocblas)
        find_rcm_module(rocblas rocblas)
    elseif(${module_name} STREQUAL rocsparse)
        find_rcm_module(rocsparse rocsparse)
    elseif(${module_name} STREQUAL rocfft)
        find_rcm_module(rocfft rocfft rocfft-device)
    else()
        message(FATAL_ERROR "Unrecognized component \"${module_name}\" in FindROCM module!")
    endif()
endforeach()


# Generates library compiled with hipcc
# Usage:
#   rocm_hip_add_library(<name> <sources> [STATIC | SHARED] [FLAGS] <flags> [OUTPUT_DIR] <dir> [INCLUDE_DIRS] <dirs ...>)
macro(rocm_hip_add_library)
    cmake_parse_arguments(
        HIP_LIB
        "SHARED;STATIC"
        "OUTPUT_DIR"
        "FLAGS;INCLUDE_DIRS"
        ${ARGN}
    )
    # allow either STATIC or SHARED
    if(HIP_LIB_SHARED AND HIP_LIB_STATIC)
        message(FATAL_ERROR "rocm_hip_add_library: library cannot by both static and shared!")
    endif()

    # default to SHARED
    if(NOT (HIP_LIB_SHARED OR HIP_LIB_STATIC))
        set(HIP_LIB_SHARED TRUE)
    endif()

    # default to top level build directory
    if(NOT HIP_LIB_OUTPUT_DIR)
        set(HIP_LIB_OUTPUT_DIR ${PROJECT_BINARY_DIR})
    endif()

    # parse positional arguments
    list(LENGTH HIP_LIB_UNPARSED_ARGUMENTS NARGS)
    if(${NARGS} LESS 2)
        message(FATAL_ERROR "rocm_hip_add_library: Not enough arguments!")
    endif()
    list(GET HIP_LIB_UNPARSED_ARGUMENTS 0 HIP_LIB_NAME)
    list(REMOVE_AT HIP_LIB_UNPARSED_ARGUMENTS 0)
    set(HIP_LIB_SOURCES ${HIP_LIB_UNPARSED_ARGUMENTS})

	# generate include flags
    unset(ROCM_INTERNAL_FULL_PATH_INCLUDE_FLAGS)
    foreach(dir IN LISTS HIP_LIB_INCLUDE_DIRS)
		if(NOT IS_ABSOLUTE ${dir})
			get_filename_component(dir ${dir} ABSOLUTE)
		endif()
		set(ROCM_INTERNAL_FULL_PATH_INCLUDE_FLAGS ${ROCM_INTERNAL_FULL_PATH_INCLUDE_FLAGS} -I${dir})
	endforeach()

	# generate full path to source files
	unset(ROCM_INTERNAL_SOURCES)
    foreach(source IN LISTS HIP_LIB_SOURCES)
		if(NOT IS_ABSOLUTE ${source})
			get_filename_component(source ${source} ABSOLUTE)
		endif()
		set(ROCM_INTERNAL_SOURCES ${ROCM_INTERNAL_SOURCES} ${source})
	endforeach()

	# generate flags to use
	set(ROCM_INTERNAL_FLAGS ${CMAKE_CXX_FLAGS})
	set(ROCM_INTERNAL_BUILD_TYPES DEBUG RELEASE RELWITHDEBINFO)
	if(CMAKE_BUILD_TYPE)
		foreach(type IN LISTS ROCM_INTERNAL_BUILD_TYPES)
			string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
			if(${type} MATCHES ${BUILD_TYPE_UPPER})
				set(ROCM_INTERNAL_FLAGS ${ROCM_INTERNAL_FLAGS} ${CMAKE_CXX_FLAGS_${type}})
			endif()
		endforeach()
	endif()

	if(NOT ROCM_HIPCC_EXECUTABLE)
		message(FATAL_ERROR "HIPCC executable not found!")
	endif()

    if(HIP_LIB_SHARED)
        set(ROCM_INTERNAL_FLAGS ${ROCM_INTERNAL_FLAGS} --shared -fPIC -o ${HIP_LIB_OUTPUT_DIR}/lib${HIP_LIB_NAME}.so)
    endif()
    if(HIP_LIB_STATIC)
        set(ROCM_INTERNAL_FLAGS ${ROCM_INTERNAL_FLAGS} -c)
    endif()

	# compile GPU kernels with hipcc compiler
    add_custom_target(HIP_TARGET_${HIP_LIB_NAME} COMMAND ${ROCM_HIPCC_EXECUTABLE} ${ROCM_INTERNAL_SOURCES} ${ROCM_INTERNAL_FLAGS} ${ROCM_INTERNAL_FULL_PATH_INCLUDE_FLAGS}
        WORKING_DIRECTORY ${HIP_LIB_OUTPUT_DIR} SOURCES ${ROCM_INTERNAL_SOURCES})

    # shared library
    if(HIP_LIB_SHARED)
        add_library(${HIP_LIB_NAME} SHARED IMPORTED)
        set_target_properties(${HIP_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${HIP_LIB_OUTPUT_DIR}/lib${HIP_LIB_NAME}.so)
    endif()

    # static library
    if(HIP_LIB_STATIC)
        # generate object file names
        unset(ROCM_INTERNAL_OBJS)
        foreach(file IN LISTS ROCM_INTERNAL_SOURCES)
            string(REGEX REPLACE "\\.[^.]*$" "" file ${file})
            get_filename_component(file ${file} NAME)
            set(ROCM_INTERNAL_OBJS ${ROCM_INTERNAL_OBJS} ${HIP_LIB_OUTPUT_DIR}/${file}.o)
        endforeach()

        # create library from object files
        add_library(${HIP_LIB_NAME} ${ROCM_INTERNAL_OBJS})
        set_target_properties(${HIP_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
        set_source_files_properties(
            ${ROCM_INTERNAL_OBJS}
            PROPERTIES
            EXTERNAL_OBJECT true
            GENERATED true
            )
    endif()

    # create library / object files before linking
    add_dependencies(${HIP_LIB_NAME} HIP_TARGET_${HIP_LIB_NAME})
endmacro()

