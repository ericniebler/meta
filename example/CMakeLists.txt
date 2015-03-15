add_executable(tutorial_snippets tutorial_snippets.cpp)
add_test(example.tutorial_snippets, tutorial_snippets)

add_executable(tutorial_snippets_cxx14 tutorial_snippets.cpp)
add_test(example.tutorial_snippets_cxx14, tutorial_snippets_cxx14)

set_target_properties(tutorial_snippets_cxx14 PROPERTIES COMPILE_FLAGS "-std=gnu++1y")

add_executable(tuple_cat tuple_cat.cpp)
add_test(example.tuple_cat, tuple_cat)
