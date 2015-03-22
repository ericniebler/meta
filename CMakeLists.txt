cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(Meta CXX)

enable_testing()

include_directories(include)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-unused-function -ftemplate-backtrace-limit=0 -Wdocumentation")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-inline -g3 -fstack-protector-all")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -g0 -march=native -mtune=native -DNDEBUG")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-unused-function")
endif()

if(BIICODE)
    ADD_BIICODE_TARGETS()

    string(REPLACE " " ";" TARGET_CXX_FLAGS         ${CMAKE_CXX_FLAGS})
    string(REPLACE " " ";" TARGET_CXX_FLAGS_DEBUG   ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE " " ";" TARGET_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})     

    target_compile_options(${BII_LIB_TARGET} INTERFACE ${TARGET_CXX_FLAGS}
                           $<$<CONFIG:Debug>:${TARGET_CXX_FLAGS_DEBUG}>
                           $<$<CONFIG:Release>:${TARGET_CXX_FLAGS_RELEASE}>)

    return()
endif()

add_subdirectory(doc)
add_subdirectory(example)
add_subdirectory(test)

if(EXISTS "${CMAKE_SOURCE_DIR}/scratch")
    add_subdirectory(scratch)
endif()
