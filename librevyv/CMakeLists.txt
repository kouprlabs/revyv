project(asl)

if(APPLE)
    set(_homebrew_root "/opt/homebrew")
    list(APPEND CMAKE_PREFIX_PATH
        "${_homebrew_root}"
        "${_homebrew_root}/opt/zeromq"
        "${_homebrew_root}/opt/cppzmq"
        "${_homebrew_root}/opt/lzo"
    )
    if(NOT ZeroMQ_DIR AND EXISTS "${_homebrew_root}/opt/zeromq/lib/cmake/ZeroMQ/ZeroMQConfig.cmake")
        set(ZeroMQ_DIR "${_homebrew_root}/opt/zeromq/lib/cmake/ZeroMQ")
    endif()
    if(NOT cppzmq_DIR AND EXISTS "${_homebrew_root}/opt/cppzmq/lib/cmake/cppzmq/cppzmqConfig.cmake")
        set(cppzmq_DIR "${_homebrew_root}/opt/cppzmq/lib/cmake/cppzmq")
    endif()
    if(NOT LZO_DIR AND EXISTS "${_homebrew_root}/opt/lzo/lib/cmake/LZO/LZOConfig.cmake")
        set(LZO_DIR "${_homebrew_root}/opt/lzo/lib/cmake/LZO")
    endif()
    if(NOT LZO_ROOT AND EXISTS "${_homebrew_root}/opt/lzo")
        set(LZO_ROOT "${_homebrew_root}/opt/lzo")
    endif()
endif()

if(APPLE AND NOT LZO_ROOT AND EXISTS "/usr/local/opt/lzo")
    set(LZO_ROOT "/usr/local/opt/lzo")
endif()

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

find_package(LZO REQUIRED)

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

target_include_directories(revyv
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(revyv
    PUBLIC
        ${_CPPZMQ_TARGET}
        ${_ZEROMQ_TARGET}
    PRIVATE
        ${_LZO_TARGET}
)

if(_LZO_NEEDS_INCLUDES AND _LZO_INCLUDE_PATH)
    target_include_directories(revyv PUBLIC ${_LZO_INCLUDE_PATH})
endif()

set_property(TARGET revyv PROPERTY CXX_STANDARD 17)

add_subdirectory(test)
