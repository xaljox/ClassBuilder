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

**It is a platform-independent timing use-after-free** in Qt's own code, observed
on **macOS** (where it surfaces most readily — cocoa native NSView reparent) and
**Linux (xcb)**. **NOT reproduced on Windows** (an earlier report naming Windows
was a miscommunication, corrected by JV 2026-07-04) — but two of three platforms
crashing in shared Qt code is reason enough to disable it everywhere rather than
gamble on the third. No safe subset: it fires same-model or cross-model, on the
first group op or the Nth. **Not** the tab styling and **not** `AnimatedDocks`
(both ruled out by test). Qt 6.11.1; no upstream fix on the 6.11 or dev branches
(only the two dock patches already backported).

**Stopgap (`QtShellWindow` ctor `setDockOptions`):** `GroupedDragging` **disabled on
all platforms** → no floating tab-group can form → crash unreachable. Everything
else works: tabs, side-by-side splits, and **single-dock floating** (drag the dock
*clear* of the main — the main eagerly re-grabs a float dropped near it, which
briefly looked like "no floats"; it isn't). **Do NOT** try to keep GroupedDragging
and block only group *formation* — no clean Qt hook, and single-model groups crash
too. **Do NOT** disable GroupedDragging per-platform expecting floating to break on
some — it doesn't; the eager-grab is universal.

**Real fix / when to re-enable:** investigate on the **Mac first** — it reproduces
most readily there (JV: "later uitzoeken op de mac waar het het makkelijkste fout
gaat"). Root-cause the `animationFinished` reparent UAF, find or write the upstream
fix, backport it like the dissolution + endDrag patches, then restore
`GroupedDragging`. Expires at Qt ≥ 6.11.2 if that clears it. Until then the
GUI-crash net emergency-saves open models, so a crash loses no work.

## 4. Dock title bar: redundant/oversized text vs its own tab label — FIXED (Linux, 2026-07-04)

**Symptom:** a standalone/floating dock's title bar (e.g. a diagram window,
`CD: <name>`) renders **bold**, at the native style's own dock-title font --
visibly bigger/heavier than the exact same name shown as a tab label the moment
that dock gets tabbed with another. Because the title bar is *never* hidden
(see #1's sibling note and "Never call `setTitleBarWidget()`" in
`QtShellWindow.cpp`), a tabbed dock shows the name **twice**: once small in the
tab, once bold/bigger in its own still-painted title bar.

**Root cause:** CB deliberately never hides or swaps a dock's title bar
(`setTitleBarWidget()` crashed on tear-off, JV-confirmed 2026-06-19), so the bar
is *always* painted, tabbed or not. The native `QStyle` paints
`CE_DockWidgetTitle` bold at its own title font regardless of what font is set
on the dock widget itself -- setting `dock->setFont(...)` alone does not
override that.

**Fix (`QtShellWindow.cpp`, `ShellSeparatorStyle::drawControl` + the new
`isDockTabbed()` helper):** intercept `CE_DockWidgetTitle` painting only --
delegate the background/float/close-button chrome to the base style with the
title text blanked, then draw the text ourselves: at the app's plain
(non-bold) font matching the tab label when the dock is standalone/floating,
or **no text at all** when `isDockTabbed()` finds a sibling `QTabBar` whose
`tabData()` points at this dock (same Qt convention `wireDockTabBars()` already
relies on for the tab close-cross) -- the tab already names it. The bar itself
stays present, draggable, and closable in both cases; no `setTitleBarWidget()`
call anywhere, so this doesn't touch the crash path above.

Verified on Linux across all three sibling states this file's working rule
calls for: tabbed, standalone/floating, and side-by-side split.

**Windows: verified and EXTENDED (2026-07-04, commit `f395d4f`).** The blanked
tabbed title exposed that Windows' own tab rendering never connects the
selected tab to the bar below it (white bordered tab on a grey strip with its
own edge line). Windows now hand-renders the notebook look the other
platforms get natively — JV's rule: **the selected tab must merge seamlessly
(same colour, NO separating line) into the pane below; that's what shows tab
and window belong together.** Both sides of the seam are owned Windows-only:
the dock-tab QSS (selected = `QPalette::Window`, open bottom; unselected a
step darker, 3px lower, edge line underneath) + a flat colour-matched
`CE_DockWidgetTitle` fill for TABBED docks. Per-tab close crosses are gone on
all platforms (the always-present title-bar close button closes a tabbed
pane).

**macOS still to verify** -- confirm the plain-font standalone title and the
blanked tabbed title both read correctly under Aqua before calling this fully
cross-platform-closed. See also the new manual UI-scale feature in
[PORTING_LINUX.md](PORTING_LINUX.md#manual-ui-scale-view--ui-scale-menu),
which is likewise untested outside Linux.

## Working rule this file encodes

UI/layout changes have non-obvious sibling states (single / tabbed / side-by-side
split / floating). Test the siblings, not just the case you fixed, and show the
diff before committing. Instrumenting with `qDebug` to confirm the real cause beats
guess-rebuild-eyeball loops.
