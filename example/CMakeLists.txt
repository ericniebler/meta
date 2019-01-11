add_executable(tutorial_snippets tutorial_snippets.cpp)
target_link_libraries(tutorial_snippets meta)
add_test(example.tutorial_snippets tutorial_snippets)

add_executable(tuple_cat tuple_cat.cpp)
target_link_libraries(tuple_cat meta)
add_test(example.tuple_cat tuple_cat)
