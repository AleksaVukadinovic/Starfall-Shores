cmake_minimum_required(VERSION 3.21)

add_library(miniaudio miniaudio.cpp)

target_include_directories(miniaudio PUBLIC include/)
