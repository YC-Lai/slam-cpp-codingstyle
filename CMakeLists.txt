cmake_minimum_required(VERSION 3.0.0)
project(slam_cpp_codingstyle VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(slam_cpp_codingstyle main.cpp)
add_executable(Item8 src/Item8.cc)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
