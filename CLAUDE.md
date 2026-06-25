# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Visual Studio 2026 (originally VS2019; retargeted, no code changes) MFC + Qt project.

The **CMake build** (preferred) produces a **single collapsed `ClassBuilder.exe`** — all ~207 logic sources in [ClassBuilder/](ClassBuilder/) plus the former thin-app [ClassBuilder.cpp](ClassBuilder.cpp) linked into one executable. The old MFC-extension-DLL + thin-EXE split existed only for the (parked) add-in mechanism; collapsing it removes that boundary and `AFX_EXT_CLASS` as an export macro. The Qt dialog code is quarantined in a separate static lib, `ClassBuilderQt` (Qt forces `UNICODE`; this keeps it off the MultiByte MFC sources).

The old MSBuild solution (`ClassBuilderEXE.sln` + `.vcxproj`/`.dsp`/`.dsw`) built the original two products — `ClassBuilder.dll` (extension DLL) + `ClassBuilderEXE` (thin app) — but had **diverged** from CMake (which produces one collapsed EXE) and gone stale. It was **removed 2026-06-06**; CMake is now the only build. A full 2026-05-14 source snapshot is preserved at the sibling dir `..\ClassBuilderXX` if those legacy files are ever needed.

### CMake (preferred)

[CMakeLists.txt](CMakeLists.txt) + [CMakePresets.json](CMakePresets.json) at the repo root. Read by VS 2026 ("Open Folder"), VS Code (CMake Tools), and the command line — all share the `out/build/<preset>` tree. The primary target is **x64 Release**.

```
cmake --preset x64
cmake --build --preset x64-release      # or x64-debug
```

Key CMakeLists facts (each there for a root-caused reason — see auto-memory):
- MFC macros are defined **explicitly**, not via `CMAKE_MFC_FLAG` — that flag is honored only by the VS generator, and VS 2026's Open Folder uses Ninja. The collapsed EXE compiles with `_AFXDLL` (MFC as a shared DLL) + **`NODLL`**; `NODLL` makes `AFX_EXT_CLASS` expand to nothing and compiles out the add-in `LoadLibrary` loop in `InitInstance`.
- The zstd lib arch is derived from `CMAKE_CXX_COMPILER_ARCHITECTURE_ID` (generator-agnostic), not `CMAKE_VS_PLATFORM_NAME`.
- Debug uses `/Z7` (embedded debug info), not `/Zi` — avoids the shared-`vc145.pdb` `mspdbsrv` race (error C1090) under the ~200-file parallel build.
- Release carries **no** debug info by design (no `/DEBUG`, no `.pdb`); debugging happens on Debug builds.
- PCH on `stdafx.h` (the two flex lexer sources opt out) — incremental rebuilds ~3.5 s.

### MSBuild (legacy, removed 2026-06-06)

