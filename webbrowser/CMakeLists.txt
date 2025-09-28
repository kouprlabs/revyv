cmake_minimum_required(VERSION 3.20)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(webbrowser)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#
# CEF configuration
#

# Specify the CEF distribution version
set(CEF_VERSION "140.1.14+geb1c06e+chromium-140.0.7339.185")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    if (CMAKE_OSX_ARCHITECTURES)
        list(GET CMAKE_OSX_ARCHITECTURES 0 PROJECT_ARCH)
    else ()
        set(PROJECT_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif ()
    if ("${PROJECT_ARCH}" STREQUAL "arm64")
        set(PROJECT_ARCH "arm64")
        set(CEF_PLATFORM "macosarm64")
    else ()
        set(PROJECT_ARCH "x86_64")
        set(CEF_PLATFORM "macosx64")
    endif ()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm")
        set(PROJECT_ARCH "arm")
        set(CEF_PLATFORM "linuxarm")
    elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64" OR
            "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
        set(PROJECT_ARCH "arm64")
        set(CEF_PLATFORM "linuxarm64")
    elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR
            "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "amd64" OR
            CMAKE_SIZEOF_VOID_P MATCHES 8)
        set(PROJECT_ARCH "x86_64")
        set(CEF_PLATFORM "linux64")
    else ()
        message(FATAL_ERROR "Linux x86 32-bit builds are discontinued.")
    endif ()
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

if (OS_MACOSX)
    set(CMAKE_OSX_ARCHITECTURES "${PROJECT_ARCH}" CACHE STRING
            "Target architecture for macOS builds" FORCE)
    # CEF 140 binaries are built against macOS 12 and must be linked with the
    # same minimum deployment target to avoid toolchain mismatches when
    # compiling the helper libraries.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING
            "Minimum macOS version to target" FORCE)
endif ()

# Add this project's cmake/ directory to the module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Download and extract the CEF binary distribution (executes DownloadCEF.cmake)
include(DownloadCEF)
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${PROJECT_SOURCE_DIR}/third_party/cef")

# Add the CEF binary distribution's cmake/ directory to the module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Load the CEF configuration (executes FindCEF.cmake)
find_package(CEF REQUIRED)

