project(compositor)

# Ensure ZeroMQ and cppzmq are available so the compositor target inherits the
# correct include directories and link libraries on all platforms.
include("${CMAKE_SOURCE_DIR}/cmake/RevyvZeroMQ.cmake")
revyv_require_zmq()

# Discover SDL2 so platform-specific include directories and link flags are
# propagated to the compositor target. Prefer the modern CMake package on
# systems such as Homebrew and fall back to pkg-config if only a .pc file is
# available (e.g. some Linux distributions).
find_package(SDL2 CONFIG QUIET)
if(NOT TARGET SDL2::SDL2)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 QUIET sdl2 IMPORTED_TARGET)
    endif()

    if(SDL2_FOUND AND NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${SDL2_CFLAGS_OTHER}"
        )
    elseif(NOT TARGET SDL2::SDL2)
        message(FATAL_ERROR "SDL2 not found. Install SDL2 or provide SDL2_DIR.")
    endif()
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
        SDL2::SDL2
        ZeroMQ::libzmq
        cppzmq::cppzmq
        pthread
        lzo2
)
set_property(TARGET compositor PROPERTY CXX_STANDARD 17)
