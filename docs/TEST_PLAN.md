# Test and Acceptance Plan

Record host, OS, package versions, command, exit code, output, date and limits. Compile success does not prove GUI acceptance.

## P0 matrix
| ID | Test | Current evidence |
|---|---|---|
| P0-01 | CMake configure/build | verified: FreeBSD 15.1-RELEASE, Qt 6.11.2, CMake 3.31.12; clean build |
| P0-02 | GUI launch | verified: normal-user launch in XFCE/Xorg; main and tray windows in X11 tree |
| P0-03 | Tray | user manually confirmed tray click opens/restores FCC; menu/host-restart behavior not yet tested |
| P0-04 | Popup | not implemented/verified: focus, outside click, Escape and edge placement |
| P0-05 | Brightness discovery | verified read-only: `/dev/backlight/backlight0`, `intel_backlight`, current 9; driver reports nlevels=0, so min/max unavailable |
| P0-06 | Brightness write | not implemented or tested; no write ioctl in P0 adapter |
| P0-07 | Mixer discovery | verified read-only: mixer0/1/2 and master volume values 60:60, 86:86, 100:100 |
| P0-08 | Audio write/mute | not implemented or tested; no write ioctl in P0 adapter |
| P0-09 | Theme | not implemented |
| P0-10 | Rendering | earlier Intel GLX acceleration evidence exists; FCC-specific renderer selection remains unverified |
| P0-11 | Failure | partial: invalid/absent endpoint paths retain explicit issues; permission-denied behavior not yet simulated |
| P0-12 | Test harness | verified: CTest discovers and passes 1 unit-test executable |

## Automated tests

- `fcc_platform_values`: pure OSS stereo-volume decoding; zero, 100 and out-of-range channels.
- Tests do not mutate host hardware.
- `fcc-platform-probe` exercises actual read-only hardware discovery on the development laptop.

## P1

Bounds, min/max, rounding, hotplug, denied permissions, absent endpoints, mixer switching, busy devices, unsupported mute, write failures, stale async completions, cancellation/timeouts/shutdown, real integration with safe restoration.

## P2

Tray host restart, notification placement/dismissal, theme persistence, autostart enable/disable and uninstall cleanup.

## Stability

Repeat core operations; check GUI responsiveness, memory/fd growth and log bounds. XFCE evidence does not imply broad desktop compatibility.
