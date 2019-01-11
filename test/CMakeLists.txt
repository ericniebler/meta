add_executable(test.meta meta.cpp)
target_link_libraries(test.meta meta)
add_test(test.meta test.meta)
