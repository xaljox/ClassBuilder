# Building ClassBuilder

Build setup, per-platform prerequisites, and the gotchas that are easy to
rediscover the hard way. For what the app *does* and the model/serialization
design, see [CLAUDE.md](CLAUDE.md); for the port and the installers, see
[crossplatform/](crossplatform/) and [installer/](installer/).

> **CMake is the only build.** The old MSBuild solution (`ClassBuilderEXE.sln`
> + `.vcxproj`) was removed in 2026-06, together with the MFC-extension-DLL /
> thin-EXE split. There is one target: a single self-contained `ClassBuilder`
> executable.

---

## Quick start

One configure preset per platform, read by VS 2026 ("Open Folder"), VS Code
(CMake Tools) and the command line — all sharing the `out/build/<preset>` tree.

```sh
cmake --preset x64       && cmake --build --preset x64-release      # Windows
cmake --preset mac       && cmake --build --preset mac-release      # macOS
cmake --preset linux-x64 && cmake --build --preset linux-release    # Linux
```

Swap `-release` for `-debug` for a debug build.

| Preset | Binary lands at |
|---|---|
| `x64` | `out/build/x64/bin/Release/ClassBuilder.exe` |
| `mac` | `out/build/mac/bin/Release/ClassBuilder.app` |
| `linux-x64` | `out/build/linux-x64/bin/Release/ClassBuilder` |

There is **no test suite**. Verification is manual and self-hosted: open the
project's own `src/model/ClassBuilder.CBZ` in CB, regenerate, rebuild, repeat.

## Prerequisites

| | Windows | macOS | Linux |
|---|---|---|---|
| Compiler | MSVC (VS 2026, v145) | Xcode Command Line Tools (clang) | GCC or clang |
| Build tools | bundled with VS | `brew install cmake ninja` (**arm64** brew) | `apt install cmake ninja-build` |
| Qt 6 | static, see below | `brew install qt` or from source | `apt install qt6-base-dev qt6-svg-dev` or from source |
| zstd | vendored in-tree | `brew install zstd` | `apt install libzstd-dev` |

C++17 (`cxx_std_17`). flex/bison are only needed if you regenerate
`Read.l`/`Read.y`; the generated `.cpp` are committed.

### Qt

The committed presets point at:

| Preset | `CMAKE_PREFIX_PATH` |
|---|---|
| `x64` | `C:/Qt-static-mt/6.11.1` — static Qt built against the **static CRT** |
| `mac` | `/opt/homebrew/opt/qt` — brew, shared |
| `linux-x64` | `$HOME/Qt-6.11.1-static` |

Those are **local paths, not something the repo can provide.** Building the
from-source Qt is documented per platform in
[crossplatform/PORTING_MAC.md](crossplatform/PORTING_MAC.md) and
[crossplatform/PORTING_LINUX.md](crossplatform/PORTING_LINUX.md) (option B).
Machine-specific overrides belong in a **gitignored `CMakeUserPresets.json`**
that inherits a committed preset and repoints `CMAKE_PREFIX_PATH` — never edit
the committed presets for one machine.

