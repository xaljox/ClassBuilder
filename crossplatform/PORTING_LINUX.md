# PORTING_LINUX.md — building ClassBuilder on Linux

Companion to [PORTING_MAC.md](PORTING_MAC.md). Covers building the committed
sources on Linux. The committed `linux-x64` preset name is cosmetic; it builds
native for whatever arch it runs on. On arm64 use arm64 apt packages (not amd64).

**Verified environments:**

| Arch | Distro | Host | Notes |
|------|--------|------|-------|
| x86_64 | Ubuntu 24.04 | WSL | First green Linux build, commit `ec86adf` |
| x86_64 | Ubuntu 26.04 | native box | Native hardware build (not WSL) — the current dev environment |
| arm64 | Ubuntu 24.04 → **26.04** | Parallels on Apple Silicon | Upgraded to 26.04 for the scaling fix — see "GNOME ≥ 47 scaling" below |
| arm64 | Raspberry Pi OS (Debian-based) | Pi 500+ | Default distro; distinct arm64 target from the Parallels VM (Broadcom GPU, Pi's Qt apt version) |

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

## GNOME ≥ 47 scaling (Mutter)

The arm64 Parallels guest was upgraded **Ubuntu 24.04 → 26.04** to get a newer
**GNOME/Mutter (≥ 47)** because of scaling issues on the older compositor.
Mutter ≥ 47 is the threshold where fractional scaling behaves correctly for the
CB window; on older Mutter the scale was wrong. This is a **desktop/compositor**
requirement, not a CB-code one — CB's own `View > UI Scale` menu (below) is the
in-app fallback, but the compositor's fractional scaling is what the 26.04
upgrade fixed. Verify per-WM: this is specific to GNOME/Mutter and won't
generalize to labwc/KDE.

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

## Manual UI scale (View > UI Scale menu)

Added 2026-07-04 for HiDPI monitors run at native resolution (e.g. a 4K panel
that's too small unscaled and too blurry at a lower non-native mode). Persists a
factor (`QSettings` key `shell/uiScale`) applied as `QT_SCALE_FACTOR` in
`QtApp.cpp`'s `Qt_EnsureApplication()` -- **before** `QApplication` is
constructed, since Qt only reads it at that point; changing it in the menu
saves the setting and offers a restart (self-relaunch via `QProcess`), it can't
apply live to the running window tree.

Two things this does NOT reach, both Linux/X11-specific and worth knowing before
re-debugging "the scale looks wrong" on another distro/WM:

- **The X11/Xcursor pointer theme.** `QT_SCALE_FACTOR` is a software scale Qt
  applies to its own painting only; the cursor is a fixed-pixel-size resource
  (`XCURSOR_SIZE`) loaded by `libXcursor` independent of Qt, so without a
  companion fix the pointer renders at native size and looks shrunk the moment
  it enters a scaled CB window. `Qt_EnsureApplication()` also scales
  `XCURSOR_SIZE` (off whatever base value is already in the environment,
  default 24) in the same block, process-local only -- doesn't touch the rest
  of the desktop. No-op on Windows/macOS (neither reads that var).
- **The window manager's own title bar.** Under xcb/XWayland (see "Running"
  above), the title bar showing "\<file\> - ClassBuilder" is painted entirely by
  the WM/compositor (labwc, GNOME/Mutter, etc.) -- it is NOT Qt, so
  `QT_SCALE_FACTOR` cannot touch its font size, no matter how it looks relative
  to CB's own (now-scaled) menu/tab text. If it reads as redundant/annoying next
  to CB's own tab label, that's a **machine-local desktop config fix**, not a CB
  one -- e.g. on labwc: `~/.config/labwc/rc.xml` → `<theme><titlebar><showTitle>
  no</showTitle></titlebar>`, then `labwc --reconfigure` (or `pkill -HUP labwc`).
  This does NOT retroactively repaint already-open windows -- only newly-mapped
  ones pick up a theme reload, which reads as "the fix didn't work" if you're
  still looking at a window opened before the reload. Not something to chase in
  this repo; the equivalent GNOME/KDE knob will differ by WM.

**Verify on other distros/WMs**: XCURSOR_SIZE handling and WM decoration
behaviour are compositor-specific -- confirm both the cursor and the (separate,
CB-side) dock-title-bar consistency fix in
[KNOWN_ISSUES.md](KNOWN_ISSUES.md#4-dock-title-bar-redundantoversized-text-vs-its-own-tab-label--fixed-linux-2026-07-04)
still look right on GNOME/KDE, and on Windows/macOS where the whole scale
feature is new and untested.

## Line endings

`.gitattributes` pins **CRLF** in the working tree on every platform, Linux
included. Don't "fix" them.
