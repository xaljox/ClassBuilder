# PORTING_LINUX.md — building ClassBuilder on Linux

Companion to [PORTING_MAC.md](PORTING_MAC.md). Covers building the committed
sources on Linux. Verified on **Ubuntu 24.04** — both x86_64 (WSL, the first
green Linux build, commit `ec86adf`) and **arm64** (Parallels on Apple Silicon).
The committed `linux-x64` preset name is cosmetic; it builds native for whatever
arch it runs on. On arm64 use arm64 apt packages (not amd64).

## Two Qt options

**A. Distro Qt (apt, fastest to green).** Ubuntu 24.04 ships **Qt 6.4.2**, and the
committed Linux-port sources build against it. The port commit (`ec86adf`)
deliberately kept the code 6.4-compatible (e.g. freedesktop icon-theme *string*
names instead of the Qt-6.7+ `QIcon::ThemeIcon` enum). This is the realistic
target for how Linux users get Qt.

```bash
sudo apt install -y build-essential cmake ninja-build pkg-config git gh \
                    qt6-base-dev qt6-svg-dev libgl1-mesa-dev libzstd-dev
cmake --preset linux-x64          # distro Qt + system libzstd
cmake --build --preset linux-debug
```
Exe: `out/build/linux-x64/bin/Debug/ClassBuilder`.

**B. From-source patched Qt 6.11.1 (parity with Win/Mac + native Wayland).** Needed
only if you want version parity, the dock tear-off fix, or native Wayland. Build
recipe mirrors [QT_DOCK_TEAROFF_PATCH.md](QT_DOCK_TEAROFF_PATCH.md) (the patch is
platform-independent). Install to `~/Qt-6.11.1-patched`, then
`cmake --preset linux-x64 -DCMAKE_PREFIX_PATH=$HOME/Qt-6.11.1-patched`.

> **CRITICAL build-order gotcha:** install the Wayland dev libs
> (`libwayland-dev wayland-protocols` + the `libxcb-*-dev` set) **before**
> configuring qtbase. Qt bakes `QT_FEATURE_wayland` at qtbase-configure time; if
> the libs aren't present, it's baked **OFF** and **qtwayland then silently builds
> nothing** (`ninja: no work to do`, no plugin). If that happened, reconfigure
> qtbase `--fresh` (libs now visible → `qtwaylandscanner ... yes` in
> config.summary), rebuild+install, then build qtwayland.
>
> Division of labour: **qtbase** builds the Wayland *client* (`libQt6WaylandClient`
> + the `libqwayland-generic.so` platform plugin). **qtwayland** builds the
> *compositor* + the client-side *decorations* (`libbradient.so`). A client app
> needs qtbase-with-wayland for the plugin and qtwayland for the title-bar
> decoration.

## Running

**xcb is the daily driver.** Under a Wayland session, the app defaults to the xcb
(X11/XWayland) platform (committed in `7087b5a`) — it has working title bars, all
cursors, and window dragging. Native Wayland *also* works but on **GNOME/Mutter it
won't drag Qt's client-side-decoration title bar** and the move/hand cursor is
missing, so xcb wins for daily use. (Wayland resize cursors need
`XCURSOR_THEME=Yaru`/`Adwaita`.)

## Rendering crispness (Parallels-specific)

Blurry/soft rendering under Parallels is **not** a Qt/backend issue — it's
Parallels resampling the VM framebuffer onto the Mac Retina panel, and it hits
xcb *and* Wayland equally. Fix: Parallels **View → Use Retina Resolution → "Best
for Retina"/"More Space"** + Ubuntu **Settings → Displays → Scale → 200%**
(`gnome-control-center display`). Then it's as crisp as the Win/Mac builds.
Full-screen does not help.

## Driving the GUI for tests

The command server is TCP `127.0.0.1:51777` on non-Windows. One JSON line
`{"cmd":"open_doc","params":{"path":"/abs/x.cbz"}}` opens a second model (copy the
file first — `open_doc` dedups by path) to exercise tabs/splits. See
[../tools/PIPE_API.md](../tools/PIPE_API.md).

## Line endings

`.gitattributes` pins **CRLF** in the working tree on every platform, Linux
included. Don't "fix" them.
