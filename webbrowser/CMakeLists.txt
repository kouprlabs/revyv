set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(webbrowser)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)

#
# CEF configuration
#

# Specify the CEF distribution version
set(CEF_VERSION "140.1.14+geb1c06e+chromium-140.0.7339.185")

function(DownloadCEF platform version download_dir)
    set(CEF_DISTRIBUTION "cef_binary_${version}_${platform}")
    set(CEF_DOWNLOAD_DIR "${download_dir}")

    set(CEF_ROOT "${CEF_DOWNLOAD_DIR}/${CEF_DISTRIBUTION}" CACHE INTERNAL "CEF_ROOT")

    if (NOT IS_DIRECTORY "${CEF_ROOT}")
        set(CEF_DOWNLOAD_FILENAME "${CEF_DISTRIBUTION}.tar.bz2")
        set(CEF_DOWNLOAD_PATH "${CEF_DOWNLOAD_DIR}/${CEF_DOWNLOAD_FILENAME}")

        if (NOT EXISTS "${CEF_DOWNLOAD_PATH}")
            set(CEF_DOWNLOAD_URL "https://cef-builds.spotifycdn.com/${CEF_DOWNLOAD_FILENAME}")
            string(REPLACE "+" "%2B" CEF_DOWNLOAD_URL_ESCAPED ${CEF_DOWNLOAD_URL})

            message(STATUS "Downloading ${CEF_DOWNLOAD_PATH}.sha1 from ${CEF_DOWNLOAD_URL_ESCAPED}...")
            file(DOWNLOAD "${CEF_DOWNLOAD_URL_ESCAPED}.sha1" "${CEF_DOWNLOAD_PATH}.sha1")
            file(READ "${CEF_DOWNLOAD_PATH}.sha1" CEF_SHA1)

            message(STATUS "Downloading ${CEF_DOWNLOAD_PATH}...")
            file(
                    DOWNLOAD "${CEF_DOWNLOAD_URL_ESCAPED}" "${CEF_DOWNLOAD_PATH}"
                    EXPECTED_HASH SHA1=${CEF_SHA1}
                    SHOW_PROGRESS)
        endif ()

        message(STATUS "Extracting ${CEF_DOWNLOAD_PATH}...")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf "${CEF_DOWNLOAD_DIR}/${CEF_DOWNLOAD_FILENAME}"
                WORKING_DIRECTORY ${CEF_DOWNLOAD_DIR})
    endif ()
endfunction()

function(_cef_get_helper_src dest pair_list result_var)
    set(_result "")
    foreach(_pair IN LISTS ${pair_list})
        if(_pair)
            string(REPLACE "|" ";" _parts "${_pair}")
            list(LENGTH _parts _len)
            if(_len GREATER 1)
                list(GET _parts 1 _name)
                if(_name STREQUAL "${dest}")
                    list(GET _parts 0 _src)
                    set(_result "${_src}")
                    break()
                endif()
            endif()
        endif()
    endforeach()
    set(${result_var} "${_result}" PARENT_SCOPE)
endfunction()

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

