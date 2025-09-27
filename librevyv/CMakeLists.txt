project(asl)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
        "/opt/homebrew/opt/zeromq"
        "/opt/homebrew/opt/cppzmq"
    )
endif()

find_package(ZeroMQ CONFIG QUIET)
if(NOT ZeroMQ_FOUND)
    find_package(ZeroMQ REQUIRED)
endif()

set(_ZEROMQ_TARGET "")
set(_ZEROMQ_NEEDS_INCLUDES OFF)
if(TARGET ZeroMQ::libzmq)
    set(_ZEROMQ_TARGET ZeroMQ::libzmq)
elseif(TARGET ZeroMQ::ZeroMQ)
    set(_ZEROMQ_TARGET ZeroMQ::ZeroMQ)
elseif(ZeroMQ_FOUND)
    set(_ZEROMQ_TARGET ${ZeroMQ_LIBRARIES})
    set(_ZEROMQ_NEEDS_INCLUDES ON)
endif()

if(NOT _ZEROMQ_TARGET)
    message(FATAL_ERROR "ZeroMQ not found. Install ZeroMQ or provide ZeroMQ_DIR.")
endif()

find_package(cppzmq CONFIG REQUIRED)
set(_CPPZMQ_TARGET cppzmq::cppzmq)

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

if(_ZEROMQ_NEEDS_INCLUDES)
    target_include_directories(revyv PUBLIC ${ZeroMQ_INCLUDE_DIRS})
endif()

set_property(TARGET revyv PROPERTY CXX_STANDARD 17)

add_subdirectory(test)
