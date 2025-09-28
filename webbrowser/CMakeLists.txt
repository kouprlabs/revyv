cmake_minimum_required(VERSION 3.20)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(webbrowser)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)

#
# CEF configuration
#

# Specify the CEF distribution version
set(CEF_VERSION "140.1.14+geb1c06e+chromium-140.0.7339.185")

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm")
    set(CEF_PLATFORM "linuxarm")
elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64" OR
        "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    set(CEF_PLATFORM "linuxarm64")
elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR
        "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "amd64" OR
        CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CEF_PLATFORM "linux64")
else ()
    message(FATAL_ERROR "Linux x86 32-bit builds are discontinued.")
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

#
# Clang-format configuration
#

set(GS_HASHPATH "linux64/clang-format.sha1")
set(GS_OUTPATH "linux64/clang-format")
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
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
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

    # Set rpath so that libraries can be placed next to the executable.
    set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
    set_target_properties(${target} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
endmacro()

set(SOURCES
        src/main.cpp
        src/render_handler.cpp
        src/render_handler.h
        src/browser_client.cpp
        src/browser_client.h
        src/event_thread.cpp
        src/event_thread.h
        src/chromium_keycodes.h)
APPEND_PLATFORM_SOURCES(${SOURCES})

add_executable(webbrowser ${SOURCES})

SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(webbrowser)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
if (NOT TARGET revyv)
    add_subdirectory("${REPO_ROOT}/librevyv" librevyv)
endif ()
target_include_directories(webbrowser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${REPO_ROOT}/thirdparty/argh"
        "${REPO_ROOT}/librevyv/include"
        "${REPO_ROOT}/compositor/include")

# Logical target used to link the libcef library
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

add_dependencies(webbrowser libcef_dll_wrapper)
target_link_libraries(webbrowser libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS} revyv cairo)

# Copy binary and resource files to the target output directory
COPY_FILES("webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
COPY_FILES("webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

# Output a message about setting SUID permissions on the chrome-sandbox target
SET_LINUX_SUID_PERMISSIONS("webbrowser" "${CMAKE_CURRENT_BINARY_DIR}/chrome-sandbox")

# Display configuration settings.
PRINT_CEF_CONFIG()
