cmake_minimum_required(VERSION 3.11)

set(TEST_APP test-app)
file(GLOB sources src/*.cpp)
file(GLOB headers include/*.hpp)

include_directories(include/)
add_executable(${TEST_APP} ${sources} ${headers})
target_link_libraries(${TEST_APP} PRIVATE matf-rg-engine)
target_compile_features(${TEST_APP} PRIVATE cxx_std_20)
set_target_properties(${TEST_APP} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
prebuild_check(${TEST_APP})