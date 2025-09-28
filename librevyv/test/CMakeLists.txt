project(client-test)

add_executable(client-test main.cpp)

if(APPLE)
    set(_homebrew_prefix $ENV{HOMEBREW_PREFIX})
    if(NOT _homebrew_prefix)
        set(_homebrew_prefix "/opt/homebrew")
        if(NOT EXISTS "${_homebrew_prefix}/bin/brew")
            set(_homebrew_prefix "/usr/local")
        endif()
    endif()

    set(_homebrew_search_prefixes
            "${_homebrew_prefix}"
            "${_homebrew_prefix}/opt/cairo"
            "/usr/local"
            "/usr/local/opt/cairo")

    find_path(HOMEBREW_CAIRO_INCLUDE_DIR
            NAMES cairo/cairo.h
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_CAIRO_INCLUDE_DIR)
        message(FATAL_ERROR "Could not locate cairo headers. Install them with 'brew install cairo'.")
    endif()

    find_library(HOMEBREW_CAIRO_LIBRARY
            NAMES cairo
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_CAIRO_LIBRARY)
        message(FATAL_ERROR "Could not locate libcairo. Install it with 'brew install cairo'.")
    endif()

    target_include_directories(client-test PRIVATE "${HOMEBREW_CAIRO_INCLUDE_DIR}")
    target_link_libraries(client-test PRIVATE revyv "${HOMEBREW_CAIRO_LIBRARY}")
else()
    target_link_libraries(client-test PRIVATE revyv cairo)
endif()

set_property(TARGET client-test PROPERTY CXX_STANDARD 17)