# Set the configuration-specific binary output directory similar to CEF examples.
if (CMAKE_GENERATOR MATCHES "Ninja" OR CMAKE_GENERATOR MATCHES "Unix Makefiles")
    if (CMAKE_BUILD_TYPE)
        set(EXAMPLE_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    else ()
        set(EXAMPLE_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
    endif ()
else ()
    set(EXAMPLE_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
endif ()
set(CEF_TARGET_OUT_DIR "${EXAMPLE_TARGET_OUT_DIR}")

#
# Clang-format configuration
#

if (OS_MACOSX)
    if ("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(GS_HASHPATH "mac/clang-format.arm64.sha1")
    else ()
        set(GS_HASHPATH "mac/clang-format.x64.sha1")
    endif ()
    set(GS_OUTPATH "mac/clang-format")
elseif (OS_LINUX)
    set(GS_HASHPATH "linux64/clang-format.sha1")
    set(GS_OUTPATH "linux64/clang-format")
elseif (OS_WINDOWS)
    set(GS_HASHPATH "win/clang-format.exe.sha1")
    set(GS_OUTPATH "win/clang-format.exe")
else ()
    message(FATAL_ERROR "Unsupported platform for clang-format download")
endif ()

set(GS_SHA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tools/buildtools/${GS_HASHPATH}")
set(GS_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/tools/buildtools/${GS_OUTPATH}")

if (NOT EXISTS "${GS_SHA_FILE}")
    message(FATAL_ERROR "Missing clang-format hash file: ${GS_SHA_FILE}")
endif ()

if (NOT EXISTS "${GS_OUTPUT}")
    file(READ "${GS_SHA_FILE}" GS_SHA1)
    string(STRIP "${GS_SHA1}" GS_SHA1)
    set(GS_URL "https://storage.googleapis.com/chromium-clang-format/${GS_SHA1}")

    message(STATUS "Downloading clang-format from ${GS_URL}...")
    file(DOWNLOAD
            "${GS_URL}"
            "${GS_OUTPUT}"
            STATUS DOWNLOAD_STATUS
            SHOW_PROGRESS)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)
    if (NOT DOWNLOAD_STATUS_CODE EQUAL 0)
        list(GET DOWNLOAD_STATUS 1 DOWNLOAD_STATUS_MESSAGE)
        message(FATAL_ERROR "Failed to download clang-format: ${DOWNLOAD_STATUS_MESSAGE}")
    endif ()
endif ()

file(CHMOD "${GS_OUTPUT}" PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

#
# Target configuration
#

# Include the libcef_dll_wrapper target (executes libcef_dll/CMakeLists.txt)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Set properties common to all example targets.
macro(SET_EXAMPLE_PROPERTIES target)
    # Output all binaries to the configuration-specific build directory.
    set_target_properties(${target} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${EXAMPLE_TARGET_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EXAMPLE_TARGET_OUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${EXAMPLE_TARGET_OUT_DIR}")

    if (OS_MACOSX OR OS_WINDOWS)
        set_property(TARGET ${target} PROPERTY FOLDER "webbrowser")
    endif ()
endmacro()

# Set properties on an example library target.
macro(SET_EXAMPLE_LIBRARY_TARGET_PROPERTIES target)
    SET_LIBRARY_TARGET_PROPERTIES(${target})
    SET_EXAMPLE_PROPERTIES(${target})
endmacro()

# Set properties on an example executable target.
macro(SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES target)
    SET_EXECUTABLE_TARGET_PROPERTIES(${target})
    SET_EXAMPLE_PROPERTIES(${target})

    if (OS_LINUX)
        # Set rpath so that libraries can be placed next to the executable.
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
        set_target_properties(${target} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    elseif (OS_MAC)
        # Locate dependent libraries inside the app bundle.
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path/../Frameworks")
        set_target_properties(${target} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    endif ()
endmacro()

set(WEBBROWSER_SRCS
        src/main.cpp
        src/render_handler.cpp
        src/render_handler.h
        src/browser_client.cpp
        src/browser_client.h
        src/event_thread.cpp
        src/event_thread.h
        src/chromium_keycodes.h)
APPEND_PLATFORM_SOURCES(WEBBROWSER_SRCS)

set(WEBBROWSER_HELPER_SRCS)
set(WEBBROWSER_HELPER_SRCS_MAC
        src/process_helper_mac.cpp)
APPEND_PLATFORM_SOURCES(WEBBROWSER_HELPER_SRCS)

set(WEBBROWSER_RESOURCES_SRCS)
set(WEBBROWSER_RESOURCES_SRCS_MAC
        mac/English.lproj/InfoPlist.strings
        mac/English.lproj/MainMenu.xib)
APPEND_PLATFORM_SOURCES(WEBBROWSER_RESOURCES_SRCS)

if (OS_MAC)
    add_executable(webbrowser MACOSX_BUNDLE ${WEBBROWSER_SRCS} ${WEBBROWSER_RESOURCES_SRCS})
else ()
    add_executable(webbrowser ${WEBBROWSER_SRCS})
endif ()

SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(webbrowser)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
if (NOT TARGET revyv)
    add_subdirectory("${REPO_ROOT}/librevyv" librevyv)
endif ()

if (OS_LINUX)
    # Ensure the webbrowser executable can locate librevyv at runtime without
    # requiring users to export LD_LIBRARY_PATH manually. The generator
    # expression resolves to the directory containing the built shared library
    # regardless of whether librevyv was added by the top-level project or via
    # the local add_subdirectory above.
    set_property(TARGET webbrowser APPEND PROPERTY INSTALL_RPATH
            "$<TARGET_FILE_DIR:revyv>")
endif ()

target_include_directories(webbrowser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${REPO_ROOT}/thirdparty/argh"
        "${REPO_ROOT}/librevyv/include"
        "${REPO_ROOT}/compositor/include")

# Logical target used to link the libcef library
if (OS_LINUX OR OS_WINDOWS)
    ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
endif ()

add_dependencies(webbrowser libcef_dll_wrapper)

if (OS_MAC)
    target_link_libraries(webbrowser PRIVATE libcef_dll_wrapper ${CEF_STANDARD_LIBS} revyv cairo)
else ()
    target_link_libraries(webbrowser PRIVATE libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS} revyv cairo)
endif ()

if (OS_MAC)
    set(WEBBROWSER_APP "${EXAMPLE_TARGET_OUT_DIR}/webbrowser.app")
    set(EXECUTABLE_NAME "webbrowser")
    set(PRODUCT_NAME "webbrowser")
    set(WEBBROWSER_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist.in")
    set(WEBBROWSER_HELPER_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist.in")

    set_target_properties(webbrowser PROPERTIES
            RESOURCE "${WEBBROWSER_RESOURCES_SRCS}"
            MACOSX_BUNDLE_INFO_PLIST "${WEBBROWSER_INFO_PLIST}")

    # Copy the CEF framework into the Frameworks directory and create the
    # necessary symlinks.
    COPY_MAC_FRAMEWORK("webbrowser" "${CEF_BINARY_DIR}" "${WEBBROWSER_APP}")

    set(WEBBROWSER_HELPER_TARGET "webbrowser_Helper")
    set(WEBBROWSER_HELPER_OUTPUT_NAME "webbrowser Helper")

    foreach (_suffix_list ${CEF_HELPER_APP_SUFFIXES})
        string(REPLACE ":" ";" _suffix_list ${_suffix_list})
        list(GET _suffix_list 0 _name_suffix)
        list(GET _suffix_list 1 _target_suffix)
        list(GET _suffix_list 2 _plist_suffix)

        set(_helper_target "${WEBBROWSER_HELPER_TARGET}${_target_suffix}")
        set(_helper_output_name "${WEBBROWSER_HELPER_OUTPUT_NAME}${_name_suffix}")

        set(_helper_info_plist "${CMAKE_CURRENT_BINARY_DIR}/helper-Info${_target_suffix}.plist.in")
        file(READ "${WEBBROWSER_HELPER_INFO_PLIST}" _plist_contents)
        string(REPLACE "\${EXECUTABLE_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${PRODUCT_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${BUNDLE_ID_SUFFIX}" "${_plist_suffix}" _plist_contents ${_plist_contents})
        file(WRITE ${_helper_info_plist} ${_plist_contents})

        add_executable(${_helper_target} MACOSX_BUNDLE ${WEBBROWSER_HELPER_SRCS})
        SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(${_helper_target})
        add_dependencies(${_helper_target} libcef_dll_wrapper)
        target_link_libraries(${_helper_target} PRIVATE libcef_dll_wrapper ${CEF_STANDARD_LIBS})
        set_target_properties(${_helper_target} PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST ${_helper_info_plist}
                OUTPUT_NAME ${_helper_output_name})

        add_dependencies(webbrowser "${_helper_target}")

        add_custom_command(TARGET webbrowser
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${EXAMPLE_TARGET_OUT_DIR}/${_helper_output_name}.app"
                "${WEBBROWSER_APP}/Contents/Frameworks/${_helper_output_name}.app"
                VERBATIM)
    endforeach ()

    set(WEBBROWSER_RESOURCE_PREFIXES
            "mac/")
    COPY_MAC_RESOURCES("${WEBBROWSER_RESOURCES_SRCS}" "${WEBBROWSER_RESOURCE_PREFIXES}"
            "webbrowser" "${CMAKE_CURRENT_SOURCE_DIR}" "${WEBBROWSER_APP}")

    add_custom_command(TARGET webbrowser
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${WEBBROWSER_APP}/Contents/Frameworks"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:revyv>"
                    "${WEBBROWSER_APP}/Contents/Frameworks/"
            VERBATIM)
else ()
    # Copy binary and resource files to the target output directory
    COPY_FILES("webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
    COPY_FILES("webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

    # Output a message about setting SUID permissions on the chrome-sandbox target
    SET_LINUX_SUID_PERMISSIONS("webbrowser" "${CMAKE_CURRENT_BINARY_DIR}/chrome-sandbox")
endif ()

# Display configuration settings.
PRINT_CEF_CONFIG()
