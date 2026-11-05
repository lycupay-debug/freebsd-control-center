# Architecture

Status: proposed for P0; contracts provisional pending platform evidence.

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

## Source map
src/app composition root; src/core capability/state/contracts; src/gui control and notification surfaces; src/services brightness/audio/notification; src/desktop x11/dbus/tray/session; src/platform/freebsd OS implementation; tests/unit and tests/integration. Add files only with a clear responsibility.

## Dependency direction
GUI -> Application Core -> service interfaces <- FreeBSD implementations. Desktop adapters implement core-facing contracts. Platform code never depends on Qt Widgets.

## Data flow
Brightness: UI intent -> Core validates capability/range -> service -> backend bounded OS operation -> result/readback -> core publishes actual state -> UI displays value/error. Audio follows same flow; device selection is separate from volume.

## Threading/lifecycle
GUI objects stay on GUI thread. Discovery/I/O use bounded workers. No thread-per-event. Requests time out; stale completions cannot overwrite a newly selected device. Shutdown stops new work, cancels/waits bounded in-flight operations, then releases adapters.

## Errors
Distinguish Unsupported, Unavailable, PermissionDenied, InvalidValue, Busy, TimedOut, IoError, InternalError. Keep OS detail for diagnostics; user messages safe. Never replace failure with plausible default.

## Configuration
Persist non-secret theme, display/audio IDs and UI preferences. Decide path/format after P0. Revalidate IDs at startup; use atomic writes/QSettings semantics.

## Rendering and open decisions
Widgets raster, explicit OpenGL contexts, Xorg compositor effects and driver selection are separate mechanisms. Measure separately. Open: brightness backend/permissions; mixer/device policy; tray protocol; popup focus; theme source; helper need (default no); meaningful GPU selection for Widgets.