Static Qt is detected automatically (via `Qt6::Core`'s TYPE); the platform and
SVG plugins are then linked into the executable, so there is no `windeployqt` /
`macdeployqt` step and no `Qt6*.dll` beside the binary. With the Svg module
present, SVG model icons are enabled.

## Things in CMakeLists.txt that look odd but are load-bearing

Each of these was root-caused; changing it reintroduces a real bug.

- **The UNICODE quarantine (Windows).** Qt 6 forces `UNICODE`/`_UNICODE` on
  anything linking it, but the model sources are a **MultiByte** build calling
  the ANSI Win32 APIs. So the Qt code lives in a static lib `ClassBuilderQt`
  that links Qt **PRIVATE**, and the EXE re-strips the defines with
  `/UUNICODE /U_UNICODE` (`cl.exe` gives `/U` precedence over `/D`). Windows-only
  — Qt does not force it elsewhere.
- **`/Z7`, not `/Zi`, for Debug.** Embedded debug info avoids the shared
  `vc145.pdb` `mspdbsrv` race (error C1090) under the ~300-file parallel build.
- **Release carries no debug info by design** — no `/DEBUG`, no `.pdb`. Debug
  on Debug builds.
- **`/MT` static CRT** (with the matching `third_party/zstd/lib-mt/` zstd), so
  the shipped `.exe` needs no VC++ redistributable.
- **PCH on [src/model/StdAfx.h](src/model/StdAfx.h)**; the flex lexer
  `Read.l.cpp` opts out (`SKIP_PRECOMPILE_HEADERS`) because it does not include
  it. Incremental rebuilds land around 3.5 s.
- **macOS `CMAKE_OSX_DEPLOYMENT_TARGET=13.0`.** Setting it is not enough on its
  own — Qt and zstd must be built for the same target or the binary is *labelled*
  13 while containing newer objects. Full explanation in
  [crossplatform/INSTALLER.md](crossplatform/INSTALLER.md).

## zstd

Vendored under [third_party/zstd/](third_party/zstd/): headers plus prebuilt
**Windows** static libs (`lib/` for `/MD`, `lib-mt/` for `/MT`). Nothing to
install on a fresh Windows checkout.

The committed `.lib` files are Windows-only and useless elsewhere; macOS and
Linux `find_library` a static `libzstd.a` (brew / `libzstd-dev`), with
`CMAKE_FIND_LIBRARY_SUFFIXES` forced to `.a` so it cannot pick up a `.dylib`/
`.so`. Override with `-DZSTD_LIB=/path/to/libzstd.a` when you need a specific
one — that is how the macOS ship build pins a deployment-target-13 zstd.

**Do not replace the vendored Windows libs with the ones from zstd's GitHub
release zip.** Those are MinGW-built: they pull in libgcc helpers
(`___chkstk_ms`, `___udivdi3`) and carry no SAFESEH metadata, both fatal for
MSVC linking. Build from source instead (`build/VS2010/libzstd/libzstd.vcxproj`,
with `<RuntimeLibrary>` set to match).

## The `_index` union (CopyShape pointer alias)

[src/model/DataModelDocObject.h](src/model/DataModelDocObject.h) declares:

```cpp
public:
    union {
        int      _index;     // transient object id during archive save/load
        intptr_t _ptrIndex;  // transient pointer alias during CopyShape
    };
```

Intentional. The two uses are temporally disjoint: serialization assigns
sequential ids (always 32-bit), while CopyShape stuffs a pointer to each shape's
new copy in there so nested shapes can find their new parent — which does not fit
an `int` on 64-bit. The union keeps the footprint at the size of the larger
member.

The codegen emits it from a literal in
[src/model/Class.cpp:2209](src/model/Class.cpp#L2209). The `_ptrIndex` read/write
sites in `*Shape.cpp` live inside `//@CODE_NNNN` markers, so they round-trip
through the model.

## Regenerating from the model — the chicken-and-egg

CB generates its own source, so an **older** `ClassBuilder` run against a
**newer** `.cbz` can produce a tree that will not build: the parts round-tripped
from the model come out new, while anything emitted by the old binary's
compiled-in codegen comes out old.

**On a fresh machine, just build from the committed sources.** They are already
consistent — do not open the model and regenerate until you have a CB built from
them. If you do get caught: build → run the *new* CB on the same `.cbz` →
regenerate again → rebuild.

This is also why the generated `.cpp`/`.h` are tracked in git rather than
ignored: macOS and Linux build what Windows generated (see
[crossplatform/README.md](crossplatform/README.md)).

## Line endings

`.gitattributes` pins **CRLF in the working tree on every platform**. CB's
codegen emits CRLF (`NL == "\015\012"`) and round-trips its own sources as CRLF,
so a Windows regen would otherwise show up as whole-file EOL churn. clang and GCC
compile CRLF fine. Do not "fix" it to LF.

## When a build breaks

| Symptom | Cause | Fix |
|---|---|---|
| `Static libzstd.a not found` | no system zstd | `brew install zstd` / `apt install libzstd-dev`, or pass `-DZSTD_LIB=` |
| `Qt6Config.cmake not found` | `CMAKE_PREFIX_PATH` points nowhere | fix it in a **`CMakeUserPresets.json`**, not the committed preset |
| Qt found, but the *wrong* one after repointing the prefix | CMake caches `Qt6Svg_DIR` and friends | reconfigure with `--fresh` |
| `ld: warning: object file was built for newer 'macOS' version` | Qt/zstd built for a newer target than CB | rebuild those deps at the same deployment target — see INSTALLER.md |
| unresolved `__imp_*` / `UNICODE`-flavoured symbol mismatches (Windows) | the UNICODE quarantine was broken | keep Qt **PRIVATE** on `ClassBuilderQt`, keep `/UUNICODE /U_UNICODE` on the EXE |
| `C1090` / `mspdbsrv` errors (Windows Debug) | `/Zi` instead of `/Z7` | leave `CMAKE_MSVC_DEBUG_INFORMATION_FORMAT` alone |
| `LNK2026: module unsafe for SAFESEH` or missing `___chkstk_ms` | a MinGW-built zstd got in | restore the in-tree MSVC-built lib |
| `*Shape.cpp` cannot find `_ptrIndex` | regenerated with an older CB | see the chicken-and-egg section above |

## Not missing — do not go looking

- No `vcpkg`/Conan: Windows dependencies are vendored, the others come from the
  system package manager.
- No CI and no test suite; verification is the self-host loop.
- No `.sln`/`.vcxproj`, no `Release\<Platform>\` output tree, no MFC.
