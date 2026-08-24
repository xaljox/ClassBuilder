# ClassBuilder installers / packaging — cross-platform coordination

Auto-memory does **not** sync between machines, so this tracked file is the
handoff for per-platform installer work. Read this together with
`PORTING_MAC.md` / `PORTING_LINUX.md`, and see **"Release" at the bottom** for
the live v3.0 status (what is attached, what is still needed, and the order).

App version: **3.0** — a new major release (the original ClassBuilder ~25 years
ago was 2.x). Keep it in sync across all installers.

It lives in **two** hand-maintained places plus the model:
`CB_VERSION` in [../CMakeLists.txt](../CMakeLists.txt) (drives the macOS bundle's
`CFBundleShortVersionString`/`CFBundleVersion`) and `MyAppVersion` in
[../installer/ClassBuilder.iss](../installer/ClassBuilder.iss) — Inno Setup can't
read CMake, so that pair is manual. The **`Project: ClassBuilder v<N>` banner in
every generated source header comes from the model**, so it only changes in CB
itself (then regenerate); it is not editable on disk — bumped to **v3.0** in all
281 generated headers on 2026-08-14 (commit 892aad6). The *default* header
template for **new user projects** carries no version line on purpose (a new
project's header belongs to the user).

## What every installer ships (mirror this across platforms)

| Item | Path in repo | Notes |
|---|---|---|
| **The app** | built exe / `.app` / binary | self-contained: static Qt (Windows also static CRT) |
| **Manual PDF** | `docs/manual/ClassBuilder_Manual.pdf` | **committed on purpose** — it is generated from the `.docx` via Word COM, which is **Windows-only**. Other platforms **bundle the committed file**; they cannot regenerate it. Refresh it on Windows when the manual changes. |
| **Example model** | `models/manual/Matrix.CBZ` | the clean Matrix model |
| **Compile-runtime** | `include/` `value/` `serialize/` `third_party/zstd/` | the `CB_*` headers + value/serialize sources + zstd header & static lib — what a user needs to compile the code ClassBuilder generates |
| **`.cbz` association** | per-platform | double-click a model → opens ClassBuilder |
| **Licence** | `LICENSE` | MIT. **Required in the package, not optional:** the shipped compile-runtime is a substantial portion of the Software and carries no per-file notice, and the generated headers say "see the LICENSE file" — which has to exist in what the user receives. All three installers ship it: macOS `Contents/Resources/LICENSE` + a visible `LICENSE.txt` in the image; Windows `{app}\LICENSE.txt` (renamed — an extensionless file opens with nothing there); Linux `/opt/classbuilder/LICENSE` **and** `/usr/share/doc/classbuilder/copyright`, the Debian-conventional path lintian expects. |

## Windows — ✅ DONE (Inno Setup)

- Script: **`installer/ClassBuilder.iss`** (Inno Setup 7). Build:
  `cmake --build --preset x64-release`, then
  `"C:\Program Files\Inno Setup 7\ISCC.exe" installer\ClassBuilder.iss`
  → `installer/output/ClassBuilderSetup.exe` (~17 MB, gitignored — regenerable).
- The exe is **full-static** (`/MT` static CRT + static-runtime Qt at
  `C:\Qt-static-mt`): **no VC++ redistributable, no Qt DLLs**. (`dumpbin
  /dependents` = only Windows system DLLs.) See `CMakeLists.txt`
  `CMAKE_MSVC_RUNTIME_LIBRARY` + the `x64` preset's `CMAKE_PREFIX_PATH`.
- Installs to `C:\Program Files\ClassBuilder`; registers `.cbz` → `ClassBuilder.Model`
  (HKLM) + `ClassBuilderDoc.ico`, Start-menu shortcut, uninstaller.
- **Assoc gotcha (handled):** a stale per-user `HKCU\Software\Classes\.cbz`
  shadows the machine-wide HKLM one (HKCU wins, same ProgId). The `.iss` clears it
  in admin mode + `ChangesAssociations=yes` broadcasts the change to Explorer.

## macOS — ✅ DONE (.dmg, 2026-08-14)

**Foundations** — see `PORTING_MAC.md` → *"Static Qt on macOS — DONE"* and the
2026-06-25/26 commits:
- **Self-contained static binary**: `mac-static` preset (in the *local*
  `CMakeUserPresets.json`, which is gitignored — recreate it on the Mac) + static
  patched Qt at `~/Qt-6.11.1-patched-static` → a **24 MB single binary**;
  `otool -L` shows only system libs, no Qt frameworks, `macdeployqt` not needed.
  zstd already static.
- **`.cbz` file association** — already fixed + committed (Info.plist document
  types), so double-click already works once the app is installed.

**The installer itself:**

- Script: **`installer/make-dmg.sh`** — the counterpart of the Windows `.iss`,
  shipping the same payload. Build:
  `cmake --build --preset mac-static-release`, then `./installer/make-dmg.sh`
  → `installer/output/ClassBuilder-<ver>-mac-<arch>.dmg` (**18 MB**, gitignored —
  regenerable). Optional arg: a path to an alternate `.app`.
- **The payload goes INSIDE the bundle**, at `Contents/Resources/{doc,examples,
  runtime}` — mirroring Windows' `{app}\doc,\examples,\runtime`. This is
  deliberate: files merely sitting beside the app in the `.dmg` are **lost** when
  the user drags only the `.app` to `/Applications`, so anything needed after
  install has to travel inside it.
- The `.dmg` holds the app + an `/Applications` symlink (drag-to-install).
- **zstd:** the committed `third_party/zstd/lib*` are Windows `.lib` files and
  useless here, so the script bundles brew's `libzstd.a` when its arch matches,
  else ships headers only and says so. The headers are always included.
- **Hard gate:** the script refuses to package a non-static build (`otool -L`
  must show only system libs). A shared-Qt `.app` would package fine and then
  fail on someone else's Mac with a missing-framework dialog.
- The script never calls `rm` — staging is a fresh `mktemp -d`, the `.dmg` is
  overwritten with `hdiutil -ov`.
- Bundle version now comes from **`CB_VERSION` in `CMakeLists.txt`** (it was
  hardcoded `1.0`, so every bundle before this shipped mislabelled).

### Deployment target: macOS 13+ — and why it needs its own Qt and zstd

The `mac` preset sets `CMAKE_OSX_DEPLOYMENT_TARGET=13.0`, so the app runs on
**macOS 13 (Ventura) and later**. It was `26.0` until 2026-08-14, which meant
every DMG built before then refused to launch on anything older than macOS 26 —
on Apple Silicon too. Check any build with:

```sh
otool -l <app>/Contents/MacOS/ClassBuilder | grep -A3 LC_BUILD_VERSION   # minos
```

**Setting the target is NOT enough by itself, and this is the trap.** The
deployment target only stamps CB's own objects; anything CB *links* keeps the
target it was built with. Both usual sources are compiled against the host SDK:

- **Homebrew bottles** are always built for the running macOS — on this box, 26.
- **A from-source Qt** built without an explicit target inherits the host SDK
  for its generated **plugin-init objects**, even when qtbase's own libraries
  say 13.

The result is a binary *labelled* `minos 13.0` that contains 26.0-built objects
— it links with `ld: warning: object file ... was built for newer 'macOS'
version (26.0)`, and may then fail at load or first call on a real macOS 13
machine, which cannot be tested from a macOS 26 box. **Treat those linker
warnings as errors: a correct ship build produces ZERO of them.**

So the ship build uses purpose-built dependencies (both local, not in git):

| | Path | Built with |
|---|---|---|
| Qt 6.11.1 static | `~/Qt-6.11.1-static-13` | `-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0` |
| zstd 1.5.7 static | `~/zstd-macos13/lib/libzstd.a` | `CFLAGS=-mmacosx-version-min=13.0` |

The gitignored `mac-static` preset points at both (`CMAKE_PREFIX_PATH` +
`ZSTD_LIB`, which overrides the `find_library` HINTS in `CMakeLists.txt`).
Do **not** pass `-DCMAKE_OSX_ARCHITECTURES=arm64` when building Qt — it then
believes it is cross-compiling and demands `QT_HOST_PATH`; a native build is
already arm64.

**After repointing `CMAKE_PREFIX_PATH` at a different Qt, reconfigure with
`--fresh`.** CMake caches `Qt6Svg_DIR` and friends, so a plain reconfigure keeps
resolving Svg against the OLD prefix — which showed up here as exactly half the
warnings surviving, all naming the previous Qt.

The dev preset (`mac-patched`, brew Qt) still links host-SDK objects and will
warn. That is fine for development; just never ship from it.

**Still open on macOS:**
1. **Signing / notarization — PARKED: no Apple Developer account** (JV,
   2026-08-14). Notarizing *requires* a paid Apple Developer Program membership
   (~USD 99/year) for a Developer ID certificate; there is no free route, and
   `codesign --sign -` (ad-hoc, what the script does) can never satisfy
   Gatekeeper no matter how it is invoked. **The shipped answer is therefore the
   recipient-side unquarantine**, which works fine and costs nothing.
   **Give recipients the command, not the right-click tip:**

   ```sh
   xattr -dr com.apple.quarantine /Applications/ClassBuilder.app
   ```

   The old **right-click → Open** shortcut is no longer reliable: macOS 15
   (Sequoia) removed that bypass for unsigned apps, so on 15/26 the GUI route is
   *System Settings → Privacy & Security → **Open Anyway*** after a first
   blocked launch. The `xattr` command works on every version and is one step.
   Tell recipients up front — an unexplained "damaged and can't be opened"
   dialog reads as a corrupt download, not a signing policy.

   Note quarantine is attached by the **downloader**, so a `.dmg` sent by
   AirDrop / USB / a local copy may carry no quarantine flag at all, and a
   locally-built `.app` never does. **Testing on this Mac therefore proves
   nothing about the recipient experience** — to test it for real, download the
   `.dmg` through a browser. Revisit only if CB is distributed beyond people who
   can be given that instruction.
2. **Intel (x86_64) `.dmg` — DONE 2026-08-24. A SECOND, separate dmg, NOT a
   universal binary.** `ClassBuilder-3.0-mac-x64.dmg` mirrors the per-arch Linux
   `.deb` split. **Why not universal:** the binary is ~27 MB because Qt is static,
   so a universal (arm64+x86_64) would roughly DOUBLE it and every Mac would
   download the half it can't use. Two dmgs → each user grabs their own arch.
   (Universal only pays off for small, dynamically-linked apps.)

   **How (built on the Apple-Silicon Mac — the macOS SDK is "fat", so emitting
   x86_64 is a normal build; Rosetta is not needed for the build):**
   - Static **x86_64** Qt 6.11.1 → `~/Qt-6.11.1-static-13-x64`, from the same
     PATCHED source, by `~/qt-build/build-static-qt-x64.sh` (the x86_64 twin of
     `build-static-qt-x64.sh`'s arm64 original). **Gotcha:** Qt treats an x86_64
     target on an arm64 host as a *cross-build* and hard-fails configure with
     "You need to set QT_HOST_PATH to cross compile Qt" — so the script passes
     `-DQT_HOST_PATH=$HOME/Qt-6.11.1-static-13`, reusing the arm64 install's
     `moc`/`rcc`/`uic` natively. Build is ~2 min on this Mac, not the ~40 min the
     Linux container recipe takes.
   - **x86_64** static zstd at `~/zstd-macos13-x64`, built from
     `~/zstd-build/zstd-1.5.7` with `-arch x86_64 -mmacosx-version-min=13.0`
     (mirrors the arm64 ship recipe two rows up).
   - The `mac-x64` preset (gitignored `CMakeUserPresets.json`) inherits `mac` with
     `CMAKE_OSX_ARCHITECTURES=x86_64` + those two prefixes. Configure with
     `--fresh` (CMake caches `Qt6*_DIR` and would otherwise reuse the arm64 Qt):
     `cmake --preset mac-x64 --fresh && cmake --build --preset mac-x64-release`.
   - `ZSTD_A=~/zstd-macos13-x64/lib/libzstd.a ./installer/make-dmg.sh \
     out/build/mac-x64/bin/Release/ClassBuilder.app`. The script now names the
     asset `-x64` itself and picks an **arch-matching** `libzstd.a` (the old
     `brew --prefix zstd` lookup returned the arm64 lib when cross-packaging, so
     the Intel dmg silently shipped headers only); `$ZSTD_A` overrides. The Read
     Me's "Platform" line is likewise filled per-arch.
   - Verified: `lipo -archs` → `x86_64`, `minos` → 13.0, `otool -L` → system libs
     only, zstd payload x86_64, and the app launches and opens `Matrix.CBZ` on
     this Apple-Silicon Mac under Rosetta. Unsigned → same `xattr` quarantine note
     as #1. Apple-Silicon users should still take the arm64 dmg.

3. **Bundle-ID collision when testing:** build-tree copies and the
   `/Applications` copy all use `com.xaljox.ClassBuilder`, so Launch Services
   points `.cbz` at whichever was registered last. After testing a build-tree
   app, re-assert the installed one with
   `lsregister -f /Applications/ClassBuilder.app`.
4. **arm64 Linux `.deb` at glibc 2.35 — can be built HERE, on the Mac's arm64
   Ubuntu Parallels VM (no Pi needed).** The Pi already produced the `-glibc2.38`
   arm64 package (covers Pi OS / Debian 13 + newer); a `-glibc2.35` one additionally
   covers arm64 Ubuntu 22.04 / Debian 12. Build it WITHOUT disturbing the VM's own
   26.04, inside an `arm64v8/ubuntu:22.04` **container** (native arm64, no emulation):
   ```sh
   docker run --rm -v "$PWD":/src -w /src arm64v8/ubuntu:22.04 bash installer/build-in-container.sh
   ```
   → `installer/output/classbuilder_3.0_arm64-glibc2.35.deb`; attach with
   `gh release upload v3.0 <file> --clobber`. (The same script builds the amd64 one on
   an x86_64 host; CI already produces amd64 — see the Linux section.) **Optional:**
   the Pi's `-glibc2.38` arm64 already covers the Pi; do this only if arm64 Ubuntu
   22.04 / Debian 12 coverage is wanted.

## Linux — ✅ DONE: glibc-labelled per-arch packages (updated 2026-08-25)

**Published on v3.0 (Linux) now:**
- `classbuilder_3.0_amd64-glibc2.35.deb` — built in **CI**
  (`ubuntu:22.04` container, `.github/workflows/linux-amd64-deb.yml`); runs on
  Ubuntu 22.04 / Debian 12 and newer.
- `classbuilder_3.0_arm64-glibc2.38.deb` — built **natively on the Pi**
  (Raspberry Pi OS / Debian 13); runs on Pi OS / Debian 13 and newer.
- `classbuilder_3.0_amd64.deb` / `_arm64.deb` — the earlier **Ubuntu 26.04**
  (glibc 2.43) builds; kept, superseded by the lower-glibc ones above.

**Two ways to build a low-glibc `.deb`** (floor goes in the filename either way):
1. **Container** — `installer/build-in-container.sh` in an `ubuntu:22.04` image;
   arch follows the host, one `docker run`. Used by CI, and runnable on the Pi or
   an Apple-Silicon Parallels VM (see macOS "Still open" **#4**).
2. **Native** — `installer/make-deb.sh` on an old-glibc box (how the Pi arm64 was
   made). It stamps the arch of the machine it runs on.

**`make-deb.sh` Depends fix (commit 684fd82):** the `ldd`→`dpkg -S` derivation
returned empty on usr-merged systems (every current Debian/Ubuntu) and silently
fell back to a short hardcoded list, dropping real deps (`libdbus-1-3`, the full
`libxcb-*` set, …). Paths are now `readlink -f`-canonicalised, so the full package
closure is derived.

Historical detail below.

### Original status (2026-08-24): amd64 + arm64 (Ubuntu) done

`make-deb.sh` **run and verified on x86_64 Ubuntu 26.04**: builds
`classbuilder_3.0_amd64.deb` (18 MB), installs under `/opt/classbuilder` with the
`/usr/bin` symlink + desktop entry + `.cbz` association + app icon, and the
in-app installer-dependent menus work once it runs installed (they show a clean
"not installed" notice from a build tree, which is correct). The shakedown found
two real bugs in the Depends derivation, both fixed (commit `b07df68`):

- **dpkg-divert lines.** `libc6` diverts `ld-linux` (usr-merge migration), so
  `dpkg -S` prints `diversion by libc6 from: /lib64/ld-linux…`; its `:` made the
  bogus "diversion by libc6 from" land in Depends. Filtered with
  `grep -v '^diversion '`. **Debian-wide, so the arm64/Pi build hits it too.**
- **`paste -sd', '`.** `-d` takes a *cyclic* delimiter list, so it alternated
  comma/space; dpkg rejects the space. Now `paste -sd,`.

**arm64 `.deb` — BUILT + attached (2026-08-24), on the Mac's arm64 Ubuntu 26.04
VM.** `make-deb.sh` ran clean there too → `classbuilder_3.0_arm64.deb` (18 MB),
installed + verified: `/opt/classbuilder`, `/usr/bin/classbuilder`, the desktop
entry, and `.cbz` double-click all correct. Attached to the v3.0 draft alongside
amd64. **Caveat (unchanged): this is a NEWER-glibc arm64 package** — it runs on
arm64 Ubuntu 26.04+, and is **not guaranteed on the Pi's Raspberry Pi OS** (older
glibc; a binary built against newer glibc does not run on an older one). So the
**only remaining Linux gap is a Pi-built arm64 `.deb`** (oldest glibc → runs on
both); build it on the Pi and replace/annotate the asset if Pi coverage is
wanted. See the arch table below. Both `.deb`s are attached to the v3.0 draft
release (see "Release" at the bottom).

Original authoring note (2026-08-14): script written on Windows and committed
before any Linux box was available, hence the shakedown above.

- Script: **`installer/make-deb.sh`** — the Linux counterpart of the `.iss` /
  `make-dmg.sh`, same payload. Build:
  `cmake --build --preset linux-release` (static Qt, PORTING_LINUX.md option B),
  then `./installer/make-deb.sh`
  → `installer/output/classbuilder_<ver>_<arch>.deb` (gitignored, regenerable).
  No root/fakeroot needed (`dpkg-deb --root-owner-group`).
- **Format = `.deb`** (not AppImage): all of JV's Linux boxes are Debian-family
  (2× Ubuntu + the Pi's Raspberry Pi OS), so a `.deb` gives the native menu
  entry + `.cbz` double-click association + clean `apt`/`dpkg` uninstall. With
  static Qt the `Depends` are just the desktop baseline (the script derives the
  real list from the binary via `ldd`+`dpkg -S`). AppImage stays a later option
  for non-Debian users. This is a private package under `/opt` — not
  Debian-archive grade (no changelog/copyright; lintian will warn; that's fine).
- **Layout** installed: `/opt/classbuilder/{ClassBuilder,doc,examples,runtime}`
  + a `/usr/bin/classbuilder` symlink. Kept together so Qt's
  `applicationDirPath()` (resolved via `/proc/self/exe`, even through the
  symlink) finds the extras beside the binary — exactly what the Help menu's
  `cbPayloadDir()` expects on non-Apple platforms. `.desktop` + shared-mime-info
  XML (`*.cbz`) + hicolor icons (`installer/linux/classbuilder{,-model}.png`,
  extracted from the `.ico`s on Windows and committed so the Linux build needs
  no icoutils/ImageMagick). `postinst`/`postrm` run
  `update-mime-database`/`update-desktop-database`/`gtk-update-icon-cache`.
- **"static" ≠ Windows single-exe.** On Linux, static Qt still links
  xcb/X11/GL/fontconfig/**glibc** against system libs — unavoidable. So glibc is
  the real portability axis (see below), and the `.deb` still `Depends` on the
  desktop baseline.

### Arch / glibc strategy — 2 builds, not 3

No cross-run on Linux, so build **natively per arch**. JV's fleet (per
`PORTING_LINUX.md`): x86_64 Ubuntu, arm64 Ubuntu (Parallels), **arm64**
Raspberry Pi OS (Pi 500+ — aarch64, *not* 32-bit armhf). So only **two** arch
packages:

| Package | Build on | Runs on |
|---|---|---|
| **amd64** | the x86_64 Ubuntu box | x86_64 Ubuntu (+ newer) |
| **arm64** | **the Pi** (Debian = oldest glibc) | the Pi **and** arm64 Ubuntu |

Build the arm64 `.deb` on the **oldest-glibc** arm64 box (the Pi's Debian, not a
newer Ubuntu): a binary built against older glibc runs on the newer box, not the
reverse. Build amd64 on the x86_64 box. No armhf build unless a 32-bit Pi OS
actually has to run CB.

## Release — v3.0 PUBLISHED on GitHub (updated 2026-08-25)

All platform installers go into **ONE GitHub release** so they stay together
under one version and out of the git history (the `.deb`/`.exe`/`.dmg` are
gitignored build artifacts, never committed).

**Current state (2026-08-25):** the release "ClassBuilder 3.0" (tag `v3.0`) is
**published**, with **6 assets**:

| Asset | Notes |
|---|---|
| `ClassBuilderSetup-3.0-x64.exe` | Windows x64, full-static |
| `ClassBuilder-3.0-mac-arm64.dmg` | macOS Apple Silicon |
| `classbuilder_3.0_amd64-glibc2.35.deb` | x86_64, glibc 2.35 (**CI**-built) — recommended amd64 |
| `classbuilder_3.0_arm64-glibc2.38.deb` | arm64, glibc 2.38 (**Pi**-built) — recommended arm64 |
| `classbuilder_3.0_amd64.deb` / `_arm64.deb` | earlier Ubuntu-26.04 (glibc 2.43) builds; superseded |

**Still to attach (both optional, on the Mac):** the **Intel `.dmg`**
`ClassBuilder-3.0-mac-x64.dmg` — now BUILT and verified (macOS "Still open" #2),
sitting in `installer/output/`, not yet uploaded:
`gh release upload v3.0 installer/output/ClassBuilder-3.0-mac-x64.dmg --clobber`
— and, if arm64 Ubuntu 22.04 / Debian 12 coverage is wanted, the **arm64
`-glibc2.35` `.deb`** via the Parallels VM container (macOS "Still open" #4).
Everything else is done.

_Historical (2026-08-24): the release started as a DRAFT with the first four
installers (the arm64 `.deb` then built on the Mac's Ubuntu 26.04 VM = newer
glibc). It has since been published and the Linux packages replaced with the
glibc-labelled per-arch builds above._

**Handoff — the plan (JV):**

1. **Windows `.exe`** and **macOS `.dmg`** are already **built and tested** — just
   attach the existing files to the draft (no rebuild needed).
2. **arm64 Linux `.deb`** — Pi not on hand, so build it on the **Mac's arm64
   Ubuntu VM** instead. That is fine, with one caveat: the VM has a **newer
   glibc** than the Pi, and a binary built against newer glibc does NOT run on an
   older one. So this `.deb` is an **"arm64 Ubuntu 26.04+ (newer glibc)"**
   package — it runs on arm64 Ubuntu (and newer), and is **not guaranteed on the
   Pi's Raspberry Pi OS**. If Pi coverage is wanted later, rebuild that one on the
   Pi (oldest glibc → runs on both) and replace/annotate the asset. Build:
   `cmake --build --preset linux-release` then `./installer/make-deb.sh`
   → `classbuilder_3.0_arm64.deb`. Check the exact minimum with
   `ldd --version` on the VM.
3. When all are attached and checked, **Publish** the draft — that creates the
   `v3.0` tag.

**How to attach:**
- `gh release upload v3.0 <file>` **works against the draft too** — verified
  2026-08-24 uploading `classbuilder_3.0_arm64.deb` from the Linux VM. (An earlier
  note here said it fails on a draft; that was wrong — `gh` resolves the draft by
  tag name even though the public tag URL 404s until Publish.)
- Web-UI alternative: repo → Releases → "ClassBuilder 3.0 (Draft)" → **Edit** →
  drag files into *Attach binaries*.

The draft's notes already list all four downloads (Windows `.exe`, macOS `.dmg`,
Linux amd64/arm64 `.deb`) with "added from …" markers on the missing ones —
adjust the filenames there to match the actual Windows/macOS output.
