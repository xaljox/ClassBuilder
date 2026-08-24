# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Originally an MFC app on Visual Studio 2019; retargeted to **VS 2026** 2026-04-27 (no code changes). **MFC was fully removed 2026-06-09** — this is now an **all-Qt** application: a Qt shell hosts an MFC-free model/logic core. There is no MFC, no `mfc*.dll` import, and no `AFX_EXT_CLASS` export boundary.

The **CMake build** produces a **single `ClassBuilder.exe`**. The source is organized into dependency tiers (restructured 2026-06-24 from the old add-in DLL/EXE-split folders):

- [include/](include/) — generic `CB_*` container / AVL-tree / value-tree / critical runtime headers (no deps)
- [value/](value/) — standalone value types (`CbColor` / `CbGeometry` = `CbRect`/`CbPoint`/`CbSize` / `CbString` / `CbTime`); **zero** serialization dependency
- [serialize/](serialize/) — `CbArchive` (`CbSerialize`) + `CbZstdStream`, the **optional** serialization layer; depends *down* on `value/`
- [src/model/](src/model/) — the MFC-free model + logic + generated classes + the self-host model `ClassBuilder.CBZ` + the tiny `WinMain.cpp` (→ `Cb_RunQtShell`) + the flex/bison `Read.l.cpp` / `Read.y.cpp`
- [src/qt/](src/qt/) — Qt views / dialogs / canvas, quarantined in a static lib `ClassBuilderQt` (Qt 6 forces `UNICODE` on consumers; the lib keeps that define off the MultiByte MFC-free model sources)
- [src/platform/](src/platform/) — `CbPlatformCompat` (the single `#ifdef` seam = the multi-platform pivot) + `CbMessageBox` / `CbShellHooks`
- [third_party/](third_party/) — `zstd` (CBZ compression) + `json` (command-server)
- [res/](res/) [models/](models/) [tools/](tools/) [docs/](docs/)
- [crossplatform/](crossplatform/) — Windows↔macOS/Linux port coordination: getting-started report, port punch-list, and required Qt patches (see [crossplatform/README.md](crossplatform/README.md))

(Historical: the EXE was once an MFC-extension-DLL + thin-EXE split that existed only for the parked add-in mechanism. The build collapsed to one EXE, MFC itself was then removed, and the folders were restructured into the tiers above. A full 2026-05-14 source snapshot is preserved at the sibling dir `..\ClassBuilderXX`, and the pre-restructure tree at `..\ClassBuilder_old`, if legacy files are ever needed. The old MSBuild solution — `ClassBuilderEXE.sln` + `.vcxproj`/`.dsp`/`.dsw` — was removed 2026-06-06; CMake is the only build.)

### CMake

[CMakeLists.txt](CMakeLists.txt) + [CMakePresets.json](CMakePresets.json) at the repo root. Read by VS 2026 ("Open Folder"), VS Code (CMake Tools), and the command line — all share the `out/build/<preset>` tree. The primary target is **x64 Release**.

```
cmake --preset x64
cmake --build --preset x64-release      # or x64-debug
```

The exe lands at `out/build/x64/bin/Release/ClassBuilder.exe`.

Key CMakeLists facts (each there for a root-caused reason — see auto-memory):
- The Qt static lib `ClassBuilderQt` links `Qt6::Widgets`/`Qt6::Network` **PRIVATE** so Qt 6's forced `UNICODE`/`_UNICODE` stays inside it; the EXE re-strips those with `/UUNICODE /U_UNICODE` (the model is a MultiByte build calling the ANSI Win32 APIs directly, and `cl.exe` gives `/U` precedence over `/D` for the same symbol).
- The zstd lib arch is derived from `CMAKE_CXX_COMPILER_ARCHITECTURE_ID` (generator-agnostic), with `CMAKE_VS_PLATFORM_NAME` preferred when the VS generator sets it.
- Debug uses `/Z7` (embedded debug info), not `/Zi` — avoids the shared-`vc145.pdb` `mspdbsrv` race (error C1090) under the ~300-file parallel build.
- Release carries **no** debug info by design (no `/DEBUG`, no `.pdb`); debugging happens on Debug builds.
- PCH on [src/model/StdAfx.h](src/model/StdAfx.h) (the flex lexer `Read.l.cpp` opts out) — incremental rebuilds ~3.5 s.
- Static Qt (`C:/Qt-static/6.11.1`) links the Qt libs + plugins into the EXE → no `Qt6*.dll` beside it, no windeployqt. With the Svg module present, SVG model icons are enabled.

For TRACE output during debug, run with F5 in VS (Output → "Debug" pane). TRACE is compiled out in Release.

`libzstd_static.lib` is linked (CBZ compression) — the `/MD` (shared-CRT) variant under [third_party/zstd/lib/](third_party/zstd/lib/)`<arch>/`. The EXE is currently `/MD` (shared CRT); the planned full-static build (`/MT` static CRT + static Qt) will use the `/MT` zstd variant already staged at `third_party/zstd/lib-mt/x64/`.

There is no test suite. Verification is manual: open the project's own `.cbz` in CB, regenerate sources, rebuild, repeat (self-host).

## What this app does

ClassBuilder is a code-generation tool. The user defines an OO data model (classes, members, methods, relations, inheritance, diagrams) in the GUI; CB writes `.h` / `.cpp` source for that model. The generated code uses a runtime support layer (the `CB_*` headers in [include/](include/)) for owned containers, AVL trees, value trees, criticals, etc.

The app is **self-hosted**: the model that generates ClassBuilder's own source ships in the repo as [src/model/ClassBuilder.CBZ](src/model/ClassBuilder.CBZ). When you change CB's source by hand and CB is open on its own model, you'll be prompted to re-read the changed sources back into the model — accept the prompt so the edit is saved into the model when you save.

