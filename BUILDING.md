# Building ClassBuilder

This file captures build setup, gotchas, and the cross-machine handoff procedure so
they don't have to be rediscovered. For what the app does, the model, and the
serialization runtime, see [CLAUDE.md](CLAUDE.md). For the agent's persistent
context (deferred decisions, incident history) see the user's auto-memory dir
referenced in CLAUDE.md.

---

## Quick start

Open [ClassBuilderEXE.sln](ClassBuilderEXE.sln) in Visual Studio 2022 / 2026
(MSBuild v145 toolset). Pick a platform + config and **F5** / **Ctrl+F5**.

| Platform | Native on | Notes |
| --- | --- | --- |
| `x86` (Win32) | x86 / x64 / ARM64 hosts (Windows emulation) | Legacy 32-bit, builds cleanly everywhere |
| `x64` | x64 / ARM64 (x64 emulation on ARM64 Windows) | Default for new desktop work |
| `ARM64` | ARM64 hosts (Apple Silicon Parallels, Surface Pro X, etc.) | See ARM64 caveat below |

Output goes to `Release\<Platform>\ClassBuilder.exe` (+ `.dll`) or
`Debug\<Platform>\` — see [output layout](#output-layout).

### Solution platform names

The .sln uses **`x86`** as the 32-bit platform name; both vcxproj files use
**`Win32`**. The .sln maps `x86 → Win32` per-project. If you invoke MSBuild
directly, pass `/p:Platform=x86` against the .sln (not `Win32`):

```
msbuild ClassBuilderEXE.sln /p:Configuration=Release /p:Platform=x86
```

For the per-project vcxproj invocation, use `Win32`:

```
msbuild ClassBuilder\ClassBuilder.vcxproj /p:Configuration=Release /p:Platform=Win32
```

---

## Output layout

Both vcxprojs use `<Config>\<Platform>\` (configuration-first), repo-relative:

```
<repo-root>/
├── Release/
│   ├── Win32/   ClassBuilder.exe + ClassBuilder.dll  (x86 final)
│   ├── x64/     ClassBuilder.exe + ClassBuilder.dll  (x64 final)
│   └── ARM64/   ClassBuilder.exe + ClassBuilder.dll  (ARM64 final)
├── Debug/
│   └── ...same structure as Release
└── ClassBuilder/
    ├── Release/
    │   ├── Win32/   DLL intermediates: .obj, .lib, .pch, .tlog
    │   ├── x64/     ...
    │   └── ARM64/   ...
    └── Debug/
        └── ...
