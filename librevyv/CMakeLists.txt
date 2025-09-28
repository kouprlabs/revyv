project(asl)

include_directories("${PROJECT_SOURCE_DIR}/include")
get_filename_component(LIBREVYV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
include_directories("${LIBREVYV_ROOT}/compositor/include")

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

target_link_libraries(revyv zmq lzo2)

set_property(TARGET revyv PROPERTY CXX_STANDARD 17)

add_subdirectory(test)
