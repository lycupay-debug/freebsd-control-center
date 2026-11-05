# Test and Acceptance Plan

Record host, OS, package versions, command, exit code, output, date and limits. Compile success is not GUI acceptance.

## P0 matrix
| ID | Test | Pass evidence |
|---|---|---|
| P0-01 | CMake configure/build | clean configure/build and warning review |
| P0-02 | GUI launch | visible in XFCE/Xorg; normal exit |
| P0-03 | Tray | visible icon, activation/menu work |
| P0-04 | Popup | focus, outside click, Escape, edge placement |
| P0-05 | Brightness discovery | real endpoint/range/permission/error |
| P0-06 | Brightness write | reversible write/readback; original restored |
| P0-07 | Mixer discovery | actual devices/controls |
| P0-08 | Audio write | volume/mute round-trip; original restored |
| P0-09 | Theme | system source or unavailable state and override |
| P0-10 | Rendering | renderer/context evidence; no driver changes |
| P0-11 | Failure | missing/denied capability no crash/false success |
| P0-12 | Test harness | CTest discovers actual tests; no fabricated pass |

P1: bounds, min/max, rounding, hotplug, denied permissions, absent endpoints, mixer switching, busy devices, unsupported mute, write failures, stale async completions, cancellation/timeouts/shutdown, real integration with safe restoration.

P2: tray host restart, notification placement/dismissal, theme persistence, autostart enable/disable and uninstall cleanup.

Stability: repeat core operations; check GUI responsiveness, memory/fd growth and log bounds. XFCE evidence does not imply broad desktop compatibility.

Current record: scaffold authored 2026-11-05; build/launch pending. No tray, hardware, GPU or deployment acceptance implied.
