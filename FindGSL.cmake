# Finds the GSL library and includes
# Copyright © 2005-2009 Swiss Tropical Institute and Liverpool School Of Tropical Medicine
# Licence: GNU General Public Licence version 2 or later (see COPYING)
#
# Defines:
# GSL_LIBRARIES
# GSL_INCLUDE_DIRS

# list common paths here:
# The gsl-111 dir is because people have GSL 1.8 installed in C:/Program Files/GnuWin32/include
# which isn't supported and gsl-1.11 extracted to in gsl-111
FIND_PATH(GSL_INCLUDE_DIR gsl/gsl_version.h
 PATHS
  ${CMAKE_SOURCE_DIR}/../gsl/include
  ${CMAKE_SOURCE_DIR}/../gsl
  "C:/Program Files/GnuWin32/gsl-111/Binaries/gsl/include"
  "C:/Program Files/GnuWin32/include"
  "C:/Program Files/GnuWin32/gsl-111/gsl/win32/include"
)

if (NOT GSL_INCLUDE_DIR)
  message (FATAL_ERROR "Unable to find gsl/gsl_version.h")
endif (NOT GSL_INCLUDE_DIR)

FIND_PATH(GSL_INCLUDE_DIR2 gsl_sys.h
 PATHS
  ${GSL_INCLUDE_DIR}
  "C:/Program Files/GnuWin32/gsl-111/Binaries/gsl/include/gsl"
  "C:/Program Files/GnuWin32/include/gsl"
  "C:/Program Files/GnuWin32/gsl-111/gsl/win32/include/gsl"
  DOC "Extra include path (usually not needed and so often NOTFOUND)"
)
if (GSL_INCLUDE_DIR2)
  MARK_AS_ADVANCED(GSL_INCLUDE_DIR2)
  if (NOT ${GSL_INCLUDE_DIR} STREQUAL ${GSL_INCLUDE_DIR2})
    set (GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR} ${GSL_INCLUDE_DIR2} CACHE PATH "GSL include dirs")
  endif (NOT ${GSL_INCLUDE_DIR} STREQUAL ${GSL_INCLUDE_DIR2})
endif (GSL_INCLUDE_DIR2)
set (GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR} CACHE PATH "GSL include dirs")

# gsl puts libraries in various places, with various suffixes. Copy the ones you want to use to ../gsl/lib and remove suffixes.
find_library (GSL_LIB gsl
  PATHS ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/../gsl/lib
  "C:/Program Files/GnuWin32/gsl-111/gsl/win32/lib"
  "C:/Program Files/GnuWin32/gsl-111/Binaries/gsl/lib"
  "C:/Program Files/GnuWin32/lib"
)
find_library (GSL_CBLAS_LIB NAMES gsl_cblas gslcblas cblas
  PATHS ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/../gsl/lib
  "C:/Program Files/GnuWin32/gsl-111/gsl/win32/lib"
  "C:/Program Files/GnuWin32/gsl-111/Binaries/gsl/lib"
  "C:/Program Files/GnuWin32/lib"
)
if (NOT GSL_LIB OR NOT GSL_CBLAS_LIB)
  message (FATAL_ERROR "Unable to find gsl library")
endif (NOT GSL_LIB OR NOT GSL_CBLAS_LIB)

SET (GSL_LIBRARIES ${GSL_LIB} ${GSL_CBLAS_LIB})

MARK_AS_ADVANCED(
  GSL_INCLUDE_DIR
  GSL_INCLUDE_DIRS
  GSL_LIB
  GSL_CBLAS_LIB
)