```

**Why it matters:** if you ever see an `MSB8012: TargetPath ... does not match the
Linker's OutputFile property value` warning, it means an `<OutDir>` got dropped
for one of the 6 (Configuration|Platform) PropertyGroups and MSBuild fell back to
its default `$(Platform)\$(Configuration)\` layout — which is the opposite order.
Add an `<OutDir>.\Release\$(Platform)\</OutDir>` (and `IntDir` match) to the
affected PropertyGroup. There are 12 entries total — 6 in each vcxproj.

**Important:** F5 in VS reads `TargetPath` (= `OutDir + TargetName + TargetExt`),
not `Linker.OutputFile`. If they diverge, the linker writes to one place and F5
looks somewhere else and fails with *"cannot find the file specified"*. So always
keep `OutDir` and `Linker.OutputFile` pointing to the same directory.

---

## zstd dependency

zstd 1.5.7 is **vendored in the repo** under [ClassBuilder/zstd/](ClassBuilder/zstd/):

```
ClassBuilder/zstd/
├── include/   (zstd.h, zdict.h, zstd_errors.h)
├── lib/
│   ├── Win32/  libzstd_static.lib  (x86, MSVC, /MD)
│   ├── x64/    libzstd_static.lib  (x64, MSVC, /MD)
│   └── ARM64/  libzstd_static.lib  (ARM64, MSVC, /MD)
└── README.upstream.md
```

All three platforms get a vendored static lib. **Nothing to install or
pre-build on a fresh checkout.** The vcxproj points at
`zstd\include` (relative) and `zstd\lib\$(Platform)`, no per-machine paths.

### Why we built from source (rather than using the GitHub prebuilt)

The official `zstd-v1.5.7-win32.zip` ships a `static/libzstd_static.lib` that is
**MinGW-built** — it depends on libgcc helpers (`___chkstk_ms`, `___udivdi3`) and
has no SAFESEH metadata. Both are fatal for MSVC linking. Don't replace any of
these vendored libs with files from the GitHub release zip — that will reintroduce
both problems.

### If you ever need to rebuild zstd from source

Use the bundled VS solution in zstd's source tree (`build/VS2010/libzstd/libzstd.vcxproj`):

1. Patch `<RuntimeLibrary>MultiThreaded</RuntimeLibrary>` → `MultiThreadedDLL`
   (/MT → /MD) in the Release ItemDefinitionGroups, so it links into our /MD MFC
   extension DLL without a CRT conflict.
2. For ARM64: clone the Win32 `ProjectConfiguration` + `PropertyGroup` +
   `ImportGroup` + `ItemDefinitionGroup` blocks and rename to ARM64
   (upstream's solution only ships Win32/x64).
3. `msbuild libzstd.vcxproj /p:Configuration=Release /p:Platform=<plat> /p:PlatformToolset=v145`
4. Copy `bin/<Platform>_Release/libzstd_static.lib` → `ClassBuilder/zstd/lib/<Platform>/`.

### Debug/Release CRT mismatch (silenced)

The vendored libs are all `/MD` (Release CRT). Debug builds of ClassBuilder use
`/MDd`, so you'd normally see `LNK4098: defaultlib 'MSVCRT' conflicts with use of
other libs: use /NODEFAULTLIB:library`. The DLL project's Debug Link sections
include `<AdditionalOptions>/ignore:4098 %(AdditionalOptions)</AdditionalOptions>`
to silence it. Safe in practice because zstd doesn't pass heap-owned memory
across the API boundary in CB's usage. If you want a fully clean Debug link,
build a `/MDd` variant of zstd and place it at e.g. `zstd/lib/<Platform>/Debug/`,
then add `<AdditionalLibraryDirectories>zstd\lib\$(Platform)\Debug;...` to the
Debug Link sections. Not required for working Debug builds.

### SAFESEH (Win32 only)

The DLL project's Link sections all have
`<ImageHasSafeExceptionHandlers>false</ImageHasSafeExceptionHandlers>`. SAFESEH
is x86-only; this disables it because the prebuilt zstd object files (back when
we tried the GitHub Win32 zip) lacked SAFESEH metadata. Vendored libs we build
ourselves would have it, but leave the project setting alone — it's harmless on
ARM64/x64 and adds no exposure on x86 for a desktop dev tool.

---

## Precompiled header (PCH) gotchas

`StdAfx.cpp` has per-platform conditional `<PrecompiledHeader>Create</PrecompiledHeader>`
entries in [ClassBuilder/ClassBuilder.vcxproj](ClassBuilder/ClassBuilder.vcxproj).
If you add a new platform configuration, **you must add a matching Create entry**
or every other source file fails with `C1083: Cannot open precompiled header
file: '.\Release\ClassBuilder.pch': No such file or directory` (because nothing
in the build is set to *create* the PCH).

`Read.l.cpp` and `Rtf.l.cpp` are flex-generated C files that don't include
`stdafx.h`. They have per-platform empty-`<PrecompiledHeader></PrecompiledHeader>`
overrides to opt out of the project-level "Use" default. New platforms need
matching entries here too or these two files fail with `C1010: unexpected end of
file while looking for precompiled header`.

---

## ARM64 caveat: PreferredToolArchitecture

When building ARM64 on an ARM64 host (Apple Silicon under Parallels, Surface Pro
X, etc.), the **ARM64-native MSVC compiler** runs out of virtual memory for
MFC's PCH and dies with:

```
fatal error C1076: compiler limit: internal heap limit reached
fatal error C3859: Failed to create virtual memory for PCH
```

Fix: use the **x64-hosted ARM64 cross-compiler** (which has more vmem headroom).
There's a top-level `PropertyGroup` near the top of
[ClassBuilder/ClassBuilder.vcxproj](ClassBuilder/ClassBuilder.vcxproj) that sets
this:

```xml
<PropertyGroup Condition="'$(Platform)'=='ARM64'">
  <PreferredToolArchitecture>x64</PreferredToolArchitecture>
</PropertyGroup>
```

**Known quirk:** as of VS 2026 / v145, this PropertyGroup placement doesn't
always take effect from the IDE. If you hit the C3859/C1076 wall, override on the
command line:

```
msbuild ClassBuilderEXE.sln /p:Configuration=Release /p:Platform=ARM64 /p:PreferredToolArchitecture=x64
```

If you're building ARM64 from VS and the property is being ignored, the
workaround is to set the environment variable `PreferredToolArchitecture=x64`
before launching VS, or build from a Developer Command Prompt with the flag
above.

This is irrelevant on Intel/AMD x64 hosts — the x64 compiler is the native one
and has no memory issue. Only matters on ARM64 *hosts* targeting ARM64.

---

## The `_index` union (CopyShape pointer alias)

[ClassBuilder/DataModelDocObject.h](ClassBuilder/DataModelDocObject.h) declares
`_index` as a union:

```cpp
public:
    union {
        int      _index;     // transient object-mapping ID during archive save/load
        LONG_PTR _ptrIndex;  // transient pointer alias during CopyShape (64-bit safe)
    };
