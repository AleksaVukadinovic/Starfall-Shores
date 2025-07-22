cmake_minimum_required(VERSION 3.21)
project(matf-rg-engine)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++23" COMPILER_SUPPORTS_CXX23)
if (NOT COMPILER_SUPPORTS_CXX23)
    message(FATAL_ERROR "The compiler does not support C++23.")
endif()

file(GLOB engine-sources src/*.cpp)
file(GLOB engine-headers include/engine/*.hpp)

################ Libs ################
add_subdirectory(libs/spdlog EXCLUDE_FROM_ALL)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE SPDLOG_DEBUG_ON SPDLOG_TRACE_ON RG_ENGINE_TRACE)

add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)
add_subdirectory(libs/glad EXCLUDE_FROM_ALL)
add_subdirectory(libs/json EXCLUDE_FROM_ALL)
add_subdirectory(libs/assimp EXCLUDE_FROM_ALL)
add_subdirectory(libs/stb EXCLUDE_FROM_ALL)
add_subdirectory(libs/imgui EXCLUDE_FROM_ALL)
add_subdirectory(libs/glm EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME} ${engine-sources} ${engine-headers})
target_include_directories(${PROJECT_NAME} PUBLIC include/)
target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw assimp ${ASSIMP_LIBRARIES} stb
        PUBLIC glm::glm-header-only spdlog::spdlog imgui json)

prebuild_check(${PROJECT_NAME})
