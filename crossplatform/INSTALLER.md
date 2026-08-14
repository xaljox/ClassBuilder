# ClassBuilder installers / packaging — cross-platform coordination

Auto-memory does **not** sync between machines, so this tracked file is the
handoff for per-platform installer work. **Windows is done; macOS (and Linux)
are the open items.** Read this together with `PORTING_MAC.md` /
`PORTING_LINUX.md`.

App version: **3.0** — a new major release (the original ClassBuilder ~25 years
ago was 2.x). Keep it in sync across all installers.

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

## macOS — TODO (much is already in place)

**Already done** — see `PORTING_MAC.md` → *"Static Qt on macOS — DONE"* and the
2026-06-25/26 commits:
- **Self-contained static binary**: `mac-static` preset (in the *local*
  `CMakeUserPresets.json`, which is gitignored — recreate it on the Mac) + static
  patched Qt at `~/Qt-6.11.1-patched-static` → a **24 MB single binary**;
  `otool -L` shows only system libs, no Qt frameworks, `macdeployqt` not needed.
  zstd already static.
- **`.cbz` file association** — already fixed + committed (Info.plist document
  types), so double-click already works once the app is installed.

**Remaining for a Mac installer:**
1. Wrap the binary in a proper **`ClassBuilder.app`** bundle — Info.plist with
   `CFBundleShortVersionString = 3.0`, the (already-done) `.cbz` document
   type/UTI, and an app icon (`res/ClassBuilder.ico` → convert to `.icns`).
2. Bundle the shared extras (into the `.app`'s `Resources/`, and/or beside it in
   the `.dmg`): the committed **`docs/manual/ClassBuilder_Manual.pdf`**,
   **`models/manual/Matrix.CBZ`**, and the **compile-runtime** (`include/`
   `value/` `serialize/` + zstd header — Mac users usually `brew install zstd`
   for the lib, or bundle a static `libzstd.a`).
3. Package as a **`.dmg`** (drag `ClassBuilder.app` → `/Applications`, the
   standard macOS install) via `hdiutil`. Today it ships as an ad-hoc-signed
   `ditto` zip (~9 MB).
4. **Signing / notarization:** currently ad-hoc `codesign --deep --sign -`,
   unsigned → Gatekeeper needs right-click→Open / strip quarantine. Real
   distribution wants a Developer ID signature + `notarytool` notarization.
   arm64-only so far.

## Linux — TODO

Static Qt at `~/Qt-6.11.1-static` (the `linux-x64` preset) → a self-contained
binary is the goal. Package as an **AppImage** (run-anywhere, self-contained) or
a `.deb`; bundle the same extras (PDF + example + compile-runtime). Handle the
`.desktop` file + MIME type for the `.cbz` association. See `PORTING_LINUX.md`.
