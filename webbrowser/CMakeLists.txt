set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(webbrowser)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)

#
# CEF configuration
#

# Specify the CEF distribution version
set(CEF_VERSION "106.1.1+g5891c70+chromium-106.0.5249.119")

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm")
    set(CEF_PLATFORM "linuxarm")
elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
    set(CEF_PLATFORM "linuxarm64")
elseif (CMAKE_SIZEOF_VOID_P MATCHES 8)
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

set(GS_PLATFORM "linux*")
set(GS_HASHPATH "linux64/clang-format.sha1")
set(GS_OUTPATH "linux64/clang-format")

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
if (NOT EXECUTE_RV STREQUAL "0")
    message(FATAL_ERROR "Execution failed with unexpected result: ${EXECUTE_RV}")
endif ()

#
# Target configuration
#

# Include the libcef_dll_wrapper target (executes libcef_dll/CMakeLists.txt)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Allow includes relative to the current source directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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
