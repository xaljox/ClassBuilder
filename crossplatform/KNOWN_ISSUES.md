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

## 5. Blurry icons on Linux/HiDPI — **RESOLVED by mutter >= 47 (GNOME 47+); was an XWayland half-res artefact**

**Status (2026-07-14):** fixed by the environment, **no CB code change ever needed**.
Modern mutter (verified on **GNOME 50 / mutter 50**, Ubuntu 26.04) scales XWayland
clients at **native resolution by default**, so a plain Wayland session now gives
Qt/`xcb` **DPR 2 — sharp icons AND draggable floating docks** (both confirmed). The
"use an Xorg session" workaround below is **obsolete**; keep reading only if you land
on an old GNOME (<= 46).

The feature is *not* a gsettings flag on modern mutter — it **graduated out of
experimental**. On GNOME 50 `org.gnome.mutter experimental-features` lists only
`kms-modifiers` and `autoclose-xwayland`; both `scale-monitor-framebuffer` and
`xwayland-native-scaling` are gone *because they are now default behaviour*. So do
**not** try to "enable" it — an earlier revision of this file wrongly recorded a
`gsettings ... xwayland-native-scaling` line, which does nothing here.

**Symptom (on GNOME <= 46):** the tree + toolbar icons look blurry/mushy while text
still reads sharp. The same Qt code is sharp on Windows and macOS. Easy to misdiagnose
as an icon-art or SVG problem — **it is not**. The icon code is fine (SVG is enabled;
`CB_HAVE_SVG` is defined, `-- Qt: Svg module found` at configure).

**Root cause:** CB forces `QT_QPA_PLATFORM=xcb` on a Wayland session
(`WinMain.cpp`) because **native Wayland forbids the client-side window placement**
CB needs for its MDI / floating tool windows. But on a HiDPI monitor (e.g. a 2x
display: 3456x2168 framebuffer, 1728x1084 logical) **XWayland hands Qt a
devicePixelRatio of 1** — so CB paints the whole UI at HALF the real resolution and
mutter then magnifies the window 2x. *Everything* is bitmap-upscaled; the icons just
show it worst (1px strokes and hard edges turn to mush, while antialiased glyphs
survive magnification, which is why text still "looks sharp").

Measured on the same box with a 4-line `QGuiApplication` probe:

| session / platform | devicePixelRatio |
|--------------------|------------------|
| Wayland + `xcb` (XWayland) | **1.0**  ← blurry |
| Wayland + `wayland`        | 2.0 |
| **Xorg (x11) + `xcb`**     | **2.0**  ← sharp |

**No app-side setting can fix the XWayland case** — the X screen genuinely only *has*
1728x1084 pixels; you cannot add resolution that is not there. Raising
`QT_SCALE_FACTOR` only makes the UI bigger *and* still upscaled.

**Fix: log in to an "Ubuntu on Xorg" session** (GDM login screen → pick the user →
gear icon, bottom-right → *Ubuntu on Xorg*). On Xorg with a 2x display GNOME sets
`Xft.dpi=192`, so Qt/xcb gets **DPR 2 (sharp)** *and* real X11 window positioning
(**floating docks still drag**). No code change. GDM remembers the choice across
reboots — but if the session ever lands back on Wayland, the blur returns.

**Do NOT "fix" this by running native Wayland** (`QT_QPA_PLATFORM=wayland`): it is
sharp, but **floating dock windows cannot be moved** (confirmed by test) — exactly the
limitation the forced-`xcb` guard exists for.

**Bonus:** a pure Xorg session has no XWayland X-selection bridge, so it should also
sidestep **item 2** (the intermittent XWayland clipboard use-after-free crash).

**The real fix is simply a newer GNOME** — see the Status note at the top. Upgrade
(GNOME >= 47; verified on 50) and the blur is gone with no session games and no code
change. The Xorg session was only ever an interim measure for GNOME <= 46, and GNOME
is retiring the X11 session anyway.

### Latent HiDPI bugs this uncovered (both since FIXED in CB)

Running on a genuinely DPR-2 Linux desktop exposed two real port bugs that were
invisible while XWayland forced DPR 1. Recorded because the *class* will recur:

