# KNOWN_ISSUES.md — parked cross-platform bugs

Cross-platform ClassBuilder bugs discovered during the port that are **not yet
fixed**, with the root cause and the *safe* fix approach recorded so no platform
re-discovers them. When one is fixed, say "fixed in \<commit\>" here rather than
deleting the entry.

## 1. Phantom dock "split bar" on initial open

**Symptom:** opening a model shows a dead ~4px separator strip on the window edge
(right edge first; after a window resize it relocates to the bottom). A manual
resize clears it. Reproduces on **macOS and Linux**.

**Cause:** `QtShellWindow` is a dock-only `QMainWindow` with a zero-size central
placeholder. `QMainWindow` reserves a `PM_DockWidgetSeparatorExtent` (4px)
separator between the single dock and that placeholder. `ShellSeparatorStyle`
suppresses the separator's *paint* but not the reserved 4px *gap*, so an unpainted
strip shows the window background. (Verified by instrumentation: with one model
the dock is `QRect(0,32 1008x691)` and the separator `QRect(1008,32 4x691)`,
`liveDocks == 1`.)

**DO NOT** fix by changing `pixelMetric`/`PM_DockWidgetSeparatorExtent`. Returning
0 when `<2` docks *did* remove the phantom but **broke dragging of a real
side-by-side split** — committed (`c4408e2`) then **reverted (`1d59d80`)**. The
separator width is what makes a real split divider grabbable; never zero it.

**Safe fix for next time:** leave the separator *width* at 4 (split-drag
untouched) and instead (a) add a timing-independent early-out in
`isPhantomSeparatorRect` — `liveDocks < 2` (visible, non-floating docks) ⇒ phantom,
since the geometry test misfires before layout settles on initial open; and (b) in
`ShellSeparatorStyle::drawPrimitive`, **paint** the phantom strip a blending colour
(try `QPalette::Base`) instead of leaving it unpainted. **Test BOTH** "phantom gone
on initial open" **and** "side-by-side split still drags" before committing — the
last attempt shipped without testing the split case, which is how it regressed.

## 2. Intermittent XWayland clipboard use-after-free crash

**Symptom:** on the Linux **xcb/XWayland** build, CB segfaults intermittently —
notably while idle, during clipboard activity (e.g. copying in another window).

**Signature:** classic use-after-free — doesn't reproduce under gdb (heap timing
shifts), survives a single run, fires on a later event. Likely the XWayland
X-selection bridge. Native Wayland avoids that code path but has its own GNOME
decoration gaps (see [PORTING_LINUX.md](PORTING_LINUX.md)).

**Next step:** chase with an **AddressSanitizer build** (`-fsanitize=address`) in a
focused session — app-level ASan pinpoints the free/use sites.

## Working rule this file encodes

UI/layout changes have non-obvious sibling states (single / tabbed / side-by-side
split / floating). Test the siblings, not just the case you fixed, and show the
diff before committing. Instrumenting with `qDebug` to confirm the real cause beats
guess-rebuild-eyeball loops.
