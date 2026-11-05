# ADR-0001: Technology Baseline

Status: accepted as P0 baseline; revisitable with evidence. Date: 2026-11-05.

## Context
Native FreeBSD control center with isolated hardware access, maintainable service contracts and first acceptance on XFCE/Xorg.

## Decision
C++20; Qt 6 Widgets; QPainter/Raster for ordinary Widgets; CMake; FreeBSD interfaces behind platform adapters; unprivileged process. Validate tray in XFCE; QSystemTrayIcon does not guarantee identical tray support. Validate OpenGL contexts separately. NVIDIA preference conditional on repeatable stability; otherwise evaluate Intel; retain CPU Raster fallback.

## Alternatives
Qt Quick deferred because second UI/rendering stack adds cost before demonstrated need. GTK not selected because Qt Widgets is the confirmed baseline. Shell-only core rejected due to service contracts, async I/O, testability and error handling. Whole-GUI root rejected due to excessive privilege.

## Consequences
Qt plugins, fonts, DPI, tray and X11 behavior need target testing. GPU preference is not a guarantee. No CUDA/NVDEC or driver reconfiguration required.

## Revisit triggers
Widgets cannot meet interaction/rendering requirements; FreeBSD services unavailable/unstable; or another toolkit materially improves native integration at acceptable migration cost.
