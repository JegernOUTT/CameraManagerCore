#
# This module detects if gsoap is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
#
# GSOAP_IMPORT_DIR      = full path to the gsoap import directory
# GSOAP_LIBRARIES       = full path to the gsoap libraries
# GSOAP_SSL_LIBRARIES   = full path to the gsoap ssl libraries
# GSOAP_INCLUDE_DIR     = include dir to be used when using the gsoap library
# GSOAP_FOUND           = set to true if gsoap was found successfully
#
# GSOAP_ROOT
#   setting this enables search for gsoap libraries / headers in this location

# -----------------------------------------------------
# GSOAP Import Directories
# -----------------------------------------------------
find_path(GSOAP_IMPORT_DIR
        NAMES wsa.h
        PATHS ${GSOAP_ROOT}/import ${GSOAP_ROOT}/share/gsoap/import
        )

# -----------------------------------------------------
# GSOAP Libraries
# -----------------------------------------------------
find_library(GSOAP_CXX_LIBRARIES
        NAMES gsoap++
        HINTS ${GSOAP_ROOT}/lib ${GSOAP_ROOT}/lib64
        ${GSOAP_ROOT}/lib32
        DOC "The main gsoap library"
        )
find_library(GSOAP_SSL_CXX_LIBRARIES
        NAMES gsoapssl++
        HINTS ${GSOAP_ROOT}/lib ${GSOAP_ROOT}/lib64
        ${GSOAP_ROOT}/lib32
        DOC "The ssl gsoap library"
        )

find_library(GSOAP_C_LIBRARIES
        NAMES gsoap
        HINTS ${GSOAP_ROOT}/lib ${GSOAP_ROOT}/lib64
        ${GSOAP_ROOT}/lib32
        DOC "The main gsoap library"
        )
find_library(GSOAP_SSL_C_LIBRARIES
        NAMES gsoapssl
        HINTS ${GSOAP_ROOT}/lib ${GSOAP_ROOT}/lib64
        ${GSOAP_ROOT}/lib32
        DOC "The ssl gsoap library"
        )

# -----------------------------------------------------
# GSOAP Include Directories
# -----------------------------------------------------
find_path(GSOAP_INCLUDE_DIR
        NAMES stdsoap2.h
        HINTS ${GSOAP_ROOT} ${GSOAP_ROOT}/include ${GSOAP_ROOT}/include/*
        DOC "The gsoap include directory"
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gsoap DEFAULT_MSG GSOAP_CXX_LIBRARIES GSOAP_C_LIBRARIES
        GSOAP_INCLUDE_DIR)
mark_as_advanced(GSOAP_INCLUDE_DIR GSOAP_LIBRARIES)