# Download and extract the CEF binary distribution
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
    set(_cef_release_framework "")
    set(_cef_debug_framework "")
    set(_cef_release_bundle "")
    set(_cef_debug_bundle "")
    set(_cef_release_resources "")
    set(_cef_debug_resources "")

    # Multi-config generators expose CEF_BINARY_DIR using a generator expression
    # (for example, "$<CONFIGURATION>") which cannot be resolved while
    # configuring. Probe the extraction root instead so we can verify the
    # unpacked framework layout up-front.
    set(_cef_probe_root "${CEF_BINARY_DIR}")
    string(FIND "${_cef_probe_root}" "\$<CONFIGURATION>" _cef_config_index)
    if (_cef_config_index GREATER -1)
        set(_cef_probe_root "${CEF_ROOT}")
    endif ()

    set(_cef_bundle_candidates
            "${_cef_probe_root}/Chromium Embedded Framework.framework"
            "${_cef_probe_root}/Release/Chromium Embedded Framework.framework")

    foreach(_candidate IN LISTS _cef_bundle_candidates)
        if (EXISTS "${_candidate}/Chromium Embedded Framework")
            set(_cef_release_bundle "${_candidate}")
            break()
        endif ()
    endforeach ()

    if (EXISTS "${_cef_probe_root}/Debug/Chromium Embedded Framework.framework/Chromium Embedded Framework")
        set(_cef_debug_bundle "${_cef_probe_root}/Debug/Chromium Embedded Framework.framework")
    endif ()

    if (NOT _cef_release_bundle)
        message(FATAL_ERROR
                "Chromium Embedded Framework binary not found in ${_cef_probe_root}. "
                "Verify that the CEF archive extracted correctly.")
    endif ()

    if (NOT _cef_debug_bundle)
        set(_cef_debug_bundle "${_cef_release_bundle}")
    endif ()

    set(_cef_release_framework "${_cef_release_bundle}/Chromium Embedded Framework")
    set(_cef_debug_framework "${_cef_debug_bundle}/Chromium Embedded Framework")

    set(_cef_resource_candidates "${_cef_release_bundle}/Resources")
    list(APPEND _cef_resource_candidates
            "${_cef_probe_root}/Resources"
            "${_cef_probe_root}/Release/Resources")

    get_filename_component(_cef_probe_root_parent "${_cef_probe_root}" DIRECTORY)
    if (NOT _cef_probe_root_parent STREQUAL "${_cef_probe_root}")
        list(APPEND _cef_resource_candidates
                "${_cef_probe_root_parent}/Resources"
                "${_cef_probe_root_parent}/Release/Resources")
    endif ()

    foreach(_candidate IN LISTS _cef_resource_candidates)
        if (EXISTS "${_candidate}")
            set(_cef_release_resources "${_candidate}")
            break()
        endif ()
    endforeach ()

    set(_cef_debug_resource_candidates "")
    if (DEFINED _cef_debug_bundle AND _cef_debug_bundle)
        list(APPEND _cef_debug_resource_candidates "${_cef_debug_bundle}/Resources")
    endif ()
    list(APPEND _cef_debug_resource_candidates "${_cef_probe_root}/Debug/Resources")
    if (NOT _cef_probe_root_parent STREQUAL "${_cef_probe_root}")
        list(APPEND _cef_debug_resource_candidates "${_cef_probe_root_parent}/Debug/Resources")
    endif ()

    foreach(_candidate IN LISTS _cef_debug_resource_candidates)
        if (EXISTS "${_candidate}")
            set(_cef_debug_resources "${_candidate}")
            break()
        endif ()
    endforeach ()

    if (NOT _cef_release_resources)
        message(FATAL_ERROR
                "CEF Resources directory not found in ${_cef_probe_root}. "
                "Verify that the CEF archive extracted correctly.")
    endif ()

    if (NOT _cef_debug_resources)
        set(_cef_debug_resources "${_cef_release_resources}")
    endif ()

    set(_cef_release_libraries_bundle "")
    set(_cef_debug_libraries_bundle "")
    if (EXISTS "${_cef_release_bundle}/Libraries")
        set(_cef_release_libraries_bundle "${_cef_release_bundle}")
        if (EXISTS "${_cef_debug_bundle}/Libraries")
            set(_cef_debug_libraries_bundle "${_cef_debug_bundle}")
        else ()
            set(_cef_debug_libraries_bundle "${_cef_release_bundle}")
        endif ()
    endif ()

    # Locate the helper app bundles shipped with CEF so they can be staged next
    # to the executable and renamed to match our target name.  Recent CEF
    # releases (starting with 140) no longer ship prebuilt helpers in the
    # binary distribution, so be prepared to run without them.
    set(_cef_release_helper_pairs "")
    set(_cef_debug_helper_pairs "")
    set(_cef_helper_destinations "")
    set(_cef_has_helpers TRUE)

    set(_cef_helper_search_roots
            "${_cef_probe_root}"
            "${_cef_probe_root}/Release"
            "${_cef_probe_root}/Debug")
    if (NOT _cef_probe_root_parent STREQUAL "${_cef_probe_root}")
        list(APPEND _cef_helper_search_roots
                "${_cef_probe_root_parent}"
                "${_cef_probe_root_parent}/Release"
                "${_cef_probe_root_parent}/Debug")
    endif ()

    foreach(_root IN LISTS _cef_helper_search_roots)
        file(GLOB_RECURSE _helpers FOLLOW_SYMLINKS "${_root}/* Helper*.app")
        foreach(_helper IN LISTS _helpers)
            if (IS_DIRECTORY "${_helper}")
                get_filename_component(_helper_name "${_helper}" NAME)
                string(REGEX REPLACE "^[^ ]+" "${PROJECT_NAME}" _dest_name "${_helper_name}")
                if (_helper MATCHES "/Debug/")
                    list(APPEND _cef_debug_helper_pairs "${_helper}|${_dest_name}")
                else ()
                    list(APPEND _cef_release_helper_pairs "${_helper}|${_dest_name}")
                endif ()
            endif ()
        endforeach ()
    endforeach ()

    if (NOT _cef_release_helper_pairs)
        set(_cef_has_helpers FALSE)
        message(WARNING
                "CEF helper applications were not found in ${_cef_probe_root}. "
                "macOS helper staging will be skipped. "
                "Consider building helper bundles from the CEF samples if your "
                "application requires them.")
    else ()
        if (NOT _cef_debug_helper_pairs)
            set(_cef_debug_helper_pairs "${_cef_release_helper_pairs}")
        endif ()

        list(REMOVE_DUPLICATES _cef_release_helper_pairs)
        list(REMOVE_DUPLICATES _cef_debug_helper_pairs)

        foreach(_pair IN LISTS _cef_release_helper_pairs _cef_debug_helper_pairs)
            if (_pair)
                string(REPLACE "|" ";" _parts "${_pair}")
                list(LENGTH _parts _len)
                if (_len GREATER 1)
                    list(GET _parts 1 _dest_name)
                    list(APPEND _cef_helper_destinations "${_dest_name}")
                endif ()
            endif ()
        endforeach ()
        list(REMOVE_DUPLICATES _cef_helper_destinations)

        if (NOT _cef_helper_destinations)
            set(_cef_has_helpers FALSE)
            message(WARNING "Failed to determine CEF helper applications to stage. "
                    "macOS helper staging will be skipped.")
        endif ()
    endif ()

    # The CEF helpers populate CEF_LIB_{DEBUG,RELEASE} as cache entries.  Force
    # both configs to use the discovered framework path so the logical target
    # resolves to an existing binary regardless of the selected build
    # configuration.
    set(CEF_LIB_DEBUG "${_cef_debug_framework}" CACHE FILEPATH "" FORCE)
    set(CEF_LIB_RELEASE "${_cef_release_framework}" CACHE FILEPATH "" FORCE)
