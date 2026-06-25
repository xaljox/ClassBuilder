# Qt float-group tear-off crash — required Qt patch (Windows **and** macOS)

**TL;DR:** Qt **6.11.1** has a crash bug when a floating tabbed dock group
dissolves down to its last member (the "2→1 tear-off"). It is a bug in Qt's own
source (`qmainwindowlayout.cpp`), **not** in ClassBuilder, and it is
**platform-independent** — it bites on Windows and on macOS. The fix is a tiny,
verbatim backport of the upstream Qt fix. The diff is committed here:

> **[`docs/qt-patches/qtbase-6.11.1-dockgroup-dissolution.patch`](qt-patches/qtbase-6.11.1-dockgroup-dissolution.patch)**

The Windows static-Qt build at `C:/Qt-static/6.11.1` already has it applied. The
**macOS** build needs the equivalent (see below).

---

## Symptom

ClassBuilder uses Qt `GroupedDragging`, so diagram/tree docks can be dragged
together into a **floating tabbed group**. Tearing members back out one at a time
is fine (3→2, N→≥2) — but the **final 2→1 dissolution crashes** with an access
violation. Plain-API float-one-out / float-all-out crash the same way (deferred).
The crash is synchronous inside the drag's mouse-release stack, so there is **no
app-side workaround** — it must be fixed in Qt.

## Root cause (one line in `QDockWidgetGroupWindow::reparentToMainWindow`)

The survivor's reparent mutates `mwLayout->layoutState`, but the drag's
`savedState` snapshot **shares** `QLayoutItem*` with `layoutState`, so it goes
stale. The nested `setFloating()` → `endDrag()` → `QMainWindowLayout::restore()`
then does `layoutState = savedState` + `reparentWidgets()` over a **freed**
`widgetItem` → AV (`qdockarealayout.cpp` ~2210). The fix clears `savedState` up
front (resets structure only, doesn't delete the shared items), which makes
`restore()` early-return. This is exactly the upstream fix.

- Upstream report: https://forum.qt.io/topic/164810 (QTBUG-118579 family)
- Maintainer (Christian Ehrlicher): "most likely fixed in Qt 6.11.2 and 6.12."

## macOS: how to get the fix

Pick based on how you obtain Qt on the Mac:

1. **Qt ≥ 6.11.2 / 6.12 (preferred).** The upstream fix is on the 6.11 branch, so
   a Qt newer than 6.11.1 should already contain it. **Use the newest Qt you can**
   and then **verify** the 2→1 tear (the maintainer said "most likely", and ours
   was a Windows-surfaced variant — confirm it on macOS). If verified fixed, you
   need **no** local patch.

2. **Building Qt from source on macOS at 6.11.1 (or any version still affected).**
   Apply the committed patch to the qtbase source, then build/install:
   ```sh
   cd <qtbase-everywhere-src-6.11.1>
   patch -p1 < <repo>/docs/qt-patches/qtbase-6.11.1-dockgroup-dissolution.patch
   # then configure + build + install qtbase as usual
   ```
   (The patch targets `src/widgets/widgets/qmainwindowlayout.cpp`. The leading
   context is intentionally loose — if `patch` can't place it, the hunk is a
   single added block right after `mwLayout->widgetAnimator.abort(dockWidget);`
   in `QDockWidgetGroupWindow::reparentToMainWindow()`; add it by hand.)

3. **Prebuilt Qt (brew / Qt installer) that is still affected.** You can't patch a
   prebuilt binary — either move to a fixed version (option 1) or build that
   version from source with the patch (option 2). As a stopgap, the app keeps a
   crash-recovery net (emergency-save `<name>.recovered.cbz` + restart) so a tear
   crash doesn't lose work.

## Verifying

Repro: build a floating tabbed group of 3+ docks, then tear members out one by one.
On an affected Qt it crashes at the **final 2→1**; with the fix, create / move /
3→2 / 2→1 all work (a one-time repaint **flash** on tear is inherent Qt reparent
behaviour and is expected, patched or not). A ~40-line standalone repro
(`QMainWindow` + `GroupedDragging` + 3 docks) exists in the pre-restructure tree at
`..\ClassBuilder_old\qtdockrepro\` (with the forum post at
`qtdockrepro\upstream\FORUM_POST.md`) — resurrect it into the repo if you want a
self-contained cross-platform verifier.

## When this can be deleted

Once the project's Qt baseline is **≥ 6.11.2 on every platform** and the 2→1 tear
is verified non-crashing, drop the local patch (Windows static-Qt rebuild without
it, and no Mac patch) and this note can be archived.
