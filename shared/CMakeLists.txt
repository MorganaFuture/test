cmake_minimum_required(VERSION 3.5)

project(MyProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(main main.cpp shared.cpp shared.hpp)
