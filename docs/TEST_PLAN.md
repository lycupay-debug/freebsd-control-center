# Test and Acceptance Plan

Record host, OS, package versions, command, exit code, output, date and limits. Compile success does not prove GUI acceptance.

## P0 matrix
| ID | Test | Current evidence |
|---|---|---|
| P0-01 | CMake configure/build | verified: FreeBSD 15.1-RELEASE, Qt 6.11.2, CMake 3.31.12; clean build |
| P0-02 | GUI launch | verified: normal-user launch in XFCE/Xorg; main and tray windows in X11 tree |
| P0-03 | Tray | user manually confirmed tray click opens/restores FCC; menu/host-restart behavior not yet tested |
| P0-04 | Popup | implemented: Qt::Popup, Escape dismissal and available-geometry clamping; Escape and placement automated tests pass; X11 outside-click/focus behavior still requires desktop acceptance |
| P0-05 | Brightness discovery | verified read-only: `/dev/backlight/backlight0`, `intel_backlight`, raw value observed as 9 in an earlier probe and 36 in a later probe; driver reports nlevels=0, so min/max unavailable |
| P0-06 | Brightness write | intentionally locked: nlevels=0 and loaded i915/LinuxKPI binary/source correspondence plus accepted range are not established; no write ioctl issued |
| P0-07 | Mixer discovery | verified read-only: mixer0/1/2 and master volume values 60:60, 86:86, 100:100 |
| P0-08 | Audio write/mute | backend setter verified on non-default mixer2: 100:100 -> 99:99 -> 100:100, both readbacks exact; default mixer0 remained 60:60; UI explicit-Apply request and per-device volume selection are automated-tested; mute remains unsupported |
| P0-09 | Theme | not implemented |
| P0-10 | Rendering | earlier Intel GLX acceleration evidence exists; FCC-specific renderer selection remains unverified |
| P0-11 | Failure | partial: invalid/absent endpoint paths retain explicit issues; permission-denied behavior not yet simulated |
| P0-12 | Test harness | verified: CTest runs 2 executables, both pass; control-panel suite covers popup, device selection and explicit Apply |

## Automated tests

- `fcc_platform_values`: pure OSS stereo-volume decoding/encoding; zero, 100 and out-of-range channels; invalid mixer path/range rejection; mute capability-bit interpretation.
- `fcc_control_panel`: popup geometry clamping (including a non-zero monitor origin), Escape dismissal, explicit Apply signal routing with distinct left/right values, per-device stereo-volume selection and disabled controls when no mixer exists.
- Popup outside-click behavior is delegated to Qt's native popup/window-system behavior. The offscreen QPA plugin does not model outside-click routing; this check must run under the target X11 window manager.
- Automated tests do not mutate host hardware. The separate one-time target experiment is recorded below and was explicitly restored. Qt tests verify that slider movement alone emits no write request.
- `fcc-platform-probe` exercises actual read-only hardware discovery on the development laptop.

## P1

- Verify X11 outside-click dismissal, tray menu actions, notification-center launch and popup focus behavior in a live XFCE session.

Remaining: brightness range/rollback; injected mixer write/readback/restoration failures; denied permissions; endpoint hotplug/removal; native XFCE outside-click/focus and tray menu; notifications, theme, persistence, autostart; long-run resource/shutdown tests.

## P2

Tray host restart, notification placement/dismissal, theme persistence, autostart enable/disable and uninstall cleanup.

## Stability

Repeat core operations; check GUI responsiveness, memory/fd growth and log bounds. XFCE evidence does not imply broad desktop compatibility.

## Target capability evidence update (2026-11-06)

- Backlight: normal-user read/write filesystem access checks pass for /dev/backlight/backlight0; current raw value is 9; nlevels=0. The generic FreeBSD backlight source passes UPDATE_STATUS to the device driver and does not establish a universal range. Actual write range and restore behavior remain unverified.
- Mixer: /dev/mixer0..2 all omit SOUND_MASK_MUTE in DEVMASK. READ_MUTE returned raw zero, but this is not a supported mute state. Mute remains unsupported for the observed devices.
- No hardware writes were performed. See docs/FREEBSD_DEVICE_CONTROL_RESEARCH.md for exact masks, source inspection, and safety decisions.


## Capability-bit regression test (2026-11-06)

- Added a pure unit test proving that mute is reported only when SOUND_MASK_MUTE is present in the advertised OSS device mask.
- The test includes the observed target mask 0x01002831, which must remain unsupported for mute.
- This tests capability interpretation only; it does not test or issue hardware mute writes.


## Audio write/readback/restore experiment (2026-11-06)

- Preconditions: `/dev/mixer2` was confirmed non-default and read 100:100; the default unit remained mixer0.
- Action: set mixer2 to 99:99, read back 99:99, then restore 100:100 and read back 100:100.
- Result: both operations returned success; a final independent read-only probe confirmed mixer2=100:100 and mixer0=60:60.
- Scope: validates only this target's master-volume write path and exact restoration. It is not evidence for mute, other hardware or injected failure paths. UI request routing is covered separately by Qt tests; live XFCE slider interaction still requires desktop acceptance.
