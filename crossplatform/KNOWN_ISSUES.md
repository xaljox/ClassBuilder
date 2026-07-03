# KNOWN_ISSUES.md — parked cross-platform bugs

Cross-platform ClassBuilder bugs discovered during the port that are **not yet
fixed**, with the root cause and the *safe* fix approach recorded so no platform
re-discovers them. When one is fixed, say "fixed in \<commit\>" here rather than
deleting the entry.

## 1. Phantom dock "split bar" on initial open — **FIXED (Windows-side, 2026-07-03)**

**Fixed in the commit adding this line** (three coordinated fixes; details in the
commit message + [QT_DOCK_TEAROFF_PATCH.md](QT_DOCK_TEAROFF_PATCH.md)):

1. **Interactivity killed (shell,** `QtShellWindow` `event()`**):** the phantom no
   longer shows a resize cursor or accepts drags. Note for anyone touching this:
   a bare `unsetCursor()` does NOT work — `QMainWindow`'s `CursorChange` handler
   re-sets its adjusted split cursor synchronously from inside the un-set; the fix
   swallows exactly that one re-adjust event (`_suppressCursorReadjust`).
2. **Post-tear-off layout freeze was a Qt 6.11.1 regression** (QTBUG-147209): after
   floating a dock out, `savedState` stayed valid and `QMainWindowLayout::
   setGeometry` early-returned forever (remaining dock didn't refit; window resizes
   ignored — this is what "a manual resize clears it / relocates it" really was).
   Upstream backport committed as
   [qt-patches/qtbase-6.11.1-enddrag-savedstate-freeze.patch](qt-patches/qtbase-6.11.1-enddrag-savedstate-freeze.patch)
   — **macOS/Linux must apply it to `~/Qt-6.11.1-patched` and rebuild QtWidgets.**
3. **The remaining 4px strip is structural** (qGeomCalc reserves one separator
   between a populated dock area and the zero-size central placeholder — the
   center cell counts as non-empty because the placeholder exists, and it must
   exist or the dock band's maximumSize stays 0 and docks collapse). It cannot be
   removed, only PLACED: `addDocument` now docks trees into the **TOP** area, so
   the strip lies above the status bar (reads as chrome) instead of a full-height
   band at the right edge, and startup matches the post-tear-off+redock state.

The old warning stands: **never** zero `PM_DockWidgetSeparatorExtent` (the
`c4408e2`→`1d59d80` revert) — `QDockAreaLayout` caches `sep` at construction, so a
dynamic metric desyncs hit-testing from layout and breaks real split drags.

## 2. Intermittent XWayland clipboard use-after-free crash

**Symptom:** on the Linux **xcb/XWayland** build, CB segfaults intermittently —
notably while idle, during clipboard activity (e.g. copying in another window).

**Signature:** classic use-after-free — doesn't reproduce under gdb (heap timing
shifts), survives a single run, fires on a later event. Likely the XWayland
X-selection bridge. Native Wayland avoids that code path but has its own GNOME
decoration gaps (see [PORTING_LINUX.md](PORTING_LINUX.md)).

**Next step:** chase with an **AddressSanitizer build** (`-fsanitize=address`) in a
focused session — app-level ASan pinpoints the free/use sites.

## 3. Floating dock tab-group crash (GroupedDragging) — disabled on all platforms

**Symptom:** forming a **floating tab group** — dragging one dock onto another so
they become tabs in one floating window (`QDockWidgetGroupWindow`) — or tearing /
closing such a group crashes intermittently (`EXC_BAD_ACCESS`).

**Backtrace (macOS):** `QMainWindowLayout::animationFinished(QWidget*)` →
`QWidget::setParent()` → `setParent_sys()` dereferencing a freed dock (address
`0x8`); the crashing `QPropertyAnimation` is being deferred-deleted. Every frame
above the event loop is Qt.

**It is a platform-independent timing use-after-free**, not macOS-specific. It
surfaces far more readily on macOS (cocoa native NSView reparent) but is confirmed
on **Linux (xcb)** too, and on Windows with enough attempts — earlier "Windows/Linux
are fine" was luck in one-model tests. No safe subset: it fires same-model or
cross-model, on the first group op or the Nth. **Not** the tab styling and **not**
`AnimatedDocks` (both ruled out by test). Qt 6.11.1; no upstream fix on the 6.11 or
dev branches (only the two dock patches already backported).

**Stopgap (`QtShellWindow` ctor `setDockOptions`):** `GroupedDragging` **disabled on
all platforms** → no floating tab-group can form → crash unreachable. Everything
else works: tabs, side-by-side splits, and **single-dock floating** (drag the dock
*clear* of the main — the main eagerly re-grabs a float dropped near it, which
briefly looked like "no floats"; it isn't). **Do NOT** try to keep GroupedDragging
and block only group *formation* — no clean Qt hook, and single-model groups crash
too. **Do NOT** disable GroupedDragging per-platform expecting floating to break on
some — it doesn't; the eager-grab is universal.

**Real fix / when to re-enable:** find or write the upstream fix for the
`animationFinished` reparent UAF, backport it like the dissolution + endDrag
patches, then restore `GroupedDragging`. Expires at Qt ≥ 6.11.2 if that clears it.
Until then the GUI-crash net emergency-saves open models, so a crash loses no work.

## Working rule this file encodes

UI/layout changes have non-obvious sibling states (single / tabbed / side-by-side
split / floating). Test the siblings, not just the case you fixed, and show the
diff before committing. Instrumenting with `qDebug` to confirm the real cause beats
guess-rebuild-eyeball loops.
