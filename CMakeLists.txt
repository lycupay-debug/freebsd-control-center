cmake_minimum_required(VERSION 3.21)

project(FreeBSDControlCenter VERSION 0.1.0 LANGUAGES CXX)
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Concurrent)

add_library(fcc_platform
  src/platform/freebsd/brightness_backend.cpp
  src/platform/freebsd/audio_backend.cpp
)
target_include_directories(fcc_platform PUBLIC include)
target_compile_options(fcc_platform PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

add_library(fcc_gui
  include/fcc/gui/control_center/control_panel.h
  src/gui/control_center/control_panel.cpp
)
target_include_directories(fcc_gui PUBLIC include)
target_link_libraries(fcc_gui PUBLIC Qt6::Widgets)
target_compile_options(fcc_gui PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

add_executable(fcc-platform-probe src/platform_probe/main.cpp)
target_link_libraries(fcc-platform-probe PRIVATE fcc_platform)
target_compile_options(fcc-platform-probe PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

add_executable(fcc src/app/main.cpp)
target_link_libraries(fcc PRIVATE fcc_platform fcc_gui Qt6::Widgets Qt6::Concurrent)
target_compile_options(fcc PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

include(CTest)
if(BUILD_TESTING)
  find_package(Qt6 6.4 REQUIRED COMPONENTS Test)
  add_executable(fcc_platform_tests tests/unit/test_platform_values.cpp)
  target_link_libraries(fcc_platform_tests PRIVATE fcc_platform Qt6::Test)
  add_test(NAME fcc_platform_values COMMAND fcc_platform_tests)

  add_executable(fcc_control_panel_tests tests/unit/test_control_panel.cpp)
  target_link_libraries(fcc_control_panel_tests PRIVATE fcc_gui Qt6::Test)
  add_test(NAME fcc_control_panel COMMAND fcc_control_panel_tests)
  set_tests_properties(fcc_control_panel PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

install(TARGETS fcc fcc-platform-probe
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES packaging/freebsd/freebsd-control-center.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES README.md LICENSE
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
install(DIRECTORY docs/
  DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs
)
