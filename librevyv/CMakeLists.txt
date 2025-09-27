project(asl)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
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
