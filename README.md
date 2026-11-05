# FreeBSD Control Center (FCC)

Native FreeBSD desktop control center: compact system controls plus a separate notification center.

**Status:** P0 platform validation with initial read-only backlight and mixer adapters. Not production-ready. Brightness/audio writes, mute behavior, popup interaction, notifications, GPU selection, autostart and packaging remain unverified.

Baseline: FreeBSD 15.1-RELEASE, XFCE/Xorg, C++20, Qt 6 Widgets, CMake, MIT. Qt Widgets/QPainter Raster is initial baseline. Qt Quick deferred. NVIDIA only after repeatable stability tests; Intel next; CPU Raster fallback. No CUDA/NVDEC dependency or driver reconfiguration.

Layout: src/app (entry), src/core (state/contracts), src/gui (surfaces), src/services (brightness/audio/notification), src/desktop (tray/D-Bus/X11/session), src/platform/freebsd (OS adapters), tests, docs.

Build as normal desktop user in active XFCE/Xorg:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure
    ./build/fcc
    ./build/fcc-platform-probe

The P0 app reads /dev/backlight and /dev/mixerN asynchronously and displays discovered state; it does not change brightness, volume or mute. See docs/PLATFORM_BACKENDS.md and the other project documents.

Every source file documents responsibility, dependencies, collaboration and side effects. Inspect call paths before coding; test error/permission paths; report only executed checks.
