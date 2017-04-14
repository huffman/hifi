#
#  FindNVTT.cmake
#
#  Try to find NVIDIA texture tools library and include path.
#  Once done this will define
#
#  NVTT_FOUND
#  NVTT_INCLUDE_DIRS
#  NVTT_LIBRARIES
#  NVTT_DLL_PATH
#
#  Created on 4/14/2017 by Stephen Birarda
#  Copyright 2017 High Fidelity, Inc.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

include("${MACRO_DIR}/HifiLibrarySearchHints.cmake")
hifi_library_search_hints("nvtt")

find_path(NVTT_INCLUDE_DIRS nvtt/nvtt.h PATH_SUFFIXES include HINTS ${NVTT_SEARCH_DIRS})

include(FindPackageHandleStandardArgs)

if (WIN32)
  find_library(NVTT_LIBRARY_RELEASE nvtt PATH_SUFFIXES "Release.x64/lib" HINTS ${NVTT_SEARCH_DIRS})
  find_library(NVTT_LIBRARY_DEBUG nvtt PATH_SUFFIXES "Debug.x64/lib" HINTS ${NVTT_SEARCH_DIRS})

  find_path(NVTT_RELEASE_DLL_PATH nvtt.dll PATH_SUFFIXES "Release.x64/bin" HINTS ${NVTT_SEARCH_DIRS})
  find_path(NVTT_DEBUG_DLL_PATH nvtt.dll PATH_SUFFIXES "Debug.x64/lib" HINTS ${NVTT_SEARCH_DIRS})

  include(SelectLibraryConfigurations)
  select_library_configurations(NVTT)

  find_package_handle_standard_args(NVTT DEFAULT_MSG NVTT_INCLUDE_DIRS NVTT_LIBRARIES NVTT_RELEASE_DLL_PATH NVTT_DEBUG_DLL_PATH)

  set(NVTT_DLL_PATH "$<$<NOT:$<CONFIG:Debug>>:${NVTT_RELEASE_DLL_PATH}>$<$<CONFIG:Debug>:${NVTT_DEBUG_DLL_PATH}>")
else ()
  find_library(NVTT_BASE_LIBRARY nvtt PATH_SUFFIXES "lib/static" HINTS ${NVTT_SEARCH_DIRS})
  find_library(NVTT_CORE_LIBRARY nvcore PATH_SUFFIXES "lib/static" HINTS ${NVTT_SEARCH_DIRS})
  find_library(NVTT_IMAGE_LIBRARY nvimage PATH_SUFFIXES "lib/static" HINTS ${NVTT_SEARCH_DIRS})
  find_library(NVTT_MATH_LIBRARY nvmath PATH_SUFFIXES "lib/static" HINTS ${NVTT_SEARCH_DIRS})
  find_library(NVTT_THREAD_LIBRARY nvthread PATH_SUFFIXES "lib/static" HINTS ${NVTT_SEARCH_DIRS})

  find_package_handle_standard_args(NVTT DEFAULT_MSG NVTT_INCLUDE_DIRS NVTT_BASE_LIBRARY NVTT_CORE_LIBRARY NVTT_IMAGE_LIBRARY NVTT_MATH_LIBRARY NVTT_THREAD_LIBRARY)
  set(NVTT_LIBRARIES ${NVTT_BASE_LIBRARY} ${NVTT_CORE_LIBRARY} ${NVTT_IMAGE_LIBRARY} ${NVTT_MATH_LIBRARY} ${NVTT_THREAD_LIBRARY})
endif ()