The original `ClassBuilderEXE.sln` (+ `.vcxproj`/`.dsp`/`.dsw`) was **removed** — it had diverged from CMake and gone stale. CMake is the only build now. Those files survive in the `..\ClassBuilderXX` 2026-05-14 snapshot if ever needed. (Historical, now in auto-memory: that MSBuild **Release** shipped ~1 MB of dead code because modern MSBuild defaults `GenerateDebugInformation` to true, silently disabling `/OPT:REF`; the CMake build doesn't have this.)

For TRACE output during debug, run with F5 in VS (Output → "Debug" pane). TRACE is compiled out in Release.

`libzstd_static.lib` is linked (CBZ compression) — the `/MD` (shared-CRT) variant under `ClassBuilder/zstd/lib/<arch>/`. The collapsed EXE is still `/MD` (shared MFC/CRT); the planned full-static build (`/MT` static CRT + static MFC + static Qt) will need a `/MT` variant of the zstd lib built.

There is no test suite. Verification is manual: open the project's own `.cbz` in CB, regenerate sources, rebuild, repeat (self-host).

## What this app does

ClassBuilder is a code-generation tool. The user defines an OO data model (classes, members, methods, relations, inheritance, diagrams) in the GUI; CB writes `.h` / `.cpp` source for that model. The generated code uses a runtime support layer (the `CB_*` headers in [Include/](Include/)) for owned containers, AVL trees, value trees, criticals, etc.

The app is **self-hosted**: the model that generates ClassBuilder's own source ships in the repo as [ClassBuilder/ClassBuilder_org.cbz](ClassBuilder/ClassBuilder_org.cbz). When you change CB's source by hand and CB is open on its own model, you'll be prompted to re-read the changed sources back into the model — accept the prompt so the edit is saved into the CBD when you save.

## Generated vs user code regions

Hand edits in CB-generated `.cpp` / `.h` are **only safe** inside:

- `//@START_USER...` / `//@END_USER...` — fully-editable user blocks.
- `{//@CODE_NNNN ... }//@CODE_NNNN` — method bodies (the body content is round-trippable to the model).
- `/*@NOTE_NNNN ... */` — note comments next to a method.

Anything outside those markers is regenerated and your edit is lost. In particular: **`Serialize` bodies, the `archive << _x` lines, are emitted by the codegen — never patch them on disk**, edit the model instead. Same for class declarations, member orderings, relation macros, etc.

## Serialization

The serialization runtime is **`CbArchive`** ([ClassBuilder/CbSerialize.h](ClassBuilder/CbSerialize.h), [CbArchive.cpp](ClassBuilder/CbArchive.cpp)) — MFC-free, with explicit class registration via `CB_DECLARE_SERIAL` / `CB_IMPLEMENT_SERIAL`. Wire format v1 has no per-version gates: every field always written, single version check on read. `.cbz` files are a raw Zstd frame around the CbArchive byte stream.

`CbObject` is the polymorphic root — a **bare class** (no `: public CObject`). DataModelDoc and other classes inherit from it directly; the MFC document framework integration lives in `CClassBuilderDoc` (which contains a `DataModelDoc` member, not is one).

The legacy MFC `CArchive` path has been **removed**: the `_mfcSerialize` model field is retained as `_mfcSerialize_notUsed` for CBZ format compatibility (it's still in the byte stream); all conditional codegen branches and the `MFC_*_SERIAL` escape-hatch macros are gone. A standalone [ClassBuilderStatic.exe](ClassBuilder/) build was produced once for portable `.cbd` → `.cbz` conversion and archived if a legacy file ever needs rescue; not part of the regular build.

Single inheritance is enforced for any class with `Serialize` enabled. The `Serialize` codegen iterates all bases at the top of the body, but the GUI rule prevents the cases (diamond, mixed ancestry) the codegen doesn't handle safely. See [project_classbuilder_serialize_single_inheritance.md](C:\Users\jimmy\.claude\projects\c--Users-jimmy-Projects-ClassBuilder\memory\project_classbuilder_serialize_single_inheritance.md) in auto-memory for the sharp edges.

## Pipe API

`ClassBuilder.exe` runs a JSON-over-named-pipe server at `\\.\pipe\ClassBuilder` ([CbCommandServer.cpp](ClassBuilder/CbCommandServer.cpp)). One JSON request per line, one reply per line. Reference and command list: [tools/PIPE_API.md](tools/PIPE_API.md). Helper PowerShell scripts in [tools/](tools/) drive bulk model edits, audit serialize/relations parity, run round-trip tests, etc. The intent is that the pipe API will also be used to drive port-related model migrations without modifying CB itself during the Qt port.

## Conventions

- **Prefer `Gti::IsX()` over MFC `IsKindOf(RUNTIME_CLASS(...))`** — `IsClass()` / `IsMethod()` / `IsArgument()` / `IsMember()` etc. are MFC-free, lighter compile dependencies, and Qt-port friendly.
- **Don't snapshot the linked-list head/tail in `CB_*_ACTIVE` cascade-delete loops** — the macros must re-query `GetFirst`/`GetLast` each iteration. Caching `_first`/`_last` and walking via `_prev`/`_next` is a use-after-free under cascade delete with VS2019+ stricter CRT.
- **Line endings are CRLF** on disk — CB stores method bodies with CRLF, regenerated source must emit CRLF. Audit `NL` usage when porting to Qt.
- **Toolbar bitmaps:** padding goes into the bitmap itself in `ScaleToolBar`, never via `SetSizes` (CToolBar adds extra height to the bottom otherwise).

## Phased port plan

The repo is mid-migration. The **strict order** is: VS2019 upgrade → VS2026 retarget (done 2026-04-27) → MFC→Qt port. Don't skip ahead. The current Windows MFC build is intended to remain the working tool throughout the Qt port, with port-related model migrations driven through the pipe API rather than by editing CB itself, to avoid the deadlock of changing the tool you depend on.

## Auto-memory

Substantial project-specific knowledge (incident history, deferred decisions, design notes) lives in the auto-memory system at `C:\Users\jimmy\.claude\projects\c--Users-jimmy-Projects-ClassBuilder\memory\`. The `MEMORY.md` index is loaded into every session — consult it for status, deferred items, and rationale that isn't in the source.
