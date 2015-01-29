cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(Meta CXX)

enable_testing()

include_directories(include)

add_subdirectory(doc)
add_subdirectory(test)
