project(asl)

# Ensure ZeroMQ and cppzmq discovery mirrors the compositor target so the
# shared library exposes the correct usage requirements to dependants.
include("${CMAKE_SOURCE_DIR}/cmake/RevyvZeroMQ.cmake")
revyv_require_zmq()

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
        cppzmq::cppzmq
        ZeroMQ::libzmq
    PRIVATE
        lzo2
)

set_property(TARGET revyv PROPERTY CXX_STANDARD 17)

add_subdirectory(test)
