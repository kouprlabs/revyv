project(compositor)

set(_SDL2_TARGET "")
set(_SDL2_INCLUDE_HINTS "")
set(_ZEROMQ_TARGET "")
set(_ZEROMQ_INCLUDE_HINTS "")
set(_CPPZMQ_TARGET "")
set(_CPPZMQ_INCLUDE_HINTS "")
set(_LZO_TARGET "")
set(_LZO_INCLUDE_HINTS "")

if(APPLE)
    set(_brew_prefixes "/opt/homebrew" "/usr/local")

    foreach(_prefix IN LISTS _brew_prefixes)
        if(NOT _zeromq_root AND EXISTS "${_prefix}/opt/zeromq")
            set(_zeromq_root "${_prefix}/opt/zeromq")
        endif()
        if(NOT _cppzmq_root AND EXISTS "${_prefix}/opt/cppzmq")
            set(_cppzmq_root "${_prefix}/opt/cppzmq")
        endif()
        if(NOT _sdl_root AND EXISTS "${_prefix}/opt/sdl2")
            set(_sdl_root "${_prefix}/opt/sdl2")
        endif()
        if(NOT _lzo_root AND EXISTS "${_prefix}/opt/lzo")
            set(_lzo_root "${_prefix}/opt/lzo")
        endif()
    endforeach()

    if(_zeromq_root AND EXISTS "${_zeromq_root}/lib/libzmq.dylib")
        set(_ZEROMQ_TARGET "${_zeromq_root}/lib/libzmq.dylib")
        if(EXISTS "${_zeromq_root}/include")
            list(APPEND _ZEROMQ_INCLUDE_HINTS "${_zeromq_root}/include")
        endif()
    endif()

    if(_cppzmq_root AND EXISTS "${_cppzmq_root}/include")
        list(APPEND _CPPZMQ_INCLUDE_HINTS "${_cppzmq_root}/include")
    endif()

    if(_sdl_root)
        if(EXISTS "${_sdl_root}/lib/libSDL2.dylib")
            set(_SDL2_TARGET "${_sdl_root}/lib/libSDL2.dylib")
        endif()
        if(EXISTS "${_sdl_root}/include")
            list(APPEND _SDL2_INCLUDE_HINTS "${_sdl_root}/include")
        endif()
    endif()

    if(_lzo_root AND EXISTS "${_lzo_root}/lib/liblzo2.dylib")
        set(_LZO_TARGET "${_lzo_root}/lib/liblzo2.dylib")
        if(EXISTS "${_lzo_root}/include")
            list(APPEND _LZO_INCLUDE_HINTS "${_lzo_root}/include")
        endif()
    endif()

    if(NOT _ZEROMQ_TARGET)
        message(FATAL_ERROR "ZeroMQ not found in Homebrew. Install it with 'brew install zeromq'.")
    endif()
    if(NOT _CPPZMQ_INCLUDE_HINTS)
        message(FATAL_ERROR "cppzmq headers not found in Homebrew. Install them with 'brew install cppzmq'.")
    endif()
    if(NOT _SDL2_TARGET)
        message(FATAL_ERROR "SDL2 not found in Homebrew. Install it with 'brew install sdl2'.")
    endif()
    if(NOT _LZO_TARGET)
        message(FATAL_ERROR "LZO not found in Homebrew. Install it with 'brew install lzo'.")
    endif()
else()
    find_package(ZeroMQ CONFIG REQUIRED)
    if(TARGET ZeroMQ::libzmq)
        set(_ZEROMQ_TARGET ZeroMQ::libzmq)
    elseif(TARGET ZeroMQ::ZeroMQ)
        set(_ZEROMQ_TARGET ZeroMQ::ZeroMQ)
    else()
        message(FATAL_ERROR "ZeroMQ config did not define an imported target")
    endif()

    find_package(cppzmq CONFIG REQUIRED)
    if(TARGET cppzmq::cppzmq)
        set(_CPPZMQ_TARGET cppzmq::cppzmq)
    else()
        message(FATAL_ERROR "cppzmq config did not define the cppzmq::cppzmq target")
    endif()

    find_package(SDL2 CONFIG QUIET)
    if(NOT SDL2_FOUND)
        find_package(SDL2 REQUIRED)
    endif()

    if(TARGET SDL2::SDL2)
        set(_SDL2_TARGET SDL2::SDL2)
    elseif(SDL2_FOUND)
        set(_SDL2_TARGET ${SDL2_LIBRARIES})
        list(APPEND _SDL2_INCLUDE_HINTS ${SDL2_INCLUDE_DIRS})
    endif()

    find_package(LZO REQUIRED)
    if(TARGET LZO::lzo2)
        set(_LZO_TARGET LZO::lzo2)
    elseif(DEFINED LZO_LIBRARIES)
        set(_LZO_TARGET ${LZO_LIBRARIES})
        if(DEFINED LZO_INCLUDE_DIRS)
            list(APPEND _LZO_INCLUDE_HINTS ${LZO_INCLUDE_DIRS})
        elseif(DEFINED LZO_INCLUDE_DIR)
            list(APPEND _LZO_INCLUDE_HINTS ${LZO_INCLUDE_DIR})
        endif()
    endif()
endif()

if(NOT _SDL2_TARGET)
    message(FATAL_ERROR "SDL2 not found. Install SDL2 or provide the library path explicitly.")
endif()

if(NOT _LZO_TARGET)
    message(FATAL_ERROR "LZO not found. Install LZO or provide the library path explicitly.")
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

if(_SDL2_INCLUDE_HINTS)
    target_include_directories(compositor PRIVATE ${_SDL2_INCLUDE_HINTS})
endif()

if(_ZEROMQ_INCLUDE_HINTS)
    target_include_directories(compositor PRIVATE ${_ZEROMQ_INCLUDE_HINTS})
endif()

if(_CPPZMQ_INCLUDE_HINTS)
    target_include_directories(compositor PRIVATE ${_CPPZMQ_INCLUDE_HINTS})
endif()

if(_LZO_INCLUDE_HINTS)
    target_include_directories(compositor PRIVATE ${_LZO_INCLUDE_HINTS})
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
