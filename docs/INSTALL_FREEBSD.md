# FreeBSD user installation and removal

## Scope

The current CMake install is a per-user development deployment. It does not install a privileged helper, change device permissions, add groups, write system configuration, or enable autostart. A FreeBSD pkg(8) package and a signed binary release are not yet produced.

## Prerequisites

- FreeBSD 15.1-RELEASE (initially validated on XFCE/Xorg).
- Qt 6 Widgets and Concurrent development/runtime packages.
- CMake 3.21 or newer and a C++20 compiler.
- A normal desktop user with read access to the supported backlight/mixer devices. Current FCC behavior is read-only.

## Build and install

Run as the logged-in desktop user, from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$HOME/.local"
```

Installed files include:

- `~/.local/bin/fcc` — tray and popup application.
- `~/.local/bin/fcc-platform-probe` — read-only platform diagnostic.
- `~/.local/share/applications/freebsd-control-center.desktop` — desktop menu entry.
- `~/.local/share/doc/FreeBSDControlCenter/` — README, MIT license and project documentation.

Start with `~/.local/bin/fcc`. The app is a normal-user process. If XFCE does not immediately show the new menu item, refresh the application menu through the desktop environment; do not run the GUI as root.

## Autostart policy

Autostart is intentionally not installed or enabled by the current deployment. It will be offered only after the tray lifecycle and shutdown behavior are verified. Any future autostart integration must be user-scoped, explicitly enabled, and removable without affecting other desktop settings.

## Verify installation

```sh
test -x "$HOME/.local/bin/fcc"
test -x "$HOME/.local/bin/fcc-platform-probe"
test -f "$HOME/.local/share/applications/freebsd-control-center.desktop"
ldd "$HOME/.local/bin/fcc"
```

Check that `ldd` reports no `not found` entries. Run the probe separately; it only reads device state.

## Remove this installation

CMake records exact installed file paths in `build/install_manifest.txt`. Review the file before removing anything. The following script removes only manifest paths under the current user's `~/.local` prefix; it refuses paths outside that prefix:

```sh
manifest="$PWD/build/install_manifest.txt"
prefix="$HOME/.local/"
test -f "$manifest" || { echo "install manifest not found" >&2; exit 1; }
while IFS= read -r path; do
    case "$path" in
        "$prefix"*) rm -f -- "$path" ;;
        *) echo "refusing path outside $prefix: $path" >&2; exit 1 ;;
    esac
done < "$manifest"
```

This does not remove the source repository, build directory, user configuration, logs, or any files not listed in the CMake install manifest. It does not attempt to remove parent directories that may contain other user files.
