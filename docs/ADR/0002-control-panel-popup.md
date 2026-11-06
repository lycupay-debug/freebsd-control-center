# ADR-0002: Control panel popup window policy

- Status: Accepted for the initial XFCE/Xorg implementation
- Date: 2026-11-06
- Context: The tray entry must expose a compact transient control panel without keeping a normal taskbar window open.
- Decision: Use a top-level Qt Widgets `QFrame` with `Qt::Popup | Qt::FramelessWindowHint`. The application composition root supplies an anchor and the active screen's available geometry. The panel clamps its geometry to that screen and chooses above/below placement based on the anchor's vertical position.
- Dismissal: Rely on Qt's native popup semantics for Escape and outside-window interaction. Escape is automated-tested; outside-click routing is a live window-system acceptance item because the offscreen QPA plugin and synthetic QtTest clicks do not model the X11 window manager's native routing.
- Constraints: The panel has no platform API dependency. Hardware reads remain asynchronous in the composition root. No write controls are exposed by this P0 UI.
- Consequences: The first target is XFCE/Xorg. Wayland, multi-desktop tray protocol differences, and multi-monitor behavior must be verified separately.
- Rejected: A persistent normal `QWidget` window, because it does not naturally provide transient popup dismissal and focus behavior.
