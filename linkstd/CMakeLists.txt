cmake_minimum_required(VERSION 3.10)
project(linkstd LANGUAGES CXX)

add_library(linkstd INTERFACE)
target_include_directories(linkstd INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(linkstd_test src/main.cpp)
target_link_libraries(linkstd_test PRIVATE linkstd)
set_target_properties(linkstd_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
