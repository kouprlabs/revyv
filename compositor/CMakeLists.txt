project(compositor)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
        "/opt/homebrew/opt/sdl2"
        "/opt/homebrew/opt/zeromq"
        "/opt/homebrew/opt/cppzmq"
    )
endif()

find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)
find_package(SDL2 REQUIRED)

set(_ZEROMQ_TARGET ZeroMQ::libzmq)
if(NOT TARGET ${_ZEROMQ_TARGET})
    set(_ZEROMQ_TARGET ZeroMQ::ZeroMQ)
endif()
if(NOT TARGET ${_ZEROMQ_TARGET})
    message(FATAL_ERROR "ZeroMQ imported target not found. Install a ZeroMQ package that provides CMake targets.")
endif()

set(_CPPZMQ_TARGET cppzmq::cppzmq)
if(NOT TARGET ${_CPPZMQ_TARGET})
    message(FATAL_ERROR "cppzmq imported target not found. Install a cppzmq package that provides CMake targets.")
endif()

set(_SDL2_TARGET SDL2::SDL2)
if(NOT TARGET ${_SDL2_TARGET})
    message(FATAL_ERROR "SDL2 imported target not found. Install SDL2 or provide SDL2_DIR.")
endif()

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

target_include_directories(compositor
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(compositor
    PRIVATE
        ${_SDL2_TARGET}
        ${_ZEROMQ_TARGET}
        ${_CPPZMQ_TARGET}
        pthread
        lzo2
)
set_property(TARGET compositor PROPERTY CXX_STANDARD 17)
