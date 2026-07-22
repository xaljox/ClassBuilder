# crossplatform/ — Windows ↔ macOS/Linux port coordination

This directory is the shared channel for taking ClassBuilder cross-platform. It
travels in git, so whatever lands here on the Windows side is readable on the Mac
side after `git pull` (and vice-versa). Keep porting notes, decisions, status, and
any required third-party patches here — not scattered through `docs/`.

## The workflow it supports

ClassBuilder **generates its own source** and only *runs* on Windows today, so the
two sides are asymmetric:

- **Windows = source of truth.** Regenerate model sources on Windows, commit the
  generated `.cpp/.h`, push.
- **Mac/Linux build the committed sources** (they don't run CB to regenerate).
- **The platform pivot is `src/platform/CbPlatformCompat.*`** — the single
  `#ifdef` seam the model touches the window system through.

So the loop is: *Windows regenerate → commit → pull on Mac → build → fix the next
platform gap in a `WIN32`/`__APPLE__` branch → push the note back here → repeat.*

## Contents

| File | What |
|------|------|
| [PORTING_MAC.md](PORTING_MAC.md) | Getting-started report: Mac prerequisites, what's already cross-platform, and the file-by-file Win32/MSVC punch-list. **Start here.** |
| [PORTING_LINUX.md](PORTING_LINUX.md) | Building on Linux: **distro (apt) Qt vs STATIC Qt 6.11.1 from source** (self-contained, the intended build — full apt build-deps + recipe; no patch), xcb-as-daily-driver, Wayland build-order gotcha, and the Parallels crispness fix. |
| [KNOWN_ISSUES.md](KNOWN_ISSUES.md) | Parked cross-platform CB bugs with root cause + the *safe* fix approach (phantom dock split-bar; XWayland clipboard UAF). Read before re-fixing dock/separator code. |
| [QT_DOCK_TEAROFF_PATCH.md](QT_DOCK_TEAROFF_PATCH.md) | The TWO Qt 6.11.1 dock bugs (float-group "2→1 tear-off" crash + post-tear-off layout freeze QTBUG-147209). **Superseded 2026-07-21:** the trigger path is now disabled in CB, so the Linux static build is stock/unpatched; kept as history + for the Win/Mac patched builds. |
| [qt-patches/](qt-patches/) | Verbatim third-party patches that must be applied to the platform's Qt build. |
| [CLAUDE_CODE_SETUP.md](CLAUDE_CODE_SETUP.md) | Shared Claude Code config: the tracked-vs-local permission split and each platform's build/launch path. Read before touching `.claude/` permissions. |

## Conventions for this directory

- One topic per file; link them from this README's table.
- Patches go in `qt-patches/` as plain `.patch` files appliable with `patch -p1`,
  with a header explaining *why* and *when to drop it*.
- When a platform gap is closed, update the relevant note (don't delete the
  history — say "fixed on macOS in <commit>") so the other side stays in sync.
