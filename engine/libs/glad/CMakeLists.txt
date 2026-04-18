cmake_minimum_required(VERSION 3.21)

add_library(glad glad.cpp)

target_include_directories(glad PUBLIC include/)