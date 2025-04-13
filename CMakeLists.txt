cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_COMPILER "clang++")

project(ExplorerChess)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)



set(RELEASE_FLAGS "-march=native -std=c++20 -O3 -DSHALLOW_SEARCH")
set(DEBUG_FLAGS "-g -fsanitize=thread")
set(BASE_FLAGS "-fomit-frame-pointer -pipe -pedantic -pedantic-errors -Werror \
    -Wall -Wextra -Wshadow -Wdeprecated -Wdiv-by-zero -Wfloat-equal \
    -Wfloat-conversion -Wsign-compare -Wpointer-arith -Wuninitialized \
    -Wunreachable-code -Wunused -Wunused-function -Wunused-label \
    -Wunused-parameter -Wunused-but-set-parameter -Wunused-but-set-variable \
    -Wunused-value -Wunused-variable -Wunused-result \
    -Wmissing-field-initializers -Wmissing-format-attribute \
    -Wmissing-include-dirs")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASE_FLAGS} ${RELEASE_FLAGS}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)


include_directories(SYSTEM ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

file(GLOB SOURCES "src/*.cpp")
file(GLOB TEST_SOURCES "test/*.cpp" "src/*.cpp")
list(REMOVE_ITEM TEST_SOURCES "/repos/ExplorerChess/src/ExplorerChess.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(ExplorerChessTest ${TEST_SOURCES})

# Set specific compile flags for the test and main binary
target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:RELEASE>:${RELEASE_FLAGS}> -DPRINT_OUT)

target_compile_options(ExplorerChessTest PUBLIC $<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>)

target_link_libraries(ExplorerChessTest PUBLIC gtest gtest_main)