- **Mouse cursor SHRANK on entering a CB window** (`QtApp.cpp`). `XCURSOR_SIZE` is in
  **device** pixels, but the code used the desktop's **logical** cursor size (24) as
  its base, emitting `24 * uiScale` = 30 at 125% — *below* the desktop's real device
  size of `24 * DPR` = 48. Fixed by setting `XCURSOR_SIZE` **after** the QApplication,
  where `devicePixelRatio()` already folds in `QT_SCALE_FACTOR` (2 * 1.25 = 2.5), so
  `24 * dpr` = 60 is correct at any UI scale and any DPR. (libXcursor reads the var
  lazily, so post-construction is in time — verified.)
- **Window-edge resize hot zone sits outside the visible edge:** **NOT a CB bug.** That
  is mutter's *invisible resize border*, by design; the file manager behaves
  identically. Do not "fix" it.

Not available on GNOME/mutter **46** (this box): its `experimental-features` schema
offers only `scale-monitor-framebuffer`, `kms-modifiers`, `autoclose-xwayland`,
`variable-refresh-rate`, `x11-randr-fractional-scaling`. And it cannot be had via
`apt` — within an Ubuntu release GNOME stays on its shipped major, so 24.04 is capped
at mutter **46.x** (the package name encodes the major: `libmutter-14-0` *is* mutter
46; no `libmutter-15/16` exists in the archive). It takes a distro release upgrade.

## 6. A distro upgrade can stop CB starting — from-source Qt hard-links `libicu*.so.<N>`

**Symptom:** after a major distro upgrade, CB **does not start at all** — a busy mouse
cursor, then nothing, no window, no message box. (Hit on Ubuntu 24.04 -> 26.04,
2026-07-14.) Easy to mistake for a GUI/scaling bug; it is neither — the process dies
in the **dynamic loader**, before it paints a single pixel.

**Diagnose (always do this FIRST when CB won't start on Linux):**

```
ldd out/build/linux-x64/bin/Release/ClassBuilder | grep "not found"
./out/build/linux-x64/bin/Release/ClassBuilder        # run it, read stderr
```

which said:

```
libicui18n.so.74 => not found
error while loading shared libraries: libicui18n.so.74
```

**Root cause:** the from-source Qt at `~/Qt-6.11.1-patched` was built on 24.04 against
**ICU 74**, and `libQt6Core.so` carries that soname. 26.04 ships **ICU 78**; ICU has
**no cross-major ABI compatibility** and the old package is *not* installable
(`apt policy libicu74` → no candidate). So Qt6Core cannot load. **Do NOT try to symlink
`.so.78` to `.so.74`** — the symbols are versioned; it would load and then crash.
It is Qt that is broken here, **not CB**.

**Fix — rebuild the patched Qt, then CB:**

```
cd ~/qt-build
cmake --fresh -G Ninja -S src/qtbase-everywhere-src-6.11.1 -B build-qtbase \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/Qt-6.11.1-patched \
      -DQT_BUILD_EXAMPLES=OFF -DQT_BUILD_TESTS=OFF
cmake --build build-qtbase --parallel && cmake --install build-qtbase
# then the same --fresh / build / install for qtsvg and qtwayland
cd <repo> && cmake --preset linux-x64 --fresh && cmake --build --preset linux-release
```

`--fresh` matters: the old CMake caches pin the previous compiler (GCC 13 -> 15) and the
now-missing ICU 74 paths. **The two dock patches survive** in the source tree (they are
hand-applied; verify by grepping for `restore(QInternal::ClearSavedState)` in
`qdockwidget.cpp` and `mwLayout->savedState.clear();` in `qmainwindowlayout.cpp` — the
`.patch` files have prose headers and no `@@` line ranges, so `patch --dry-run` cannot
check them).

**Bonus — the rebuild made this unrepeatable:** with no `libicu-dev` present, Qt now
configures **`ICU ... no`** and uses its own built-in Unicode support (all CB needs).
The resulting Qt has **no `libicu` dependency at all**, so the next ICU bump cannot
break it. Do not "helpfully" reinstall `libicu-dev` and reintroduce the coupling.

**Applies to every from-source-Qt Linux box** — the Pi (Debian) will hit exactly this on
its next major upgrade. Verify after any distro upgrade with the `ldd | grep "not found"`
one-liner above.

## Working rule this file encodes

UI/layout changes have non-obvious sibling states (single / tabbed / side-by-side
split / floating). Test the siblings, not just the case you fixed, and show the
diff before committing. Instrumenting with `qDebug` to confirm the real cause beats
guess-rebuild-eyeball loops.
