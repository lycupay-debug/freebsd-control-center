# FreeBSD Control Center (FCC)

Native FreeBSD desktop control center: compact system controls plus a separate notification center.

**Status:** P0 platform validation; this scaffold is not production-ready. Hardware controls, tray/popup behavior, notification delivery, GPU selection, autostart and packaging remain unverified.

Baseline: FreeBSD 15.1-RELEASE, XFCE/Xorg, C++20, Qt 6 Widgets, CMake, MIT. Qt Widgets/QPainter Raster is initial baseline. Qt Quick deferred. NVIDIA only after repeatable stability tests; Intel next; CPU Raster fallback. No CUDA/NVDEC dependency or driver reconfiguration.

Layout: src/app (entry), src/core (state/contracts), src/gui (surfaces), src/services (brightness/audio/notification), src/desktop (tray/D-Bus/X11/session), src/platform/freebsd (OS adapters), tests, docs.

Build as normal desktop user in active XFCE/Xorg:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure
    ./build/fcc

Compile success does not prove tray, popup, hardware, GPU or deployment readiness. See docs/PRD.md, ARCHITECTURE.md, API.md, SECURITY.md, BUILD_FREEBSD.md, TEST_PLAN.md and ADR/0001-technology-baseline.md.

Every source file documents responsibility, dependencies, collaboration and side effects. Inspect call paths before coding; test error/permission paths; report only executed checks.
