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
- `allow: ["Bash"]` — **all** shell commands run without prompting (curating a
  per-command allowlist was pure whack-a-mole: read tools, pipeline segments, and
  out-of-project Qt work kept surfacing new prompts).
- `deny` (takes precedence) — **only what "refetch the last commit" can't
  undo.** The safety net is git: commit/push regularly, and reverting an
  experiment is just discarding uncommitted changes (`git reset --hard` /
  `git restore .` / `git checkout -- .` — all allowed). So deny is limited to:
  - `git push --force` / `-f` / `--force-with-lease` — rewrites the *pushed*
    remote, i.e. the safety net itself.
  - `rm`, `rmdir`, `sudo`, `dd`, `mkfs`, `shutdown`, `reboot` — can destroy
    things git doesn't protect: untracked files and **out-of-repo assets**
    (e.g. the from-source patched Qt at `~/Qt-6.11.1-patched` / `~/qt-build`,
    not in any commit).

  Everything else — including local history ops like `git reset --hard`,
  `git clean`, `git rebase` — runs without a prompt.

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
