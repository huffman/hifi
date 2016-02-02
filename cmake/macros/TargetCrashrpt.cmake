#
#  Copyright 2016 High Fidelity, Inc.
#  Created by Ryan Huffman on 2016-1-28
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

macro(TARGET_CRASHRPT)
  if (WIN32)
    add_dependency_external_projects(crashrpt)
    find_package(Crashrpt REQUIRED)

    target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${CRASHRPT_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} ${CRASHRPT_LIBRARIES})
    message("Adding dll dir " ${CRASHRPT_DLL_DIR})
    message("Adding " ${CRASHRPT_INCLUDE_DIRS})
    message("Adding " ${CRASHRPT_LIBRARY_RELEASE})
    message("Adding " ${CRASHRPT_LIBRARIES})
  endif ()
endmacro()
