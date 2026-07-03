# Qt dock bugs — required Qt patches (Windows, macOS **and** Linux)

**TL;DR:** Qt **6.11.1** has TWO platform-independent dock bugs in qtbase, both
from the same 6.11 dock rework, both **not** in ClassBuilder, both fixed by
tiny verbatim backports of the upstream fixes. The diffs are committed here:

> **[`qt-patches/qtbase-6.11.1-dockgroup-dissolution.patch`](qt-patches/qtbase-6.11.1-dockgroup-dissolution.patch)**
> — crash when a floating tabbed dock group dissolves to its last member
> (the "2→1 tear-off"); QTBUG-118579 family. Details below.
>
> **[`qt-patches/qtbase-6.11.1-enddrag-savedstate-freeze.patch`](qt-patches/qtbase-6.11.1-enddrag-savedstate-freeze.patch)**
> — ADDED 2026-07-03: after a dock is torn out and left floating, the main
> window's dock layout FREEZES (remaining docks don't refit; main-window
> resizes no longer resize the dock area) until something is docked back.
> Upstream **QTBUG-147209**, a 6.11.1-only regression, fixed upstream for
> 6.11.2 (commit `e9a22af5ab7f`). Two lines in `qdockwidget.cpp`
> (`restore(QInternal::KeepSavedState)` → `ClearSavedState` in `startDrag` +
> `endDrag`). Apply it to the same patched Qt tree, rebuild QtWidgets, done —
> the full doc header inside the patch file has symptom/root-cause detail.

The Windows static-Qt build at `C:/Qt-static/6.11.1` has BOTH applied
(2026-07-03). The **macOS and Linux** patched trees at `~/Qt-6.11.1-patched`
had only the first at that date — apply the second the same way (by hand: two
one-word edits) and rebuild/install qtbase (Widgets), then rebuild CB. Both
patches expire together at Qt ≥ 6.11.2.

The remainder of this doc covers the first (tear-off crash) patch in detail;
the build/apply recipe per platform applies to both.

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

### DONE on macOS (2026-06-25) — option 2, from-source patched Qt

brew only ships the affected **6.11.1** (no 6.11.2/6.12 bottle yet), so we built
a patched Qt from source and pointed the build at it. Reproducible recipe (arm64,
Apple Silicon):

```sh
# 1. Get the matching source (qtbase + qtsvg; CB uses Widgets/Network/Svg)
mkdir -p ~/qt-build/src && cd ~/qt-build/src
B=https://download.qt.io/official_releases/qt/6.11/6.11.1/submodules
curl -fsSLO $B/qtbase-everywhere-src-6.11.1.tar.xz
curl -fsSLO $B/qtsvg-everywhere-src-6.11.1.tar.xz
tar xf qtbase-everywhere-src-6.11.1.tar.xz && tar xf qtsvg-everywhere-src-6.11.1.tar.xz

# 2. Apply the patch BY HAND (the .patch hunk header has no line numbers, so
#    `patch -p1` / `git apply` can't place it): insert mwLayout->savedState.clear();
#    right after `mwLayout->widgetAnimator.abort(dockWidget);` in
#    qtbase-.../src/widgets/widgets/qmainwindowlayout.cpp
#    (function QDockWidgetGroupWindow::reparentToMainWindow).

# 3. Build qtbase, then qtsvg, into ~/Qt-6.11.1-patched.
#    Two macOS-specific gotchas, both worked around with cache flags / arch:
#    (a) Qt's CMake aborts with "Can't determine Xcode version" when only the
#        Command Line Tools are installed (no full Xcode). The macOS SDK comes
#        from `xcrun` (works); only `xcodebuild` is missing -> skip that check
#        with -DQT_NO_XCODE_MIN_VERSION_CHECK=ON (+ -DQT_NO_APPLE_SDK_MIN_VERSION_CHECK=ON).
#        NO full Xcode install needed.
#    (b) **Toolchain arch trap:** if `which cmake`/`ninja` resolve to an
#        *x86_64* Homebrew (/usr/local), the native Qt build comes out x86_64
#        and won't link against the arm64 app (ld: "found architecture 'x86_64',
#        required architecture 'arm64'"). Specifying -DCMAKE_OSX_ARCHITECTURES=arm64
#        instead makes Qt think it's cross-compiling and demand QT_HOST_PATH.
#        FIX: install/use arm64 cmake+ninja from the /opt/homebrew brew so the
#        native build is arm64. Verify: `file $(which cmake)` -> arm64.
cd ~/qt-build
cmake -S src/qtbase-everywhere-src-6.11.1 -B build-qtbase -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/Qt-6.11.1-patched \
  -DQT_BUILD_TESTS=OFF -DQT_BUILD_EXAMPLES=OFF \
  -DFEATURE_sql=OFF -DFEATURE_dbus=OFF -DFEATURE_printsupport=ON \
  -DQT_NO_XCODE_MIN_VERSION_CHECK=ON -DQT_NO_APPLE_SDK_MIN_VERSION_CHECK=ON
cmake --build build-qtbase --parallel && cmake --install build-qtbase
cmake -S src/qtsvg-everywhere-src-6.11.1 -B build-qtsvg -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$HOME/Qt-6.11.1-patched \
  -DCMAKE_INSTALL_PREFIX=$HOME/Qt-6.11.1-patched \
  -DQT_NO_XCODE_MIN_VERSION_CHECK=ON -DQT_NO_APPLE_SDK_MIN_VERSION_CHECK=ON
cmake --build build-qtsvg --parallel && cmake --install build-qtsvg

# 4. Point CB at it. A local-only CMakeUserPresets.json (gitignored) adds a
#    `mac-patched` preset inheriting `mac` but with
#    CMAKE_PREFIX_PATH=$env{HOME}/Qt-6.11.1-patched. The committed `mac` preset
#    stays on brew Qt for any machine that doesn't have the patched build.
cmake --preset mac-patched && cmake --build --preset mac-patched-debug
```

The committed `mac` preset is unchanged (still brew). **Drop all of this and use
brew `mac` once brew ships Qt ≥ 6.11.2.**

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
