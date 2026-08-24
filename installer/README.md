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

| Platform | File | Minimum |
|----------|------|---------|
| Windows x64 | `ClassBuilderSetup-3.0-x64.exe` | Windows 10/11 x64 |
| macOS (Apple Silicon / arm64) | `ClassBuilder-3.0-mac-arm64.dmg` | macOS 13 (Ventura)+ |
| Linux x86_64 | `classbuilder_3.0_amd64.deb` | glibc ≥ 2.43 (Ubuntu 26.04+) |
| **Linux arm64 (recommended)** | `classbuilder_3.0_arm64-glibc2.38.deb` | **glibc ≥ 2.38** — Raspberry Pi OS / Debian 13 **and** newer |
| Linux arm64 (Ubuntu 26.04+) | `classbuilder_3.0_arm64.deb` | glibc ≥ 2.43 |

On **arm64, prefer `…_arm64-glibc2.38.deb`** — it is built on Raspberry Pi OS
(Debian 13), so its glibc floor is 2.38 and it runs on the widest range of arm64
systems (Pi OS, Debian 13, Ubuntu 24.04, Ubuntu 26.04, …). The plain
`…_arm64.deb` is built on Ubuntu 26.04 (glibc 2.43) and will **not** run on
Raspberry Pi OS / Debian 13.

### Which build runs where

A Linux binary runs on its build-time glibc **or newer, never older**. So the
lowest-glibc build has the widest reach.

| Your system (arch) | glibc | Use |
|--------------------|:-----:|-----|
| Raspberry Pi OS / Debian 13 (arm64) | 2.41 | `classbuilder_3.0_arm64-glibc2.38.deb` |
| Ubuntu 24.04 LTS (arm64) | 2.39 | `classbuilder_3.0_arm64-glibc2.38.deb` |
| Ubuntu 26.04+ (arm64) | 2.43 | either arm64 `.deb` (the `-glibc2.38` one also runs) |
| Debian 12 / Ubuntu 22.04 (arm64) | 2.36 / 2.35 | ❌ too old — [build from source](#need-an-older-distro) |
| Ubuntu 26.04+ (x86_64) | 2.43 | `classbuilder_3.0_amd64.deb` |
| x86_64 with glibc < 2.43 | < 2.43 | ❌ too old — [build from source](#need-an-older-distro) |

Check your glibc with `ldd --version`.

<a name="need-an-older-distro"></a>
#### Need it on an older distro?

The floor comes from the **statically-linked Qt** (baked in when Qt is built),
not from ClassBuilder's own code, so it can't be lowered by changing CB — you
rebuild the whole stack (Qt + CB) on the older distro, whose older glibc then
sets a lower floor. See `crossplatform/PORTING_LINUX.md` (option B, static Qt)
then run `installer/make-deb.sh` — it stamps the `.deb` for the arch of the
machine you build on. This is exactly how the `-glibc2.38` arm64 package above
was produced (built on Raspberry Pi OS rather than Ubuntu 26.04).

---

## Linux (`.deb`)

A Debian package for Debian-family distributions (Ubuntu, Debian, Raspberry Pi
OS). Native menu entry, `.cbz` double-click association, and clean
`apt`/`dpkg` uninstall.

### Install

```sh
sudo apt install ./classbuilder_3.0_amd64.deb              # x86_64
sudo apt install ./classbuilder_3.0_arm64-glibc2.38.deb    # arm64 (Pi OS / Debian 13 + newer)
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

### Minimum system requirements — glibc is the real gate

Each `.deb`'s glibc floor is set by the machine its **static Qt** was built on
(the floor is baked into Qt, not into ClassBuilder's own code — so it can't be
lowered by changing CB, only by building the stack on an older distro). See the
[compatibility table](#which-build-runs-where) above; in short:

- **`classbuilder_3.0_arm64-glibc2.38.deb`** — built on **Raspberry Pi OS
  (Debian 13)** → **glibc ≥ 2.38**. Runs on Pi OS, Debian 13, Ubuntu 24.04,
  Ubuntu 26.04 and newer arm64. This is the recommended arm64 download.
- **`classbuilder_3.0_arm64.deb`** and **`classbuilder_3.0_amd64.deb`** — built
  on **Ubuntu 26.04** → **glibc ≥ 2.43**. The highest symbol is `acosf@GLIBC_2.43`
  (from `libQt6Gui`); these will **not** run on Pi OS / Debian 13
  (`version 'GLIBC_2.43' not found`).

A binary built against a given glibc does **not** run on an older one, so the
lowest-glibc build reaches the most systems. Check yours with `ldd --version`.
On a distro below the floor, [build from source](#need-an-older-distro).

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
