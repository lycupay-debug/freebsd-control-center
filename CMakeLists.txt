cmake_minimum_required(VERSION 3.21)

# FCC build graph: platform backends are isolated from Qt and presentation.
project(FreeBSDControlCenter VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Concurrent Test)

add_library(fcc_platform
  src/platform/freebsd/brightness_backend.cpp
  src/platform/freebsd/audio_backend.cpp
)
target_include_directories(fcc_platform PUBLIC include)
target_compile_options(fcc_platform PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

add_executable(fcc-platform-probe src/platform_probe/main.cpp)
target_link_libraries(fcc-platform-probe PRIVATE fcc_platform)
target_compile_options(fcc-platform-probe PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

add_executable(fcc src/app/main.cpp)
target_link_libraries(fcc PRIVATE fcc_platform Qt6::Widgets Qt6::Concurrent)
target_compile_options(fcc PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

include(CTest)
if(BUILD_TESTING)
  add_executable(fcc_platform_tests tests/unit/test_platform_values.cpp)
  target_link_libraries(fcc_platform_tests PRIVATE fcc_platform Qt6::Test)
  add_test(NAME fcc_platform_values COMMAND fcc_platform_tests)
endif()
