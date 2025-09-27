# Simple ZeroMQ finder that works with standard installations and Homebrew.

find_path(ZeroMQ_INCLUDE_DIR
    NAMES zmq.h
    HINTS
        /opt/homebrew/opt/zeromq/include
        /opt/homebrew/include
)

find_library(ZeroMQ_LIBRARY
    NAMES zmq libzmq
    HINTS
        /opt/homebrew/opt/zeromq/lib
        /opt/homebrew/lib
)

set(ZeroMQ_INCLUDE_DIRS "${ZeroMQ_INCLUDE_DIR}")
set(ZeroMQ_LIBRARIES "${ZeroMQ_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQ
    REQUIRED_VARS ZeroMQ_LIBRARY ZeroMQ_INCLUDE_DIR
)

if(ZeroMQ_FOUND)
    if(NOT TARGET ZeroMQ::libzmq)
        add_library(ZeroMQ::libzmq UNKNOWN IMPORTED)
        set_target_properties(ZeroMQ::libzmq PROPERTIES
            IMPORTED_LOCATION "${ZeroMQ_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIR}"
        )
    endif()
    if(NOT TARGET ZeroMQ::ZeroMQ)
        add_library(ZeroMQ::ZeroMQ INTERFACE IMPORTED)
        target_link_libraries(ZeroMQ::ZeroMQ INTERFACE ZeroMQ::libzmq)
    endif()
endif()
