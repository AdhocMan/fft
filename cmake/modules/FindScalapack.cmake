# - Find the Scalapack library
#
# Usage:
#   find_package(Scalapack [REQUIRED] [QUIET] )
#
# It sets the following variables:
#   Scalapack_FOUND ----------- true if scalapack is found on the system
#   Scalapack_LIBRARIES ------- full path to scalapack library
#   Scalapack_INCLUDE_DIRS ---- scalapack include directory
#
# The following variables can be set to specify a search location
#   Scalapack_ROOT ------------ if set, the libraries are exclusively searched under this path


#If environment variable Scalapack_ROOT is specified
if(NOT Scalapack_ROOT AND ENV{Scalapack_ROOT})
    file(TO_CMAKE_PATH "$ENV{Scalapack_ROOT}" Scalapack_ROOT)
    set(Scalapack_ROOT "${Scalapack_ROOT}" CACHE PATH "Prefix for scalapack installation.")
endif()

# Check if we can use PkgConfig
find_package(PkgConfig QUIET)
#Determine from PKG
if(PKG_CONFIG_FOUND AND NOT Scalapack_ROOT)
    pkg_check_modules(PKG_Scalapack QUIET "scalapack")
endif()


if(Scalapack_ROOT)
    #find libs
    find_library(
            Scalapack_LIBRARIES
            NAMES "scalapack"
            PATHS ${Scalapack_ROOT}
            PATH_SUFFIXES "lib" "lib64"
            NO_DEFAULT_PATH
    )
else()
    find_library(
            Scalapack_LIBRARIES
            NAMES "scalapack"
            PATHS ${PKG_Scalapack_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
            PATH_SUFFIXES "lib" "lib64"
    )
endif(Scalapack_ROOT)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Scalapack "scalapack library could not be found. Please specify Scalapack_ROOT."
        Scalapack_LIBRARIES)

set(Scalapack_INCLUDE_DIRS)
mark_as_advanced(Scalapack_INCLUDE_DIRS Scalapack_LIBRARIES)
