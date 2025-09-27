cmake_minimum_required(VERSION 3.16)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum supported macOS version")
endif()

project(revyv)

include_directories("${PROJECT_SOURCE_DIR}/librevyv/include")
include_directories("${PROJECT_SOURCE_DIR}/compositor/include")
include_directories("${PROJECT_SOURCE_DIR}/thirdparty/argh")

add_subdirectory(compositor)
add_subdirectory(librevyv)
add_subdirectory(webbrowser)
