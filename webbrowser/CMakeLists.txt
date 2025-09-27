set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(webbrowser)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)

#
# CEF configuration
#

# Specify the CEF distribution version
set(CEF_VERSION "140.1.14+geb1c06e+chromium-140.0.7339.185")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm")
        set(CEF_PLATFORM "linuxarm")
    elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(CEF_PLATFORM "linuxarm64")
    elseif (CMAKE_SIZEOF_VOID_P MATCHES 8)
        set(CEF_PLATFORM "linux64")
    else ()
        message(FATAL_ERROR "Linux x86 32-bit builds are discontinued.")
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(CEF_PLATFORM "macosarm64")
    else ()
        set(CEF_PLATFORM "macosx64")
    endif ()
else ()
    message(FATAL_ERROR "Unsupported host platform: ${CMAKE_SYSTEM_NAME}")
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
# Python configuration
#

# Support specification of the Python executable path via the command-line
if (DEFINED ENV{PYTHON_EXECUTABLE})
    file(TO_CMAKE_PATH "$ENV{PYTHON_EXECUTABLE}" PYTHON_EXECUTABLE)
endif ()

if (NOT PYTHON_EXECUTABLE)
    unset(PYTHON_EXECUTABLE)

    # Find the python interpreter
    find_package(PythonInterp)

    if (NOT ${PYTHONINTERP_FOUND})
        message(FATAL_ERROR "A Python installation is required. Set the "
                "PYTHON_EXECUTABLE environment variable to explicitly "
                "specify the Python executable path.")
    endif ()
endif ()

message(STATUS "Using Python: ${PYTHON_EXECUTABLE}")

#
# Clang-format configuration
#

if (APPLE)
    # download_from_google_storage.py compares the supplied --platform regex
    # against sys.platform which is "darwin" on macOS. Accept that spelling so
    # the tool actually downloads the formatter instead of skipping it.
    set(GS_PLATFORM "darwin")
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(GS_HASHPATH "mac/clang-format.arm64.sha1")
        set(GS_OUTPATH "mac/clang-format.arm64")
    else ()
        set(GS_HASHPATH "mac/clang-format.x64.sha1")
        set(GS_OUTPATH "mac/clang-format.x64")
    endif ()
else ()
    # Match both "linux" and legacy "linux2" style sys.platform values.
    set(GS_PLATFORM "linux.*")
    set(GS_HASHPATH "linux64/clang-format.sha1")
    set(GS_OUTPATH "linux64/clang-format")
endif ()

