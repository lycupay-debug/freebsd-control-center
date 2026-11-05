# Build and Deployment on FreeBSD

## Baseline
FreeBSD 15.1-RELEASE amd64, Xorg, XFCE. Other desktops not acceptance-tested.

## Dependencies
Clang/C++20, CMake >=3.21, Qt 6 Widgets (API baseline >=6.4). Optional: Ninja, clang-format, Qt Test. Verify package origins/versions. Target observation 2026-11-05: Qt 6.11.2 present; CMake 3.31.12 installed from official FreeBSD quarterly repository.

## Build as normal user
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure

## Run
Run inside active Xorg/XFCE session. SSH without DISPLAY cannot verify GUI behavior. Never run FCC as root.

## Installation
No system-wide install or autostart during P0. Validate from build tree. Later use explicit package/install manifest and uninstall procedure. Autostart opt-in and reversible.

## Diagnostics
Record OS/kernel/arch; Qt/CMake/compiler and package origins; DISPLAY/window manager/compositor; tray capability; brightness endpoints/permissions; mixer devices/controls; OpenGL renderer/context separately from Widgets raster; exact commands/conditions.

## Rollback
P0 artifacts stay under repository build/. Initial scaffold changes no kernel, driver, sysctl, mixer or global desktop config.
