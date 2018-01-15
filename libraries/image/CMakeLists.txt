set(TARGET_NAME image)
setup_hifi_library()
link_hifi_libraries(shared gpu)

if (NOT ANDROID)

    add_dependency_external_projects(nvtt)
    find_package(NVTT REQUIRED)

    #list(APPEND NVTT_LIBRARIES "/home/ryan/dev/hifi/build/ext/makefiles/nvtt/project/lib/libnvimage.so")
    #list(APPEND NVTT_LIBRARIES "/home/ryan/dev/hifi/build/ext/makefiles/nvtt/project/lib/libnvmath.so")
    #list(APPEND NVTT_LIBRARIES "/home/ryan/dev/hifi/build/ext/makefiles/nvtt/project/lib/libnvcore.so")

    message("NVTT Libs: " ${NVTT_LIBRARIES} ${NVTT_BUILD_DIR})

    target_include_directories(${TARGET_NAME} PRIVATE ${NVTT_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} ${NVTT_LIBRARIES})
    add_paths_to_fixup_libs(${NVTT_DLL_PATH})
endif()
