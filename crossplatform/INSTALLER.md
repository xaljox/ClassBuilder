# ClassBuilder installers / packaging — cross-platform coordination

Auto-memory does **not** sync between machines, so this tracked file is the
handoff for per-platform installer work. **Windows and macOS are done; Linux is
the open item.** Read this together with `PORTING_MAC.md` / `PORTING_LINUX.md`.

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
2. **arm64 only.** No Intel or universal binary. Needs a second Qt build
   (x86_64) plus `lipo`, so it is a real chunk of work — do it only if an Intel
   Mac actually has to run CB.
3. **Bundle-ID collision when testing:** build-tree copies and the
   `/Applications` copy all use `com.xaljox.ClassBuilder`, so Launch Services
   points `.cbz` at whichever was registered last. After testing a build-tree
   app, re-assert the installed one with
   `lsregister -f /Applications/ClassBuilder.app`.

## Linux — TODO

Static Qt at `~/Qt-6.11.1-static` (the `linux-x64` preset) → a self-contained
binary is the goal. Package as an **AppImage** (run-anywhere, self-contained) or
a `.deb`; bundle the same extras (PDF + example + compile-runtime). Handle the
`.desktop` file + MIME type for the `.cbz` association. See `PORTING_LINUX.md`.
