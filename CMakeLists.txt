cmake_minimum_required(VERSION 3.21)

# FCC build definition; Qt Widgets is the only dependency in this probe.
# src/app/main.cpp owns startup and the initial desktop capability check.
# Generates build files only in the selected binary directory.
project(FreeBSDControlCenter VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
add_executable(fcc src/app/main.cpp)
target_link_libraries(fcc PRIVATE Qt6::Widgets)
target_compile_options(fcc PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)
include(CTest)
