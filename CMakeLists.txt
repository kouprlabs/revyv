cmake_minimum_required(VERSION 3.2.2)
project(revyv)

include_directories("${PROJECT_SOURCE_DIR}/librevyv/include")
include_directories("${PROJECT_SOURCE_DIR}/compositor/include")
include_directories("${PROJECT_SOURCE_DIR}/thirdparty/argh")

add_subdirectory(compositor)
add_subdirectory(librevyv)
add_subdirectory(webbrowser)
