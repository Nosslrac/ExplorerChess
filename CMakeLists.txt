# Copyright (C) 2024 OpenDLV

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_COMPILER "clang++")

project(ExplorerChess)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} \   ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
    -g -fsanitize=thread")
    # -fsanitize=address 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -march=native -Ofast -DSHALLOW_SEARCH -DPRINT_OUT \
    -D_XOPEN_SOURCE=700 -fstack-protector \
    -fomit-frame-pointer -pipe -pedantic -pedantic-errors -Werror \
    -Wall -Wextra -Wshadow -Wdeprecated -Wdiv-by-zero -Wfloat-equal \
    -Wfloat-conversion -Wsign-compare -Wpointer-arith -Wuninitialized \
    -Wunreachable-code -Wunused -Wunused-function -Wunused-label \
    -Wunused-parameter -Wunused-but-set-parameter -Wunused-but-set-variable \
    -Wunused-value -Wunused-variable -Wunused-result \
    -Wmissing-field-initializers -Wmissing-format-attribute \
    -Wmissing-include-dirs")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")


include_directories(SYSTEM ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

file(GLOB SOURCES "src/*.cpp")


add_executable(${PROJECT_NAME} ${SOURCES})


