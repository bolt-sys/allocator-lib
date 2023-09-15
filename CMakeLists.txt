cmake_minimum_required(VERSION 3.27)
project(AllocatorLib
        LANGUAGES C
        VERSION 0.1.0
        DESCRIPTION "Kernel memory allocation libary for Bolt"
)


if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

    file(GLOB_RECURSE SOURCES
            source/*.c
            source/*.h
            executable/*.c
            executable/*.h
    )

    add_executable(AllocatorLib main.c)
    target_sources(
            AllocatorLib
            PRIVATE
            ${SOURCES}
    )
    target_compile_options(
            AllocatorLib
            PRIVATE
            -Wall -Wextra -Werror
            -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-ignored-attributes
    )
else ()
    set(BOLT_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")
    set(BOLT_TARGET_MODE "KERNEL" CACHE STRING "Target mode e.g. KERNEL, USER")
    set(BOLT_BUILD_TESTS OFF CACHE BOOL "Build tests")

    include(GNUInstallDirs)
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Location of header files")

    set(C_OPTIONS "")

    list(APPEND C_OPTIONS "-Wall -Wextra -Werror")
    list(APPEND C_OPTIONS "-Wno-unused-parameter" "-Wno-unused-function" "-Wno-unused-variable" "-Wno-ignored-attributes")

    # Freestanding environment
    list(APPEND C_OPTIONS "-ffreestanding" "-nostdlib" "-nostdinc")

    # Stack protection are not possible in some cases.
    list(APPEND C_OPTIONS "-fno-stack-protector" "-fno-stack-check")

    # Disable features we cannot use in kernel mode
    if (BOLT_TARGET_MODE STREQUAL "KERNEL")
        list(APPEND C_OPTIONS "-mno-80387")    # Disable FPU (Requires additional setup)
        list(APPEND C_OPTIONS "-mno-mmx")      # Disable MMX  (Requires FPU)
        list(APPEND C_OPTIONS "-mno-sse")      # Disable SSE  (Requires MMX)
        list(APPEND C_OPTIONS "-mno-sse2")     # Disable SSE2 (Requires MMX)
        list(APPEND C_OPTIONS "-mno-red-zone") # Disable red zone, causes issues with interrupts
    endif ()

    target_include_directories(
            AllocatorLib
            PUBLIC
            $(CMAKE_CURRENT_LIST_DIR)/include
            PRIVATE
            $(CMAKE_CURRENT_LIST_DIR)/source
    )

    file(GLOB_RECURSE SOURCES
            source/*.c
            source/*.h
    )

    add_library(AllocatorLib STATIC ${SOURCES})

    if (BOLT_TARGET_MODE STREQUAL "KERNEL")
        target_compile_definitions(
                RuntimeLib
                PUBLIC
                -DBOLT_KERNEL=1
        )
    elseif (BOLT_TARGET_MODE STREQUAL "USER")
        target_compile_definitions(
                RuntimeLib
                PUBLIC
                -DBOLT_USER=1
        )
    else ()
        message(FATAL_ERROR "Invalid target mode: ${BOLT_TARGET_MODE}")
    endif ()

    set_target_properties(
            RuntimeLib
            PROPERTIES
            C_STANDARD 11
            C_STANDARD_REQUIRED YES    # Require C11
            C_EXTENSIONS NO     # Disable GNU extensions
            C_VISIBILITY_PRESET hidden # Don't export symbols by default
    )

    target_compile_options(
            RuntimeLib
            PRIVATE
            "$<$<COMPILE_LANGUAGE:C>:${C_OPTIONS}>"
    )

    # Testing
    if (BOLT_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif ()

    # Install

    install(
            TARGETS
            RuntimeLib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(
            DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    include(CMakePackageConfigHelpers)

    configure_package_config_file(
            "${CMAKE_CURRENT_LIST_DIR}/cmake/RuntimeLibConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/RuntimeLibConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RuntimeLib
            PATH_VARS INCLUDE_INSTALL_DIR
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/RuntimeLibConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    install(
            FILES
            "${CMAKE_CURRENT_BINARY_DIR}/RuntimeLibConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/RuntimeLibConfigVersion.cmake"
            DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/RuntimeLib
    )

    # CPack
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VENDOR "Bolt")
    set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${BOLT_TARGET_ARCH}-${BOLT_TARGET_MODE}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_LIST_DIR}/README.md")
    set(CPACK_GENERATOR "ZIP")
    include(CPack)

endif ()