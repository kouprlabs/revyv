project(client-test)

add_executable(client-test main.cpp)

target_link_libraries(client-test revyv cairo)

set_property(TARGET client-test PROPERTY CXX_STANDARD 17)
