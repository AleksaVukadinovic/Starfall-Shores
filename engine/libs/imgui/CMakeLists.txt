cmake_minimum_required(VERSION 3.21)

file(GLOB IMGUI_SOURCES "src/*.cpp")

add_library(imgui ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC include/)
target_link_libraries(imgui PRIVATE glad glfw)
target_compile_definitions(imgui PUBLIC -DIMGUI_IMPL_OPENGL_LOADER_GLAD)