```

This is intentional. `_index` is used for two non-overlapping purposes:

1. **Serialization:** during archive save/load, the framework assigns sequential
   integer IDs to objects. Always fits in 32 bits.
2. **CopyShape:** when deep-copying a diagram, each shape stuffs a pointer to its
   "new copy" into `_index` so nested shapes can find the new parent. On x86 a
   pointer fits in an int; on x64/ARM64 it doesn't, hence `_ptrIndex` (8 bytes).

The two uses are temporally disjoint. The union keeps the object footprint
minimal (4 bytes on Win32, 8 bytes on 64-bit — same size as the largest member).

The codegen emits this union for every class flagged as a "document object" via
the literal in [ClassBuilder/Class.cpp:2202](ClassBuilder/Class.cpp#L2202). The
13 `_ptrIndex = LONG_PTR(...)` write sites in `*Shape.cpp` and the 7
`(SomeShape*)x->_ptrIndex` read sites in CopyShape live inside
`//@CODE_NNNN` markers, so they round-trip through the model.

**On a fresh machine, do NOT regenerate sources from the model before reading
[Cross-machine bootstrap](#cross-machine-bootstrap) below — there's a one-pass
chicken-and-egg.**

---

## Cross-machine bootstrap

When moving the repo (and the .cbz model) to a new machine:

1. **Sync the whole repo.** All vcxproj/sln/zstd/source edits travel via git or
   file copy. The vendored zstd libs are in the tree.
2. **Build CB normally for x86 first** (`x86 Release`). It's the most stable
   target and doesn't need ARM64 toolchain bits.
3. **Then build x64 / ARM64** as needed.

### The .cbz model regeneration chicken-and-egg

If the other machine has an **older CB.exe** whose codegen still emits
`int _index;` (instead of the union), and you run that old CB on the *new* .cbz:

- CB regens `Class.cpp` with the new union-emitting literal (round-tripped from
  the .cbz). Good.
- CB regens `DataModelDocObject.h` using its compiled-in **old** codegen → emits
  `int _index;` (no union). **Bad.**
- Subsequent build fails because `*Shape.cpp` references `_ptrIndex` which
  doesn't exist in the regenerated header.

Three ways to break the cycle on the new machine:

1. **Skip the first regen.** Don't open the .cbz in CB until you've already
   built a CB with the new codegen. Just build directly from the synced sources
   (they already have the union edits, no regen needed).
2. **Build-twice.** Open .cbz in old CB → regen → build → run *new* CB on the
   same .cbz → regen again → final build.
3. **One-shot hand-patch.** After the first old-CB regen, manually edit
   `DataModelDocObject.h:46` to insert the union (the form shown above). Then
   build. The next regen with the new CB produces identical content.

Easiest: option (1). Just don't regen until you have to.

---

## What to do if a build breaks

| Symptom | Probable cause | Fix |
| --- | --- | --- |
| `C1083: Cannot open include file: 'zstd.h'` | Include path lost in vcxproj | Check `<AdditionalIncludeDirectories>` includes `zstd\include` |
| `LNK1181: cannot open input file 'libzstd_static.lib'` | Lib path or platform mismatch | Check `<AdditionalLibraryDirectories>` includes `zstd\lib\$(Platform)`; verify `zstd/lib/<plat>/libzstd_static.lib` exists |
| `LNK2026: module unsafe for SAFESEH image` | Vendored lib lacks SAFESEH | Check `<ImageHasSafeExceptionHandlers>false</ImageHasSafeExceptionHandlers>` in Link section, or use the in-tree MSVC-built lib (not a GitHub prebuilt) |
| `LNK2019: unresolved external symbol ___chkstk_ms` | Linking a MinGW-built zstd | Replace with the MSVC-built lib from `ClassBuilder/zstd/lib/<plat>/` (do not use the GitHub release zip's static lib) |
| `C1853: precompiled header file is from a different version` | Stale PCH from another platform | Delete `ClassBuilder\Release\ClassBuilder.pch` (or do a clean rebuild) |
| `C1083: Cannot open precompiled header file ... No such file` | Missing `<PrecompiledHeader>Create</PrecompiledHeader>` entry for StdAfx.cpp on this platform | Add the per-platform Create entry in the ClCompile block for StdAfx.cpp |
| `C1076 / C3859: virtual memory for PCH` | ARM64-native compiler on ARM64 host | Pass `/p:PreferredToolArchitecture=x64` to msbuild |
| `LNK4098: defaultlib 'MSVCRT' conflicts...` | /MD-vs-/MDd CRT mismatch | Already silenced via `/ignore:4098` for Debug; if you need a clean build, build a /MDd zstd variant |
| `MSB8012: TargetPath ... does not match Linker's OutputFile` | Missing `<OutDir>` for one PropertyGroup | Add `<OutDir>.\<Config>\$(Platform)\</OutDir>` to the missing config |
| `F5 / Ctrl+F5: cannot find the file specified` | Same as MSB8012 — TargetPath and OutputFile diverge | Same fix; also confirm a build completed after fixing |

---

## What this tree is **not** missing

So you don't go looking for stuff that isn't there:

- No `Release\ClassBuilder.exe` at the repo root anymore. The legacy x86 layout
  (everything dumping into `Release\` flat) was replaced with `Release\<Platform>\`.
- No `vcpkg` or `Conan` integration. Dependencies are vendored.
- No CI. Verification is manual: open `ClassBuilder/ClassBuilder_org.cbz` in CB,
  regenerate, rebuild, repeat (self-host).
- No test suite. (Yet.)
