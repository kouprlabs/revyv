project(asl)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
        "/opt/homebrew/opt/zeromq"
        "/opt/homebrew/opt/cppzmq"
    )
endif()

find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)

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
        lzo2
)

set_property(TARGET revyv PROPERTY CXX_STANDARD 17)

add_subdirectory(test)
