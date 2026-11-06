# FreeBSD Control Center (FCC)

Native FreeBSD desktop control center: compact system controls plus a separate notification center.

**Status:** P0 platform validation. Read-only backlight/mixer discovery, a Qt tray-launched transient control panel, and a guarded mixer master-volume write/readback/restore backend are implemented. The UI does not yet expose device write controls. Brightness writes, mute, full notifications, theme persistence, GPU selection, autostart and release packaging remain incomplete. Not production-ready.

Baseline: FreeBSD 15.1-RELEASE, XFCE/Xorg, C++20, Qt 6 Widgets, CMake, MIT. Qt Widgets/QPainter Raster is initial baseline. Qt Quick deferred. NVIDIA only after repeatable stability tests; Intel next; CPU Raster fallback. No CUDA/NVDEC dependency or driver reconfiguration.

Layout: src/app (entry), src/core (state/contracts), src/gui (surfaces), src/services (brightness/audio/notification), src/desktop (tray/D-Bus/X11/session), src/platform/freebsd (OS adapters), tests, docs.

Build as normal desktop user in active XFCE/Xorg:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure
    ./build/fcc
    ./build/fcc-platform-probe

Install for the current user (no root privileges and no autostart):
    cmake --install build --prefix "$HOME/.local"
    "$HOME/.local/bin/fcc"

See `docs/INSTALL_FREEBSD.md` for verification and safe uninstall instructions.

The P0 app reads /dev/backlight and /dev/mixerN asynchronously and displays discovered state. The tray opens a transient popup; the popup has refresh, notification-center and quit actions. Device operations remain read-only. See docs/PLATFORM_BACKENDS.md, docs/TEST_PLAN.md and docs/SECURITY.md.

Every source file documents responsibility, dependencies, collaboration and side effects. Inspect call paths before coding; test error/permission paths; report only executed checks.
