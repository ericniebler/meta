cmake_minimum_required(VERSION 3.6)
get_directory_property(is_subproject PARENT_DIRECTORY)

project(Meta CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Export compilation data-base

set(META_CXX_STD 11 CACHE STRING "C++ standard version.")

add_library(meta INTERFACE)
target_include_directories(meta INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>)
target_include_directories(meta SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)

include(CTest) # invokes enable_testing() and defines BUILD_TESTING variable, defaulting to ON

if("x${CMAKE_CXX_COMPILER_ID}" MATCHES "x.*Clang")
  if("x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    set(META_CXX_COMPILER_CLANGCL TRUE)
  else()
    set(META_CXX_COMPILER_CLANG TRUE)
  endif()
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(META_CXX_COMPILER_GCC TRUE)
elseif("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC")
  set(META_CXX_COMPILER_MSVC TRUE)
endif()

if(META_CXX_COMPILER_CLANGCL OR META_CXX_COMPILER_MSVC)
  # Clang-CL will blow up in the standard library if compiling with less than
  # C++14, and MSVC doesn't support less than C++14 at all.
  if(META_CXX_STD LESS 14)
    set(META_CXX_STD 14)
  endif()
  # MSVC is currently supported only in 17+ mode
  if(META_CXX_COMPILER_MSVC AND META_CXX_STD LESS 17)
    set(META_CXX_STD 17)
  endif()
  set(CMAKE_CXX_FLAGS "/std:c++${META_CXX_STD} /permissive- /WX ${CMAKE_CXX_FLAGS}")
elseif(META_CXX_COMPILER_CLANG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${META_CXX_STD} -ftemplate-backtrace-limit=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Werror -pedantic-errors -Wdocumentation")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-documentation-unknown-command -Wno-missing-prototypes")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-inline -g3 -fstack-protector-all")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -g0 -march=native -mtune=native -DNDEBUG")
elseif(META_CXX_COMPILER_GCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${META_CXX_STD} -ftemplate-backtrace-limit=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -pedantic-errors")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-inline -g3 -fstack-protector-all")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -g0 -march=native -mtune=native -DNDEBUG")
endif()

add_subdirectory(doc)
add_subdirectory(example)
add_subdirectory(test)

if(EXISTS "${CMAKE_SOURCE_DIR}/scratch")
    add_subdirectory(scratch)
endif()
