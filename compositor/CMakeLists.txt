project(compositor)

include_directories("${PROJECT_SOURCE_DIR}/src")

set(SOURCES
        include/compositor/types.h
        src/compositor.h
        src/compositor.cpp
        src/server.h
        src/server.cpp
        src/window_manager.h
        src/window_manager.cpp
        src/sdl_compositor.h
        src/sdl_compositor.cpp
        src/sdl_event_source.h
        src/sdl_event_source.cpp
        src/sdl_window.h
        src/sdl_window.cpp
        src/window.h
        src/window.cpp
        src/event_source.h
        src/main.cpp
        src/publisher.cpp
        src/publisher.h
        src/listener.cpp
        src/listener.h
        src/error.h)
add_executable(compositor ${SOURCES})

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
            "${_homebrew_prefix}/opt/sdl2"
            "${_homebrew_prefix}/opt/zeromq"
            "${_homebrew_prefix}/opt/cppzmq"
            "${_homebrew_prefix}/opt/lzo"
            "/usr/local"
            "/usr/local/opt/sdl2"
            "/usr/local/opt/zeromq"
            "/usr/local/opt/cppzmq"
            "/usr/local/opt/lzo")

    find_path(HOMEBREW_CPPZMQ_INCLUDE_DIR
            NAMES zmq.hpp
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_CPPZMQ_INCLUDE_DIR)
        message(FATAL_ERROR "Could not locate zmq.hpp. Install it with 'brew install zeromq cppzmq'.")
    endif()

    find_path(HOMEBREW_ZMQ_INCLUDE_DIR
            NAMES zmq.h
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_ZMQ_INCLUDE_DIR)
        message(FATAL_ERROR "Could not locate zmq.h. Install it with 'brew install zeromq'.")
    endif()

    find_path(HOMEBREW_SDL2_INCLUDE_ROOT
            NAMES SDL2/SDL.h
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_SDL2_INCLUDE_ROOT)
        message(FATAL_ERROR "Could not locate SDL2 headers. Install them with 'brew install sdl2'.")
    endif()

    find_library(HOMEBREW_SDL2_LIBRARY
            NAMES SDL2
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_SDL2_LIBRARY)
        message(FATAL_ERROR "Could not locate libSDL2. Install it with 'brew install sdl2'.")
    endif()

    find_library(HOMEBREW_ZMQ_LIBRARY
            NAMES zmq
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_ZMQ_LIBRARY)
        message(FATAL_ERROR "Could not locate libzmq. Install it with 'brew install zeromq'.")
    endif()

    find_library(HOMEBREW_LZO_LIBRARY
            NAMES lzo2
            PATHS ${_homebrew_search_prefixes}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH)

    if(NOT HOMEBREW_LZO_LIBRARY)
        message(FATAL_ERROR "Could not locate liblzo2. Install it with 'brew install lzo'.")
    endif()

    target_include_directories(compositor PRIVATE
            "${HOMEBREW_CPPZMQ_INCLUDE_DIR}"
            "${HOMEBREW_SDL2_INCLUDE_ROOT}")
    if(HOMEBREW_ZMQ_INCLUDE_DIR AND NOT HOMEBREW_ZMQ_INCLUDE_DIR STREQUAL HOMEBREW_CPPZMQ_INCLUDE_DIR)
        target_include_directories(compositor PRIVATE "${HOMEBREW_ZMQ_INCLUDE_DIR}")
    endif()
    target_link_libraries(compositor PRIVATE
            "${HOMEBREW_ZMQ_LIBRARY}"
            "${HOMEBREW_SDL2_LIBRARY}"
            "${HOMEBREW_LZO_LIBRARY}"
            pthread)
else()
    target_link_libraries(compositor PRIVATE zmq SDL2 pthread lzo2)
endif()

set_property(TARGET compositor PROPERTY CXX_STANDARD 17)