set(GS_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/tools/buildtools/${GS_OUTPATH}")

if (EXISTS "${GS_OUTPUT}")
    message(STATUS "Using cached clang-format at ${GS_OUTPUT}")
else ()
    message(STATUS "Downloading clang-format from Google Storage...")
    execute_process(
            COMMAND "${PYTHON_EXECUTABLE}"
            "tools/buildtools/download_from_google_storage.py"
            "--no_resume"
            "--platform=${GS_PLATFORM}"
            "--no_auth"
            "--bucket" "chromium-clang-format"
            "-s" "tools/buildtools/${GS_HASHPATH}"
            "-o" "tools/buildtools/${GS_OUTPATH}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE EXECUTE_RV
    )
    if (EXECUTE_RV STREQUAL "0" AND EXISTS "${GS_OUTPUT}")
        message(STATUS "clang-format download complete")
    else ()
        find_program(SYSTEM_CLANG_FORMAT_EXECUTABLE NAMES clang-format)
        if (SYSTEM_CLANG_FORMAT_EXECUTABLE)
            message(WARNING
                    "Failed to download bundled clang-format (exit code ${EXECUTE_RV}). "
                    "Falling back to system clang-format at ${SYSTEM_CLANG_FORMAT_EXECUTABLE}. "
                    "Install the Python 'six' module to enable automatic downloads.")
        else ()
            message(WARNING
                    "Failed to download bundled clang-format (exit code ${EXECUTE_RV}). "
                    "No system clang-format executable was found. Please install the "
                    "Python 'six' module or provide clang-format manually if formatting is required.")
        endif ()
    endif ()
endif ()

#
# Target configuration
#

# Include the libcef_dll_wrapper target (executes libcef_dll/CMakeLists.txt)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

if (APPLE)
    # Xcode 16+ passes a `-target arm64-apple-macosXX.Y` flag that overrides the
    # `-mmacosx-version-min=12.0` argument baked into the CEF SDK sources.  That
    # mismatch triggers `-Werror,-Woverriding-option` during compilation of the
    # wrapper target.  Suppress the single diagnostic so the build can proceed
    # while still honouring the rest of CEF's warning settings.
    target_compile_options(libcef_dll_wrapper PRIVATE "-Wno-error=overriding-option")
endif ()

# Allow includes relative to the current source directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(_CAIRO_TARGET "")
set(_CAIRO_INCLUDE_HINTS "")

if (APPLE)
    set(_brew_prefixes "/opt/homebrew" "/usr/local")
    foreach(_prefix IN LISTS _brew_prefixes)
        if (NOT _cairo_root AND EXISTS "${_prefix}/opt/cairo")
            set(_cairo_root "${_prefix}/opt/cairo")
        endif ()
    endforeach ()

    if (_cairo_root AND EXISTS "${_cairo_root}/lib/libcairo.dylib")
        set(_CAIRO_TARGET "${_cairo_root}/lib/libcairo.dylib")
        if (EXISTS "${_cairo_root}/include")
            list(APPEND _CAIRO_INCLUDE_HINTS "${_cairo_root}/include")
        endif ()
    endif ()

    if (NOT _CAIRO_TARGET)
        message(FATAL_ERROR "Cairo not found in Homebrew. Install it with 'brew install cairo'.")
    endif ()
else ()
    find_package(Cairo REQUIRED)

    if (TARGET Cairo::Cairo)
        set(_CAIRO_TARGET Cairo::Cairo)
    elseif (DEFINED CAIRO_LIBRARIES)
        set(_CAIRO_TARGET ${CAIRO_LIBRARIES})
        if (DEFINED CAIRO_INCLUDE_DIRS)
            list(APPEND _CAIRO_INCLUDE_HINTS ${CAIRO_INCLUDE_DIRS})
        elseif (DEFINED CAIRO_INCLUDE_DIR)
            list(APPEND _CAIRO_INCLUDE_HINTS ${CAIRO_INCLUDE_DIR})
        endif ()
    else ()
        message(FATAL_ERROR "Cairo configuration did not provide libraries to link against.")
    endif ()
endif ()

if (NOT _CAIRO_TARGET)
    message(FATAL_ERROR "Cairo not found. Install Cairo or provide the library path explicitly.")
endif ()

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
    if (APPLE)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path")
    else ()
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
    endif ()
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

if (APPLE)
    # Xcode injects a deployment target via -target which collides with the
    # fixed -mmacosx-version-min flag bundled in the CEF SDK. Demote the
    # resulting overriding-option diagnostic back to a warning so the build
    # continues without relaxing the rest of the warning budget.
    target_compile_options(webbrowser PRIVATE "-Wno-error=overriding-option")
endif ()

# Logical target used to link the libcef library. On macOS CEF ships as a
# framework whose binary lives inside the "Chromium Embedded Framework"
# bundle. The FindCEF module does not currently populate the CEF_LIB_* paths
# with that framework binary, so do it here if necessary to avoid link errors.
if (APPLE)
    set(_cef_framework "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework/Chromium Embedded Framework")
    if (NOT CEF_LIB_DEBUG AND EXISTS "${CEF_BINARY_DIR}/Debug")
        set(CEF_LIB_DEBUG "${CEF_BINARY_DIR}/Debug/Chromium Embedded Framework.framework/Chromium Embedded Framework")
    elseif (NOT CEF_LIB_DEBUG)
        set(CEF_LIB_DEBUG "${_cef_framework}")
    endif ()
    if (NOT CEF_LIB_RELEASE AND EXISTS "${CEF_BINARY_DIR}/Release")
        set(CEF_LIB_RELEASE "${CEF_BINARY_DIR}/Release/Chromium Embedded Framework.framework/Chromium Embedded Framework")
    elseif (NOT CEF_LIB_RELEASE)
        set(CEF_LIB_RELEASE "${_cef_framework}")
    endif ()
endif ()

# Logical target used to link the libcef library
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

add_dependencies(webbrowser libcef_dll_wrapper)
target_link_libraries(webbrowser libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS} revyv ${_CAIRO_TARGET})

if (_CAIRO_INCLUDE_HINTS)
    target_include_directories(webbrowser PRIVATE ${_CAIRO_INCLUDE_HINTS})
endif ()

# Copy binary and resource files to the target output directory
COPY_FILES("webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
COPY_FILES("webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

if (NOT APPLE)
    # Output a message about setting SUID permissions on the chrome-sandbox target
    SET_LINUX_SUID_PERMISSIONS("webbrowser" "${CMAKE_CURRENT_BINARY_DIR}/chrome-sandbox")
endif ()

# Display configuration settings.
PRINT_CEF_CONFIG()
