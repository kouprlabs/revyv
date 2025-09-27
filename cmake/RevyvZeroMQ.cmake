include_guard(GLOBAL)

function(revyv_require_zmq)
    if(TARGET ZeroMQ::libzmq AND TARGET cppzmq::cppzmq)
        return()
    endif()

    set(_revyv_need_pkg_config FALSE)

    if(NOT TARGET ZeroMQ::libzmq)
        find_package(ZeroMQ CONFIG QUIET)
        if(TARGET ZeroMQ::libzmq)
            # already provided by the package configuration
        elseif(TARGET libzmq)
            add_library(ZeroMQ::libzmq INTERFACE IMPORTED)
            set_target_properties(ZeroMQ::libzmq PROPERTIES
                INTERFACE_LINK_LIBRARIES libzmq)
            if(DEFINED ZeroMQ_INCLUDE_DIRS AND ZeroMQ_INCLUDE_DIRS)
                set_target_properties(ZeroMQ::libzmq PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIRS}")
            endif()
            if(DEFINED ZeroMQ_CFLAGS_OTHER AND ZeroMQ_CFLAGS_OTHER)
                set_target_properties(ZeroMQ::libzmq PROPERTIES
                    INTERFACE_COMPILE_OPTIONS "${ZeroMQ_CFLAGS_OTHER}")
            endif()
        else()
            set(_revyv_need_pkg_config TRUE)
        endif()
    endif()

    if(NOT TARGET cppzmq::cppzmq)
        find_package(cppzmq CONFIG QUIET)
        if(TARGET cppzmq::cppzmq)
            # available directly
        elseif(TARGET cppzmq)
            add_library(cppzmq::cppzmq INTERFACE IMPORTED)
            set_target_properties(cppzmq::cppzmq PROPERTIES
                INTERFACE_LINK_LIBRARIES cppzmq)
            if(DEFINED cppzmq_INCLUDE_DIRS AND cppzmq_INCLUDE_DIRS)
                set_target_properties(cppzmq::cppzmq PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${cppzmq_INCLUDE_DIRS}")
            endif()
            if(DEFINED cppzmq_CFLAGS_OTHER AND cppzmq_CFLAGS_OTHER)
                set_target_properties(cppzmq::cppzmq PROPERTIES
                    INTERFACE_COMPILE_OPTIONS "${cppzmq_CFLAGS_OTHER}")
            endif()
        else()
            set(_revyv_need_pkg_config TRUE)
        endif()
    endif()

    if(_revyv_need_pkg_config)
        find_package(PkgConfig QUIET)
    endif()

    if(NOT TARGET ZeroMQ::libzmq)
        if(PkgConfig_FOUND)
            pkg_check_modules(PC_ZeroMQ QUIET libzmq)
            if(PC_ZeroMQ_FOUND)
                add_library(ZeroMQ::libzmq INTERFACE IMPORTED)
                if(PC_ZeroMQ_INCLUDE_DIRS)
                    set_target_properties(ZeroMQ::libzmq PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${PC_ZeroMQ_INCLUDE_DIRS}")
                endif()
                if(PC_ZeroMQ_LIBRARIES)
                    set_target_properties(ZeroMQ::libzmq PROPERTIES
                        INTERFACE_LINK_LIBRARIES "${PC_ZeroMQ_LIBRARIES}")
                endif()
                if(PC_ZeroMQ_CFLAGS_OTHER)
                    set_target_properties(ZeroMQ::libzmq PROPERTIES
                        INTERFACE_COMPILE_OPTIONS "${PC_ZeroMQ_CFLAGS_OTHER}")
                endif()
            endif()
        endif()
    endif()

    if(NOT TARGET cppzmq::cppzmq)
        if(PkgConfig_FOUND)
            pkg_check_modules(PC_cppzmq QUIET cppzmq)
            if(PC_cppzmq_FOUND)
                add_library(cppzmq::cppzmq INTERFACE IMPORTED)
                if(PC_cppzmq_INCLUDE_DIRS)
                    set_target_properties(cppzmq::cppzmq PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${PC_cppzmq_INCLUDE_DIRS}")
                endif()
                if(PC_cppzmq_LIBRARIES)
                    set_target_properties(cppzmq::cppzmq PROPERTIES
                        INTERFACE_LINK_LIBRARIES "${PC_cppzmq_LIBRARIES}")
                endif()
                if(PC_cppzmq_CFLAGS_OTHER)
                    set_target_properties(cppzmq::cppzmq PROPERTIES
                        INTERFACE_COMPILE_OPTIONS "${PC_cppzmq_CFLAGS_OTHER}")
                endif()
            endif()
        endif()
    endif()

    if(NOT TARGET ZeroMQ::libzmq)
        message(FATAL_ERROR "ZeroMQ not found. Install libzmq or provide ZeroMQ_DIR/PKG_CONFIG_PATH.")
    endif()
    if(NOT TARGET cppzmq::cppzmq)
        message(FATAL_ERROR "cppzmq not found. Install cppzmq or provide cppzmq_DIR/PKG_CONFIG_PATH.")
    endif()
endfunction()
