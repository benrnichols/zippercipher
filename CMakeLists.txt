cmake_minimum_required(VERSION 2.8)
project(zippercipher)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -std=c11")
add_executable(zipper zipper.c)