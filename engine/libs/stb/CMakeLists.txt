cmake_minimum_required(VERSION 3.21)

add_library(stb stb.cpp)

target_include_directories(stb PUBLIC include/)