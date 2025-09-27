project(compositor)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
        "/opt/homebrew/opt/sdl2"
        "/opt/homebrew/opt/zeromq"
        "/opt/homebrew/opt/cppzmq"
        "/opt/homebrew/opt/lzo"
    )
endif()

find_package(ZeroMQ REQUIRED)

if(TARGET ZeroMQ::libzmq)
    set(_ZEROMQ_TARGET ZeroMQ::libzmq)
elseif(TARGET ZeroMQ::ZeroMQ)
    set(_ZEROMQ_TARGET ZeroMQ::ZeroMQ)
else()
    message(FATAL_ERROR "ZeroMQ package did not define an imported target.")
endif()

find_package(cppzmq CONFIG REQUIRED)
set(_CPPZMQ_TARGET cppzmq::cppzmq)

find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

find_package(LZO REQUIRED)

set(_SDL2_TARGET "")
set(_SDL2_NEEDS_INCLUDES OFF)
if(TARGET SDL2::SDL2)
    set(_SDL2_TARGET SDL2::SDL2)
elseif(SDL2_FOUND)
    set(_SDL2_TARGET ${SDL2_LIBRARIES})
    set(_SDL2_NEEDS_INCLUDES ON)
endif()

if(NOT _SDL2_TARGET)
    message(FATAL_ERROR "SDL2 not found. Install SDL2 or provide SDL2_DIR.")
endif()

set(_LZO_TARGET "")
set(_LZO_NEEDS_INCLUDES OFF)
if(TARGET LZO::lzo2)
    set(_LZO_TARGET LZO::lzo2)
elseif(DEFINED LZO_LIBRARIES)
    set(_LZO_TARGET ${LZO_LIBRARIES})
    set(_LZO_NEEDS_INCLUDES ON)
endif()

if(NOT _LZO_TARGET)
    message(FATAL_ERROR "LZO not found. Install LZO or provide LZO_DIR.")
endif()

set(_LZO_INCLUDE_PATH "")
if(DEFINED LZO_INCLUDE_DIRS)
    set(_LZO_INCLUDE_PATH ${LZO_INCLUDE_DIRS})
elseif(DEFINED LZO_INCLUDE_DIR)
    set(_LZO_INCLUDE_PATH ${LZO_INCLUDE_DIR})
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

if(_SDL2_NEEDS_INCLUDES)
    target_include_directories(compositor PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

if(_LZO_NEEDS_INCLUDES AND _LZO_INCLUDE_PATH)
    target_include_directories(compositor PRIVATE ${_LZO_INCLUDE_PATH})
endif()

target_link_libraries(compositor
    PRIVATE
        ${_SDL2_TARGET}
        ${_ZEROMQ_TARGET}
        ${_CPPZMQ_TARGET}
        pthread
        ${_LZO_TARGET}
)
set_property(TARGET compositor PROPERTY CXX_STANDARD 17)
