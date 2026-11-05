# Architecture

Status: P0 read-only FreeBSD platform adapters implemented; service/API boundaries remain provisional.

## Principles
Separate presentation, application coordination, services, desktop integration and OS adapters. UI never accesses sysctl/device/mixer directly. Core does not depend on Qt Widgets. Hardware services do not depend on desktop environment. Blocking I/O stays off GUI thread. Missing devices are non-fatal. No silent fallback or false success. Avoid speculative abstractions.

## Layers
| Layer | Responsibility | Must not own |
|---|---|---|
| Presentation | Qt windows, controls, theme | sysctl/ioctl/mixer |
| Application Core | lifecycle, state, command dispatch | Qt Widgets types, hardware details |
| Services | brightness/audio/notification orchestration | desktop window policy |
| Desktop Integration | tray, D-Bus, X11, theme, autostart | hardware implementation |
| FreeBSD Platform | sysctl, ioctl, mixer, backlight, ACPI | UI state/layout |

## Implemented source map
- `src/app/main.cpp`: composition root, tray/window, asynchronous read-only probe results.
- `include/fcc/platform/probe_result.h`: shared probe records and diagnostics.
- `include/fcc/platform/brightness_backend.h`, `src/platform/freebsd/brightness_backend.cpp`: read-only backlight discovery.
- `include/fcc/platform/audio_backend.h`, `src/platform/freebsd/audio_backend.cpp`: read-only mixer/master-volume discovery.
- `src/platform_probe/main.cpp`: command-line diagnostics.
- `tests/unit/test_platform_values.cpp`: pure OSS volume decoder tests.

## Dependency direction
Presentation -> application/service contracts <- FreeBSD implementations. The P0 composition root temporarily invokes the probe interfaces directly; device calls remain outside widgets. Platform code never depends on Qt.

## Data flow
P0 composition root -> backend -> O_RDONLY device open + read ioctls -> result/diagnostics -> UI status label. No brightness, volume or mute write path exists.

## Threading/lifecycle
GUI objects stay on GUI thread. Initial discovery uses QtConcurrent. No thread-per-event. Requests time out; stale completions cannot overwrite a newly selected device. Shutdown stops new work, cancels/waits bounded in-flight operations, then releases adapters.

## Errors
Distinguish Unsupported, Unavailable, PermissionDenied, InvalidValue, Busy, TimedOut, IoError, InternalError. Keep OS detail for diagnostics; user messages safe. Never replace failure with plausible default.

## Configuration
Persist non-secret theme, display/audio IDs and UI preferences. Decide path/format after P0. Revalidate IDs at startup; use atomic writes/QSettings semantics.

## Rendering and open decisions
Widgets raster, explicit OpenGL contexts, Xorg compositor effects and driver selection are separate mechanisms. Measure separately. Open: brightness backend/permissions; mixer/device policy; tray protocol; popup focus; theme source; helper need (default no); meaningful GPU selection for Widgets.
