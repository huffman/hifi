#
#  FindCrashrpt.cmake
#
#  Try to find the Crashrpt library
#
#  Once done this will define
#
#  CRASHRPT_FOUND - system found Crashrpt
#  CRASHRPT_INCLUDE_DIRS - the Crashrpt include directory
#  CRASHRPT_LIBRARIES - link to this to use Crashrpt
#
#  Created on 2/01/2016 by Ryan Huffman
#  Copyright 2016 High Fidelity, Inc.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

include(SelectLibraryConfigurations)
# include(AddPathsToFixupLibs)


select_library_configurations(CRASHRPT)

set(CRASHRPT_LIBRARIES ${CRASHRPT_LIBRARY})

mark_as_advanced(CRASHRPT_LIBRARIES CRASHRPT_INCLUDE_DIRS)

if (WIN32)
  add_paths_to_fixup_libs(${CRASHRPT_DLL_PATH})
endif ()

message("Crashrpt_LIBRARIES is: " ${CRASHRPT_LIBRARIES})