## Generated vs user code regions

Hand edits in CB-generated `.cpp` / `.h` are **only safe** inside:

- `//@START_USER...` / `//@END_USER...` — fully-editable user blocks.
- `{//@CODE_NNNN ... }//@CODE_NNNN` — method bodies (the body content is round-trippable to the model).
- `/*@NOTE_NNNN ... */` — note comments next to a method.

Anything outside those markers is regenerated and your edit is lost. In particular: **`Serialize` bodies, the `archive << _x` lines, are emitted by the codegen — never patch them on disk**, edit the model instead. Same for class declarations, member orderings, relation macros, etc.

## Serialization

The serialization runtime is **`CbArchive`** ([serialize/CbSerialize.h](serialize/CbSerialize.h), [serialize/CbSerialize.cpp](serialize/CbSerialize.cpp)) — MFC-free, with explicit class registration via `CB_DECLARE_SERIAL` / `CB_IMPLEMENT_SERIAL`. Wire format v1 has no per-version gates: every field always written, single version check on read. `.cbz` files are a raw Zstd frame ([serialize/CbZstdStream.h](serialize/CbZstdStream.h)) around the CbArchive byte stream.

`CbObject` is the polymorphic root — a **bare class** (no `: public CObject`). DataModelDoc and other classes inherit from it directly; the framework integration lives in `CClassBuilderDoc` (which contains a `DataModelDoc` member, not is one).

The legacy MFC `CArchive` path has been **removed**: the `_mfcSerialize` model field is retained as `_mfcSerialize_notUsed` for CBZ format compatibility (it's still in the byte stream); all conditional codegen branches and the `MFC_*_SERIAL` escape-hatch macros are gone. A standalone `ClassBuilderStatic.exe` was produced once for portable `.cbd` → `.cbz` conversion and **archived** (not in this repo); it is not part of the regular build.

Single inheritance is enforced for any class with `Serialize` enabled. The `Serialize` codegen iterates all bases at the top of the body, but the GUI rule prevents the cases (diamond, mixed ancestry) the codegen doesn't handle safely. See [project_classbuilder_serialize_single_inheritance.md](C:\Users\jimmy\.claude\projects\c--Users-jimmy-Projects-ClassBuilder\memory\project_classbuilder_serialize_single_inheritance.md) in auto-memory for the sharp edges.

## Pipe API

`ClassBuilder.exe` runs a JSON-over-named-pipe server at `\\.\pipe\ClassBuilder` ([src/model/CbCommandServer.cpp](src/model/CbCommandServer.cpp)). One JSON request per line, one reply per line. Reference and command list: [tools/PIPE_API.md](tools/PIPE_API.md). Helper PowerShell scripts in [tools/](tools/) drive bulk model edits, audit serialize/relations parity, run round-trip tests, etc. The pipe API is also the intended way to drive port-related model migrations without modifying CB itself.

## Conventions

- **Prefer `Gti::IsX()` over MFC `IsKindOf(RUNTIME_CLASS(...))`** — `IsClass()` / `IsMethod()` / `IsArgument()` / `IsMember()` etc. are MFC-free, lighter compile dependencies, and Qt-port friendly.
- **Don't snapshot the linked-list head/tail in `CB_*_ACTIVE` cascade-delete loops** — the macros must re-query `GetFirst`/`GetLast` each iteration. Caching `_first`/`_last` and walking via `_prev`/`_next` is a use-after-free under cascade delete with VS2019+ stricter CRT.
- **Line endings are CRLF** on disk — CB stores method bodies with CRLF, regenerated source must emit CRLF. Audit `NL` usage when porting to other platforms.
- **Toolbar bitmaps:** padding goes into the bitmap itself in `ScaleToolBar`, never via `SetSizes` (the toolbar adds extra height to the bottom otherwise).
- **Window-system access only via `CbPlatformCompat`** ([src/platform/](src/platform/)) — never inline Win32/Qt; the port is `#ifdef` branches inside that seam.
- **Prefer `bool` over `BOOL`** in new/edited hand-written code (don't mass-rewrite the model's `BOOL`).

## Port status

Historical migration order: VS2019 upgrade → VS2026 retarget (done 2026-04-27) → MFC→Qt port (**done 2026-06-09**, zero `mfc*.dll` imports). The app is now all-Qt, the folders are restructured for multi-platform, and the repo is in Git (**public, MIT-licensed** since 2026-08-24 — `github.com/xaljox/ClassBuilder`; ClassBuilder was originally open source). The **next** phase is the cross-platform (Mac/Linux) build: the committed generated sources build on other platforms (CB can't yet *run* there to regenerate), pivoting on the single `CbPlatformCompat` `#ifdef` seam. Port-related model migrations are driven through the pipe API rather than by editing CB itself, to avoid the deadlock of changing the tool you depend on.

## Auto-memory

Substantial project-specific knowledge (incident history, deferred decisions, design notes) lives in the auto-memory system at `C:\Users\jimmy\.claude\projects\c--Users-jimmy-Projects-ClassBuilder\memory\`. The `MEMORY.md` index is loaded into every session — consult it for status, deferred items, and rationale that isn't in the source.

**Auto-memory is per-machine — it does not sync between Windows/macOS/Linux.** Notes that must be shared across platforms go in the tracked repo instead (they travel in git). Claude Code config shared across platforms — the tracked-vs-local `.claude/` permission split and each platform's build/launch path — is documented in [crossplatform/CLAUDE_CODE_SETUP.md](crossplatform/CLAUDE_CODE_SETUP.md); read it before touching `.claude/` permissions.
