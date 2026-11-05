# Product Requirements

Status: baseline; implementation not accepted. Version 0.1.0.

## Intent and environment
FCC is a native FreeBSD desktop control center: compact system controls plus independent notification center. It does not replace desktop settings, notification daemon, network manager or audio server. First target: FreeBSD 15.1-RELEASE amd64, Xorg, XFCE. Run unprivileged. Missing devices/services and denied permissions are valid states.

## V1.0 requirements
| ID | Requirement | Acceptance evidence |
|---|---|---|
| FCC-F-001 | Tray entry | Visible; activation/menu work in XFCE |
| FCC-F-002 | Compact popup | Focus, outside click, Escape, edge placement tested |
| FCC-F-003 | Brightness read | Real endpoint/range/permission/value recorded |
| FCC-F-004 | Brightness write | Reversible write/readback; original restored |
| FCC-F-005 | Mixer enumeration | Devices and controls recorded |
| FCC-F-006 | Volume control | Write/readback; original state restored |
| FCC-F-007 | Mute | Mute/unmute round-trip |
| FCC-F-008 | Notification center | Independent lifecycle/placement verified |
| FCC-F-009 | FCC notifications | Delivery/display verified |
| FCC-F-010 | Theme | System detection and manual override tested |
| FCC-F-011 | Preferences | Persist across restart |
| FCC-F-012 | Diagnostics | UI reflects observed capability |
| FCC-F-013 | XFCE autostart | Login/logout and disable/uninstall tested |

## Non-functional
GUI thread never blocks on device I/O; operations are bounded; errors explicit; hardware backends isolated from Qt Widgets; no root GUI; user-owned secret-free config; missing optional capability does not crash; no false success or silent fallback.

## Out of scope
Bluetooth, Wi-Fi management, power policy, replacing notification daemon, desktop plugin framework, complex 3D effects, Wayland-specific positioning/hotkeys, broad cross-desktop guarantees. KDE/MATE/LXQt/Cinnamon/GNOME/Wayland are later targets.

## UX and platform strategy
Tray opens control surface; notification center separate. Test focus, outside click, Escape, scaling, edges and multi-monitor. Blur is optional; readable fallback required. Brightness inventory is read-only first. Inspect FreeBSD mixer; do not assume PulseAudio/PipeWire. Distinguish Widgets raster, OpenGL context, Xorg compositor and GPU selection. NVIDIA only after repeatable evidence; Intel next; CPU Raster fallback. No CUDA/NVDEC or driver reconfiguration.

## Release gates
P0 platform feasibility; P1 services and MVP controls; P2 notifications/preferences/diagnostics/autostart; P3 packaging/install/uninstall/rollback. A plan or successful compile alone does not complete a gate.
