# - Find the ROCM library
#
# Usage:
#   find_package(ROCM [REQUIRED] [QUIET] COMPONENTS [components ...] )
#
# Compnents available:
#  - hip
#  - hipblas
#  - hipsparse
#  - rocblas
#  - rocsparse
#
# It sets the following variables:
#   ROCM_FOUND ----------- true if ROCM is found on the system
#   ROCM_LIBRARIES ------- full path to ROCM
#   ROCM_INCLUDE_DIRS ---- ROCM include directories
#   ROCM_HCC_COMPILER ---- ROCM HCC compiler
#   [ROCM_HIP_COMPILER]- ROCM HIPCC compiler (only defined of hip component requested)
#
# The following variables can be set to specify a search location
#   ROCM_ROOT ------------ if set, the libraries are exclusively searched under this path
#   {compnents}_ROOT ------ if set, search for component specific libraries at given path. Takes precedence over ROCM_ROOT

#If environment variable ROCM_ROOT is specified
if(NOT ROCM_ROOT AND ENV{ROCM_ROOT})
    file(TO_CMAKE_PATH "$ENV{ROCM_ROOT}" ROCM_ROOT)
    set(ROCM_ROOT "${ROCM_ROOT}" CACHE PATH "Root directory for ROCM installation.")
endif()

set(ROCM_FOUND FALSE)
set(ROCM_LIBRARIES)
set(ROCM_INCLUDE_DIRS)
set(ROCM_${MODULE_NAME_UPPER}_COMPILER)
set(ROCM_HIPCC_COMPILER)

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

    # handler for error messages

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
            find_package_handle_standard_args(ROCM FAIL_MESSAGE
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
            find_package_handle_standard_args(ROCM FAIL_MESSAGE
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
            find_package_handle_standard_args(ROCM FAIL_MESSAGE
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
    find_package_handle_standard_args(ROCM FAIL_MESSAGE
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

function(find_rocm_compiler module_name exectuable_name)
    string(TOUPPER ${module_name} MODULE_NAME_UPPER)
    set(ROCM_${MODULE_NAME_UPPER}_COMPILER PARENT_SCOPE)
    if(${MODULE_NAME_UPPER}_ROOT)
            find_path(
                ROCM_${MODULE_NAME_UPPER}_COMPILER
                NAMES bin/${exectuable_name}
                PATHS ${${MODULE_NAME_UPPER}_ROOT}
                NO_DEFAULT_PATH
            )
        set(ROCM_${MODULE_NAME_UPPER}_COMPILER ${ROCM_${MODULE_NAME_UPPER}_COMPILER}/bin/${exectuable_name} PARENT_SCOPE)
    elseif(ROCM_ROOT)
            find_path(
                ROCM_${MODULE_NAME_UPPER}_COMPILER
                NAMES ${module_name}/bin/${exectuable_name}
                PATHS ${ROCM_ROOT}
                NO_DEFAULT_PATH
            )
        set(ROCM_${MODULE_NAME_UPPER}_COMPILER ${ROCM_${MODULE_NAME_UPPER}_COMPILER}/${module_name}/bin/${exectuable_name} PARENT_SCOPE)
    else()
            find_path(
                ROCM_${MODULE_NAME_UPPER}_COMPILER
                NAMES ${module_name}/bin/${exectuable_name}
                PATHS "/opt/rocm"
            )
        set(ROCM_${MODULE_NAME_UPPER}_COMPILER ${ROCM_${MODULE_NAME_UPPER}_COMPILER}/${module_name}/bin/${exectuable_name} PARENT_SCOPE)
    endif()
    find_package_handle_standard_args(ROCM FAIL_MESSAGE
        "ROCM ${exectuable_name} executable could not be found. Please specify ROCM_ROOT or HCC_ROOT."
        REQUIRED_VARS ROCM_${MODULE_NAME_UPPER}_COMPILER)
endfunction()




find_rocm_compiler(hcc hcc)
message(STATUS "ROCM_HCC_COMPILER: ${ROCM_HCC_COMPILER}")
find_rcm_module(hcc LTO OptRemarks mcwamp mcwamp_cpu mcwamp_hsa hc_am)
if(ROCM_HCC_COMPILER)
    set(ROCM_FOUND TRUE)
else()
    set(ROCM_FOUND FALSE)
endif()



# find libraries for each specified components
foreach(module_name IN LISTS ROCM_FIND_COMPONENTS)
    # set required libaries for each module
    if(${module_name} STREQUAL hip)
        find_rcm_module(hip hip_hcc)
        find_rocm_compiler(hip hipcc)
    elseif(${module_name} STREQUAL hipblas)
        find_rcm_module(hipblas hipblas)
    elseif(${module_name} STREQUAL hipblas)
        find_rcm_module(hipblas hipblas)
    elseif(${module_name} STREQUAL hipsparse)
        find_rcm_module(hipblas hipsparse)
    elseif(${module_name} STREQUAL hsa)
        find_rcm_module(hsa hsa-ext-image64 hsa-runtime-tools64 hsa-runtime64)
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

mark_as_advanced(ROCM_LIBRARIES)
mark_as_advanced(ROCM_INCLUDE_DIRS)
mark_as_advanced(ROCM_FOUND)
