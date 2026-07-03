# Claude Code cross-platform setup — shared notes

Shared source of truth for how Claude Code is configured to work on this repo
across **Windows, macOS, and Linux**. This file travels in git, so all three
platforms read the same picture.

> **Why this file exists:** Claude Code's per-machine auto-memory
> (`~/.claude/projects/.../memory/`) does **not** sync between machines. Anything
> that must be shared across platforms lives here, in the tracked repo, instead.

## Permission split — the core idea

The goal is a **prompt-free normal loop** on every platform:
*edit → commit → push* on one box, *pull → build → (maybe edit) → commit → push*
on another. Permissions are split by what is portable:

| Scope | File | Synced? | Holds |
|-------|------|---------|-------|
| **Portable** | `.claude/settings.json` | **Tracked (git)** | Everything identical on all platforms |
| **Per-machine** | `.claude/settings.local.json` | Gitignored | Only the built-binary launch path (differs per OS) |

### Tracked `.claude/settings.json` (shared, identical everywhere)

- `defaultMode: acceptEdits` — file edits never prompt.
- git: `status pull fetch push add commit diff log show branch checkout switch stash restore remote rev-parse`
- build/tools: `cmake ninja ctest otool`
- fs helpers: `cp mkdir touch pkill`
- **Destructive git stays gated on purpose** — `reset --hard`, `clean -fd`,
  `rebase`, `push --force` still prompt. They are not part of the "normal" loop.

### Per-machine `.claude/settings.local.json` (launch path only)

Each platform needs its own launch entry merged into `permissions.allow`
(union with what's already there — don't overwrite):

| Platform | Preset / build dir | Launch path (`Bash(...:*)`) |
|----------|--------------------|-----------------------------|
| **Windows** | `x64` → `out/build/x64` | `./out/build/x64/bin/Debug\|Release/ClassBuilder.exe` |
| **macOS** | `mac-patched` → `out/build/mac-patched` | `./out/build/mac-patched/bin/Debug\|Release/ClassBuilder.app/Contents/MacOS/ClassBuilder` |
| **Linux** | `linux-x64` → `out/build/linux-x64` | `./out/build/linux-x64/bin/Debug\|Release/ClassBuilder` |

## Platform specifics worth knowing

- **macOS `.app` bundle** — the real executable is nested at
  `.../ClassBuilder.app/Contents/MacOS/ClassBuilder`, deeper than the plain
  Windows/Linux path.
- **macOS `mac-patched` preset is gitignored** — it is **not** in the tracked
  `CMakePresets.json` (which only has `mac`/`mac-debug`/`mac-release`, using brew
  Qt). It lives in a gitignored `CMakeUserPresets.json` override that repoints
  `CMAKE_PREFIX_PATH` to a local patched Qt 6.11.1 at `~/Qt-6.11.1-patched` (dock
  tear-off fix + Wayland — see [QT_DOCK_TEAROFF_PATCH.md](QT_DOCK_TEAROFF_PATCH.md)).
  Both halves of the override are gitignored together (the preset **and** its
  launch path in `settings.local.json`), so there's no tracked-vs-local conflict.
  **Temporary** until brew ships Qt ≥ 6.11.2; then swap back to the committed
  `mac` preset (`out/build/mac`) and drop the override + its local launch path.
- **Linux** builds against the same patched Qt 6.11.1 at `~/Qt-6.11.1-patched`
  (dock tear-off fix + Wayland); the `linux-x64` build dir caches that prefix.

## Adding a new platform

1. `git pull` — the tracked `settings.json` already grants git/build/edit.
2. Find the build's launch path (`CMakePresets.json` / `out/build/<preset>/...`).
3. Add its `Bash(<launch path>:*)` entries to the gitignored
   `.claude/settings.local.json` (Debug + Release).
4. Leave `settings.json` alone unless adding a genuinely portable allow (then
   commit + push so every platform gets it — as was done for `cp/mkdir/touch/pkill`).
