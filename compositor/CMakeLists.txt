project(compositor)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
        "/opt/homebrew/opt/sdl2"
        "/opt/homebrew/opt/zeromq"
        "/opt/homebrew/opt/cppzmq"
    )
endif()

find_package(ZeroMQ CONFIG)

set(_ZEROMQ_TARGET "")
if(ZeroMQ_FOUND)
    foreach(_candidate ZeroMQ::libzmq ZeroMQ::ZeroMQ)
        if(TARGET ${_candidate})
            set(_ZEROMQ_TARGET ${_candidate})
            break()
        endif()
    endforeach()
endif()

if(NOT _ZEROMQ_TARGET)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(ZeroMQ libzmq IMPORTED_TARGET GLOBAL)
        if(TARGET PkgConfig::ZeroMQ)
            set(_ZEROMQ_TARGET PkgConfig::ZeroMQ)
        endif()
    endif()
endif()

if(NOT _ZEROMQ_TARGET)
    message(FATAL_ERROR "ZeroMQ not found. Install ZeroMQ or provide ZeroMQ_DIR / pkg-config metadata.")
endif()

find_package(cppzmq CONFIG)
set(_CPPZMQ_TARGET "")
if(cppzmq_FOUND AND TARGET cppzmq::cppzmq)
    set(_CPPZMQ_TARGET cppzmq::cppzmq)
endif()

if(NOT _CPPZMQ_TARGET)
    if(NOT PkgConfig_FOUND)
        find_package(PkgConfig QUIET)
    endif()
    if(PkgConfig_FOUND)
        pkg_check_modules(cppzmq cppzmq IMPORTED_TARGET GLOBAL)
        if(TARGET PkgConfig::cppzmq)
            set(_CPPZMQ_TARGET PkgConfig::cppzmq)
        endif()
    endif()
endif()

if(NOT _CPPZMQ_TARGET)
    message(FATAL_ERROR "cppzmq not found. Install cppzmq or provide cppzmq_DIR / pkg-config metadata.")
endif()

find_package(SDL2 REQUIRED)

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