endif ()

# Logical target used to link the libcef library
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

if (APPLE)
    if (NOT _cef_has_helpers)
        target_compile_definitions(webbrowser PRIVATE CEF_NO_HELPERS=1)
    endif ()
endif ()

add_dependencies(webbrowser libcef_dll_wrapper)
target_link_libraries(webbrowser libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS} revyv ${_CAIRO_TARGET})

if (_CAIRO_INCLUDE_HINTS)
    target_include_directories(webbrowser PRIVATE ${_CAIRO_INCLUDE_HINTS})
endif ()

# Stage the shared librevyv runtime next to the executable so the
# @loader_path rpath added above can resolve the dependency without
# requiring users to tweak DYLD_LIBRARY_PATH manually.
add_custom_command(TARGET webbrowser POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:revyv>"
                "$<TARGET_FILE_DIR:webbrowser>")

# Copy binary and resource files to the target output directory
COPY_FILES("webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
COPY_FILES("webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

if (APPLE)
    add_custom_command(TARGET webbrowser POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:webbrowser>/../Frameworks"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:webbrowser>/../Frameworks/Chromium Embedded Framework.framework"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "$<IF:$<CONFIG:Debug>,${_cef_debug_bundle},${_cef_release_bundle}>"
                    "$<TARGET_FILE_DIR:webbrowser>/../Frameworks/Chromium Embedded Framework.framework"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:webbrowser>/Resources"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "$<IF:$<CONFIG:Debug>,${_cef_debug_resources},${_cef_release_resources}>"
                    "$<TARGET_FILE_DIR:webbrowser>/Resources")

    if (_cef_release_libraries_bundle)
        set(_cef_library_source "$<IF:$<CONFIG:Debug>,${_cef_debug_libraries_bundle},${_cef_release_libraries_bundle}>/Libraries")
        add_custom_command(TARGET webbrowser POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${_cef_library_source}"
                        "$<TARGET_FILE_DIR:webbrowser>")
    endif ()

    if (_cef_has_helpers)
        foreach(_dest_name IN LISTS _cef_helper_destinations)
            _cef_get_helper_src("${_dest_name}" _cef_release_helper_pairs _cef_release_helper_src)
            _cef_get_helper_src("${_dest_name}" _cef_debug_helper_pairs _cef_debug_helper_src)

            if (NOT _cef_release_helper_src)
                message(FATAL_ERROR "No release helper found matching ${_dest_name}.")
            endif ()

            if (NOT _cef_debug_helper_src)
                set(_cef_debug_helper_src "${_cef_release_helper_src}")
            endif ()

            get_filename_component(_cef_release_helper_we "${_cef_release_helper_src}" NAME_WE)
            get_filename_component(_cef_dest_helper_we "${_dest_name}" NAME_WE)

            add_custom_command(TARGET webbrowser POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:webbrowser>/../Frameworks/${_dest_name}"
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                            "$<IF:$<CONFIG:Debug>,${_cef_debug_helper_src},${_cef_release_helper_src}>"
                            "$<TARGET_FILE_DIR:webbrowser>/../Frameworks/${_dest_name}>"
                    COMMAND ${CMAKE_COMMAND} -E rename
                            "$<TARGET_FILE_DIR:webbrowser>/../Frameworks/${_dest_name}/Contents/MacOS/${_cef_release_helper_we}"
                            "$<TARGET_FILE_DIR:webbrowser>/../Frameworks/${_dest_name}/Contents/MacOS/${_cef_dest_helper_we}"
                    COMMAND /usr/bin/env bash -c "if [ -x /usr/libexec/PlistBuddy ]; then /usr/libexec/PlistBuddy -c \"Set :CFBundleExecutable ${_cef_dest_helper_we}\" \"$<TARGET_FILE_DIR:webbrowser>/../Frameworks/${_dest_name}/Contents/Info.plist\" && /usr/libexec/PlistBuddy -c \"Set :CFBundleName ${_cef_dest_helper_we}\" \"$<TARGET_FILE_DIR:webbrowser>/../Frameworks/${_dest_name}/Contents/Info.plist\"; fi")
        endforeach ()
    endif ()
endif ()

if (NOT APPLE)
    # Output a message about setting SUID permissions on the chrome-sandbox target
    SET_LINUX_SUID_PERMISSIONS("webbrowser" "${CMAKE_CURRENT_BINARY_DIR}/chrome-sandbox")
endif ()

# Display configuration settings.
PRINT_CEF_CONFIG()
