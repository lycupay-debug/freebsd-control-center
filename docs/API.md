# Proposed Service Contracts

Status: P0 discovery and guarded audio-write contracts implemented; public service boundaries remain provisional.

## Shared result
Carry success/failure, capability state, optional value, stable error category, safe diagnostic detail, timestamp and device identity. Categories: Unsupported, Unavailable, PermissionDenied, InvalidArgument, Busy, TimedOut, IoError, InternalError. Never fabricate defaults for missing data.

## Brightness (P0 read-only)
Enumerate backlight endpoints; read device name/type, current value and raw supported levels. Report min/max only when the driver supplies a level list. A zero level count is valid and means range metadata is unavailable. Write/update operations remain absent until range, permissions and rollback are verified.

## Audio (P0 discovery plus guarded master-volume write)
Enumerate mixer units; identify default unit when its sysctl is readable; read master left/right volume percentages. The guarded `setMasterVolume` operation accepts only canonical `/dev/mixerN` paths (N=0..31), validates each channel in 0..100, requires the master-volume capability bit, saves the original value, writes and verifies readback, and attempts restoration if verification fails. The popup exposes this operation through explicit Apply with independent left/right values; slider movement alone emits no write request. Mute remains unsupported unless the device advertises `SOUND_MASK_MUTE`.

## Notifications
V1.0 supports FCC-originated notifications and selected app status. Do not replace system notification daemon. Treat content as data, not executable markup; delivery errors observable.

## Theme
Modes system/light/dark. System source may be unavailable; expose source/status and fallback. Manual override wins.

## Desktop
Tray adapter detects capability and handles activation/menu/absence. Popup adapter handles focus/dismissal/screen/edge. Session adapter manages reversible user autostart.

Freeze interfaces only after P0 evidence. Update callers, tests, docs and migration notes together.


## Control panel UI contract (P0)

`fcc::gui::ControlPanel` accepts status and per-mixer volume snapshots and emits `refreshRequested`, `volumeApplyRequested(path, value)`, `notificationCenterRequested`, and `quitRequested`. It does not own platform backends or issue system calls. A volume change is submitted only after the user activates Apply; the composition root invokes `AudioBackend::setMasterVolume` asynchronously and reports the result. The UI exposes independent left/right sliders and preserves the selected mixer's stereo balance until the user changes a channel. The popup's `popupPosition` helper clamps placement to a supplied screen available-geometry rectangle.


### Audio write result

`VolumeWriteResult` reports success, optional observed volume, whether the original value was restored after a failed verification, and a diagnostic detail string. A successful return requires readback to match both requested channels. A failure does not imply that the hardware remained unchanged; callers must inspect the restoration status and surface the diagnostic.
