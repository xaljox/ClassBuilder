# ClassBuilder 3.0 — Installers

ClassBuilder is a code-generation tool: you define an object model (classes,
members, methods, relations, diagrams) in a GUI and it writes the C++ `.h`/`.cpp`
source for that model. Every installer is **self-contained** — Qt is linked
statically, so no Qt runtime has to be installed separately.

Each installer also ships, next to the app: the **manual (PDF)**, an **example
model** (`Matrix.CBZ`), and the **compile-runtime** (the `CB_*` headers + value/
serialize sources you need to compile the code ClassBuilder generates). All are
reachable from the Help menu once installed.

## Downloads (v3.0)

Pick the file for your platform. The version is in the filename, so successive
releases sit side by side.

| Platform | File |
|----------|------|
| Windows x64 | `ClassBuilderSetup-3.0-x64.exe` |
| macOS (Apple Silicon / arm64) | `ClassBuilder-3.0-mac-arm64.dmg` |
| Linux x86_64 | `classbuilder_3.0_amd64.deb` |
| Linux arm64 | `classbuilder_3.0_arm64.deb` |

---

## Linux (`.deb`)

A Debian package for Debian-family distributions (Ubuntu, Debian, Raspberry Pi
OS). Native menu entry, `.cbz` double-click association, and clean
`apt`/`dpkg` uninstall.

### Install

```sh
sudo apt install ./classbuilder_3.0_amd64.deb      # x86_64
sudo apt install ./classbuilder_3.0_arm64.deb      # arm64
```

`apt install ./<file>` pulls the required system libraries automatically. (You
can also use `sudo dpkg -i <file>`, but then you resolve any missing
dependencies yourself.)

### What it installs

| Path | |
|------|-|
| `/opt/classbuilder/ClassBuilder` | the application |
| `/usr/bin/classbuilder` | launcher symlink (run `classbuilder` from a terminal) |
| `/opt/classbuilder/{doc,examples,runtime}` | manual PDF, example model, compile-runtime |
| menu entry + icon | "ClassBuilder" in the applications menu |
| `.cbz` association | double-click a model to open it |

### Uninstall

```sh
sudo apt remove classbuilder
```

### Minimum system requirements — READ THIS re: which distro

The packages published here are built on **Ubuntu 26.04**, and that sets a
**hard floor**:

- **glibc ≥ 2.43** → **Ubuntu 26.04 (or newer), or any distro whose glibc is
  ≥ 2.43.** A binary built against a given glibc does **not** run on an older
  one. Check yours with `ldd --version`.
  - This floor does **not** come from ClassBuilder's own code — it comes from
    the **statically-linked Qt** (`libQt6Gui`, compiled on Ubuntu 26.04). One
    symbol, `acosf@GLIBC_2.43`, is the highest; without it the binary needs only
    glibc 2.38. Because it is baked in when **Qt** is built, it can't be dropped
    by changing CB — the fix is to build the whole stack (Qt + CB) on an
    **older** distro, whose glibc then sets a lower floor. That is exactly why
    the Pi build (oldest glibc) runs everywhere.
  - **Raspberry Pi OS / Debian 13 and earlier have an OLDER glibc and will
    refuse to run these packages** (`version 'GLIBC_2.43' not found`). For those,
    a `.deb` built on that older system is needed (a build against older glibc
    runs on both it and newer systems, not the reverse). If you need Pi/Debian
    coverage, build from source there — see `crossplatform/PORTING_LINUX.md`
    (option B, static Qt) + `installer/make-deb.sh`.
- **libstdc++6** providing `GLIBCXX_3.4.30` (GCC 12+) — present on any current
  desktop.
- **Desktop baseline libraries**, pulled in automatically by the package's
  `Depends` (Qt is static, so these are *only* the platform libs Qt links against
  the system): `libxcb1` + `libxcb-cursor0` and the rest of the `libxcb-*` set,
  `libx11-6`/`libx11-xcb1`, `libxkbcommon0`/`libxkbcommon-x11-0`, `libfontconfig1`,
  `libfreetype6`, `libgl1`/`libglx0`, `libglib2.0-0`, `libdbus-1-3`, plus the
  usual `libc6`, `libstdc++6`, `zlib1g`. Any GNOME/KDE/Xfce desktop already has
  all of these; there is no single "minimum libxcb version" that matters in
  practice — **glibc is the only real gate.**

The app runs on **X11 / XWayland** by default (it forces the `xcb` platform under
a Wayland session, for working title bars, cursors and window dragging). See
`crossplatform/PORTING_LINUX.md` for the details.

---

## Windows (`.exe`)

Installer: `ClassBuilderSetup-3.0-x64.exe` (Inno Setup). Full-static build — no
VC++ redistributable and no Qt DLLs required. Installs to
`C:\Program Files\ClassBuilder`, adds a Start-menu shortcut and the `.cbz`
association, and includes an uninstaller.

*(Expand this section with the Windows minimum — e.g. Windows 10/11 x64 — when
confirmed.)*

## macOS (`.dmg`)

Disk image: `ClassBuilder-3.0-mac-arm64.dmg` (Apple Silicon / arm64). Drag
`ClassBuilder.app` onto Applications. Runs on **macOS 13 (Ventura) and later**.

The app is **not signed** with an Apple Developer certificate, so macOS blocks it
on first launch ("damaged and can't be opened" / "developer cannot be verified").
The download is fine — this is Gatekeeper's policy for unsigned apps. Allow it
once with:

```sh
xattr -dr com.apple.quarantine /Applications/ClassBuilder.app
```

See `installer/dmg-readme.txt` (bundled inside the `.dmg`) for the full first-run
note.
