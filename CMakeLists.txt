cmake_minimum_required(VERSION 3.21)
project(matf-rg-project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3)
include(cmake/functions.cmake)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++23" COMPILER_SUPPORTS_CXX23)
if (NOT COMPILER_SUPPORTS_CXX23)
    message(FATAL_ERROR "The compiler does not support C++23.")
endif()


############## ENGINE  ###############
add_subdirectory(engine EXCLUDE_FROM_ALL)

############# TEST ###############
option(BUILD_TEST_APP "Builds a test application" ON)
if (BUILD_TEST_APP)
    add_subdirectory(engine/test/app)
endif ()

############ APP #################
option(BUILD_APP "Builds the app" ON)
if (BUILD_APP)
    add_subdirectory(app/)
endif ()