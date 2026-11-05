# Proposed Service Contracts

Status: design proposal only; no public API frozen.

## Shared result
Carry success/failure, capability state, optional value, stable error category, safe diagnostic detail, timestamp and device identity. Categories: Unsupported, Unavailable, PermissionDenied, InvalidArgument, Busy, TimedOut, IoError, InternalError. Never fabricate defaults for missing data.

## Brightness
Enumerate display/backlight endpoints; read value and min/max; set validated value; read back; expose backend and permission status. Discovery differs from writable capability. Revalidate IDs at startup. A write is verified only after readback.

## Audio
Enumerate mixer devices/controls; read selected output volume/mute; set bounded volume/mute; return post-write state. Device selection separate from volume. Do not assume one mixer or uniform control names. Handle removal, busy, unsupported mute and denied permissions.

## Notifications
V1.0 supports FCC-originated notifications and selected app status. Do not replace system notification daemon. Treat content as data, not executable markup; delivery errors observable.

## Theme
Modes system/light/dark. System source may be unavailable; expose source/status and fallback. Manual override wins.

## Desktop
Tray adapter detects capability and handles activation/menu/absence. Popup adapter handles focus/dismissal/screen/edge. Session adapter manages reversible user autostart.

Freeze interfaces only after P0 evidence. Update callers, tests, docs and migration notes together.
