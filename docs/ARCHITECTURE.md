# Architecture

Status: P0 read-only backlight and mixer discovery plus a tray/popup with explicit master-volume control implemented; service/API boundaries remain provisional.

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
- `src/app/main.cpp`: composition root, tray lifecycle, asynchronous probe orchestration and notification-center shell.
- `include/fcc/gui/control_center/control_panel.h`, `src/gui/control_center/control_panel.cpp`: transient Qt popup, screen-bounded positioning and request signals; no platform access.
- `include/fcc/platform/probe_result.h`: shared probe records and diagnostics.
- `include/fcc/platform/brightness_backend.h`, `src/platform/freebsd/brightness_backend.cpp`: read-only backlight discovery.
- `include/fcc/platform/audio_backend.h`, `src/platform/freebsd/audio_backend.cpp`: read-only mixer discovery and guarded explicit master-volume write/readback/restore.
- `src/platform_probe/main.cpp`: command-line diagnostics.
- `tests/unit/test_platform_values.cpp`: OSS decode/encode, bounds, path rejection and mute-capability tests.
- `tests/unit/test_control_panel.cpp`: popup geometry/Escape, mixer selection, stereo values and explicit-Apply signal tests.

## Dependency direction
Presentation -> application/service contracts <- FreeBSD implementations. The P0 composition root temporarily invokes the probe interfaces directly; device calls remain outside widgets. Platform code never depends on Qt.

## Data flow
P0 composition root -> backend -> read-only brightness/mixer discovery -> snapshot/diagnostics -> popup state. A user Apply request -> asynchronous composition-root task -> validated `/dev/mixerN` O_RDWR write -> readback/restoration result -> UI reconciliation. Widgets never open devices. Brightness remains read-only; mute writes are absent because target devices do not advertise mute.

## Threading/lifecycle
GUI objects stay on GUI thread. Discovery and explicit volume writes use QtConcurrent. The current composition root serializes refresh and volume writes by disabling controls and rejecting overlapping requests. The current implementation waits for in-flight futures at shutdown; bounded cancellation/timeouts and stale-result generation tokens remain unimplemented and are release blockers for a hardened service layer.

## Errors
Distinguish Unsupported, Unavailable, PermissionDenied, InvalidValue, Busy, TimedOut, IoError, InternalError. Keep OS detail for diagnostics; user messages safe. Never replace failure with plausible default.

## Configuration
Persist non-secret theme, display/audio IDs and UI preferences. Decide path/format after P0. Revalidate IDs at startup; use atomic writes/QSettings semantics.

## Rendering and open decisions
Widgets raster, explicit OpenGL contexts, Xorg compositor effects and driver selection are separate mechanisms. Measure separately. Open: brightness backend/permissions; mixer/device policy; tray protocol; popup focus; theme source; helper need (default no); meaningful GPU selection for Widgets.
