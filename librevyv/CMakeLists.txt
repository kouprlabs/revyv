project(asl)

include_directories("${PROJECT_SOURCE_DIR}/include")
get_filename_component(LIBREVYV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
include_directories("${LIBREVYV_ROOT}/compositor/include")

set(SOURCES
        src/capi.cpp
        src/connector.cpp
        src/connector.h
        src/socket.h
        src/compressor.h
        )
set(HEADERS
        include/revyv/revyv.h
        include/revyv/keycodes.h
        include/revyv/scancodes.h)
add_library(revyv SHARED ${SOURCES} ${HEADERS})

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
            "${_homebrew_prefix}/opt/cppzmq"
            "${_homebrew_prefix}/opt/zeromq"
            "/usr/local"
            "/usr/local/opt/cppzmq"
            "/usr/local/opt/zeromq")

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

    target_include_directories(revyv PRIVATE "${HOMEBREW_CPPZMQ_INCLUDE_DIR}")
    if(HOMEBREW_ZMQ_INCLUDE_DIR AND NOT HOMEBREW_ZMQ_INCLUDE_DIR STREQUAL HOMEBREW_CPPZMQ_INCLUDE_DIR)
        target_include_directories(revyv PRIVATE "${HOMEBREW_ZMQ_INCLUDE_DIR}")
    endif()
    target_link_libraries(revyv "${HOMEBREW_ZMQ_LIBRARY}" "${HOMEBREW_LZO_LIBRARY}")
else()
    target_link_libraries(revyv zmq lzo2)
endif()

set_property(TARGET revyv PROPERTY CXX_STANDARD 17)

add_subdirectory(test)
