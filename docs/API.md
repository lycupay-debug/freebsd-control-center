# Proposed Service Contracts

Status: initial read-only P0 platform contracts implemented; public control/write API remains unfrozen.

## Shared result
Carry success/failure, capability state, optional value, stable error category, safe diagnostic detail, timestamp and device identity. Categories: Unsupported, Unavailable, PermissionDenied, InvalidArgument, Busy, TimedOut, IoError, InternalError. Never fabricate defaults for missing data.

## Brightness (P0 read-only)
Enumerate backlight endpoints; read device name/type, current value and raw supported levels. Report min/max only when the driver supplies a level list. A zero level count is valid and means range metadata is unavailable. Write/update operations remain absent until range, permissions and rollback are verified.

## Audio (P0 read-only)
Enumerate mixer units; identify default unit when its sysctl is readable; read master left/right volume percentages. Mute and volume writes are not implemented. Mute capability and semantics require separate verification.

## Notifications
V1.0 supports FCC-originated notifications and selected app status. Do not replace system notification daemon. Treat content as data, not executable markup; delivery errors observable.

## Theme
Modes system/light/dark. System source may be unavailable; expose source/status and fallback. Manual override wins.

## Desktop
Tray adapter detects capability and handles activation/menu/absence. Popup adapter handles focus/dismissal/screen/edge. Session adapter manages reversible user autostart.

Freeze interfaces only after P0 evidence. Update callers, tests, docs and migration notes together.
