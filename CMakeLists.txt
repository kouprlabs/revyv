cmake_minimum_required(VERSION 3.16)
project(revyv)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories("${PROJECT_SOURCE_DIR}/librevyv/include")
include_directories("${PROJECT_SOURCE_DIR}/compositor/include")
include_directories("${PROJECT_SOURCE_DIR}/thirdparty/argh")

add_subdirectory(compositor)
add_subdirectory(librevyv)
add_subdirectory(webbrowser)
