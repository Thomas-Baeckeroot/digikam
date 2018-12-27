# - Try to find the Exiv2 library
#
#  EXIV2_MIN_VERSION - You can set this variable to the minimum version you need
#                      before doing FIND_PACKAGE(Exiv2). The default is 0.26.
#
# Once done this will define
#
#  EXIV2_FOUND       - system has libexiv2
#  EXIV2_VERSION     - the version of libexiv2
#  EXIV2_INCLUDE_DIR - the libexiv2 include directory
#  EXIV2_LIBRARIES   - Link these to use libexiv2
#  EXIV2_DEFINITIONS - Compiler switches required for using libexiv2
#
# The minimum required version of Exiv2 can be specified using the
# standard syntax, e.g. find_package(Exiv2 0.26)
#
# For compatibility, also the variable EXIV2_MIN_VERSION can be set to the minimum version
# you need before doing FIND_PACKAGE(Exiv2). The default is 0.26.
#
# Copyright (c) 2010,      Alexander Neundorf, <neundorf at kde dot org>
# Copyright (c) 2008-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Support EXIV2_MIN_VERSION for compatibility:

if(NOT Exiv2_FIND_VERSION)

    set(Exiv2_FIND_VERSION "${EXIV2_MIN_VERSION}")

endif()

# The minimum version of exiv2 we require
if(NOT Exiv2_FIND_VERSION)

    set(Exiv2_FIND_VERSION "0.26")

endif()

if(NOT WIN32)

   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   find_package(PkgConfig)
   pkg_check_modules(PC_EXIV2 QUIET exiv2)
   set(EXIV2_DEFINITIONS ${PC_EXIV2_CFLAGS_OTHER})

endif()

find_path(EXIV2_INCLUDE_DIR NAMES exiv2/exif.hpp
          HINTS
          ${PC_EXIV2_INCLUDEDIR}
          ${PC_EXIV2_INCLUDE_DIRS}
        )

find_library(EXIV2_LIBRARY NAMES exiv2 libexiv2 exiv2lib
             HINTS
             ${PC_EXIV2_LIBDIR}
             ${PC_EXIV2_LIBRARY_DIRS}
            )

if (EXIV2_INCLUDE_DIR  AND NOT  EXIV2_VERSION)

    # For Exiv2 <= 0.26, get the version number from exiv2/version.hpp and store it in the cache:

    file(READ ${EXIV2_INCLUDE_DIR}/exiv2/version.hpp EXIV2_VERSION_CONTENT)
    string(REGEX MATCH "#define EXIV2_MAJOR_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
    set(EXIV2_VERSION_MAJOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define EXIV2_MINOR_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
    set(EXIV2_VERSION_MINOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define EXIV2_PATCH_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
    set(EXIV2_VERSION_PATCH "${CMAKE_MATCH_1}")

    if(NOT "${EXIV2_VERSION_MAJOR}" STREQUAL "" AND
       NOT "${EXIV2_VERSION_MINOR}" STREQUAL "" AND
       NOT "${EXIV2_VERSION_PATCH}" STREQUAL "")

        set(EXIV2_VERSION "${EXIV2_VERSION_MAJOR}.${EXIV2_VERSION_MINOR}.${EXIV2_VERSION_PATCH}"
            CACHE STRING "Version number of Exiv2" FORCE)
    else()

        # For Exiv2 >= 0.27, get the version number from exiv2/exv_conf.h and store it in the cache:

        file(READ ${EXIV2_INCLUDE_DIR}/exiv2/exv_conf.h EXIV2_VERSION_CONTENT)
        string(REGEX MATCH "#define EXIV2_MAJOR_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
        set(EXIV2_VERSION_MAJOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define EXIV2_MINOR_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
        set(EXIV2_VERSION_MINOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define EXIV2_PATCH_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
        set(EXIV2_VERSION_PATCH "${CMAKE_MATCH_1}")

        set(EXIV2_VERSION "${EXIV2_VERSION_MAJOR}.${EXIV2_VERSION_MINOR}.${EXIV2_VERSION_PATCH}"
            CACHE STRING "Version number of Exiv2" FORCE)

    endif()

endif()

set(EXIV2_LIBRARIES "${EXIV2_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Exiv2 REQUIRED_VARS EXIV2_LIBRARY EXIV2_INCLUDE_DIR
                                        VERSION_VAR   EXIV2_VERSION)

MESSAGE(STATUS "EXIV2_FOUND       = ${EXIV2_FOUND}")
MESSAGE(STATUS "EXIV2_VERSION     = ${EXIV2_VERSION}")
MESSAGE(STATUS "EXIV2_INCLUDE_DIR = ${EXIV2_INCLUDE_DIR}")
MESSAGE(STATUS "EXIV2_LIBRARY     = ${EXIV2_LIBRARY}")
MESSAGE(STATUS "EXIV2_DEFINITIONS = ${EXIV2_DEFINITIONS}")

mark_as_advanced(EXIV2_INCLUDE_DIR EXIV2_LIBRARY)
