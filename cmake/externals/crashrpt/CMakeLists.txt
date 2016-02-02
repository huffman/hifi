include(ExternalProject)
include(SelectLibraryConfigurations)

set(EXTERNAL_NAME crashrpt)


string(TOUPPER ${EXTERNAL_NAME} EXTERNAL_NAME_UPPER)

message("proj prefix: " ${EXTERNAL_PROJECT_PREFIX})

if (WIN32)
  message(adding crashrpt)
  ExternalProject_Add(
    ${EXTERNAL_NAME}
    URL http://localhost:8000/crash_rpt.zip
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCRASHRPT_BUILD_SHARED_LIBS=1
    INSTALL_COMMAND ""
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    BINARY_DIR ${EXTERNAL_PROJECT_PREFIX}/build
  )
endif ()

set_target_properties(${EXTERNAL_NAME} PROPERTIES FOLDER "hidden/externals")

ExternalProject_Get_Property(${EXTERNAL_NAME} SOURCE_DIR)
ExternalProject_Get_Property(${EXTERNAL_NAME} BINARY_DIR)

set(CRASHRPT_LIB_DIR "${BINARY_DIR}/x64/lib")
set(CRASHRPT_LIB_EXT "lib")
set(CRASHRPT_DLL_DIR "${BINARY_DIR}/x64/bin" CACHE FILEPATH "CrashRpt dll dir")

set(CRASHRPT_SENDER_PATH "${BINARY_DIR}/bin/x64/CrashSender1403.exe" CACHE FILEPATH "CrashRpt CrashSender.exe path")

set(${EXTERNAL_NAME_UPPER}_INCLUDE_DIRS ${SOURCE_DIR}/include CACHE FILEPATH "Path to crashrpt include directory")
set(${EXTERNAL_NAME_UPPER}_LIBRARY_DEBUG ${BINARY_DIR}/lib/x64/CrashRpt1403d.lib CACHE FILEPATH "Crashrpt debug library")
set(${EXTERNAL_NAME_UPPER}_LIBRARY_RELEASE ${BINARY_DIR}/lib/x64/CrashRpt1403.lib CACHE FILEPATH "Crashrpt release library")

set(${EXTERNAL_NAME_UPPER}_DLL_PATH ${BINARY_DIR}/bin/x64 CACHE FILEPATH "Crashrpt dll path")
set(${EXTERNAL_NAME_UPPER}_DLL_DEBUG ${BINARY_DIR}/bin/x64/CrashRpt1403d.dll CACHE FILEPATH "Crashrpt debug dll")
set(${EXTERNAL_NAME_UPPER}_DLL_RELEASE ${BINARY_DIR}/bin/x64/CrashRpt1403.dll CACHE FILEPATH "Crashrpt release dll")

# set(${EXTERNAL_NAME_UPPER}_LIBRARY_DEBUG ${BINARY_DIR}/lib/x64/CrashRpt1403d.lib CACHE FILEPATH "Crashrpt debug library")
# set(${EXTERNAL_NAME_UPPER}_LIBRARY_RELEASE ${BINARY_DIR}/lib/x64/CrashRpt1403.lib CACHE FILEPATH "Crashrpt release library")

message("Include dirs is: " ${CRASHRPT_INCLUDE_DIRS})
message("Library dirs is: " ${CRASHRPT_LIBRARY_DEBUG})
