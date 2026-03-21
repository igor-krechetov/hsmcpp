cmake_minimum_required(VERSION 3.16)
project(07_build_conan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(hsmcpp CONFIG REQUIRED)

add_executable(07_build_conan main.cpp)
target_link_libraries(07_build_conan PRIVATE hsmcpp::hsmcpp)
