// qt/QtDesktopTheme.h -- the desktop-theme bits CB has to read itself on Linux.
//
// On Windows and macOS the theme reaches Qt through what the app already links:
// the DWM registry, and the Cocoa integration inside the platform plugin. On
// Linux it arrives through a SEPARATE platform-theme plugin (gtk3) -- and a
// static Qt built without GTK simply has none, so Qt falls back to its own
// defaults: the accent reads as #308cc6 blue and the icon theme as Adwaita,
// whatever the desktop is actually set to (both measured 2026-07-22, and both
// visible: CB went blue, and the file dialogs kept blue Adwaita folders).
//
// Building the gtk3 plugin instead was measured and rejected: it cannot be
// statically linked (no libgtk-3.a exists; libgtk-3-dev ships only .so), it
// would add the whole GTK stack -- 83 shared libs transitively -- to a build
// whose point is NOT depending on external versioned libraries, GTK is LGPL so
// static linking carries a relink obligation, and CB would stop starting on a
// box without GTK3. Reading the two settings ourselves costs nothing at
// runtime.
//
// Everything here is a no-op off Linux.
#pragma once

#include <QColor>
#include <QFileDialog>

// The accent as the DESKTOP publishes it, over D-Bus from
// org.freedesktop.portal.Settings ("org.freedesktop.appearance"/"accent-color").
// Invalid QColor when there is no portal, no session bus, or no accent set --
// callers then fall back to the palette.
QColor Cb_PortalAccent();

// Fallback accent for desktops whose portal publishes NO accent-color -- labwc /
// Pi OS on wlroots only exposes color-scheme + contrast, not accent-color. There
// the chosen Qt palette lives in the active qt5ct/qt6ct colour scheme (what a
// DYNAMIC Qt would have loaded through the qt5ct/qt6ct platform-theme plugin; a
// static Qt has no such plugin). This reads that scheme's Highlight straight from
// the file -- QtCore only, no extra module or plugin, no D-Bus. Invalid QColor
// when there is no qt5ct/qt6ct custom palette to read.
QColor Cb_PlatformThemeAccent();

// Watch the portal for accent changes; `onChanged` runs on each one. Call once,
// after the QApplication exists. This is also the only live trigger there is:
// Qt delivers no ApplicationPaletteChange for an accent switch on any desktop
// we measured, so without the portal signal CB would need a restart.
void Cb_StartPortalAccentWatch(void (*onChanged)());

// Point QIcon at the desktop's icon theme. The portal does not publish it (its
// appearance namespace is colour-scheme / accent-colour / contrast only), and
// no file carries the user's choice -- GNOME keeps it in dconf. So this asks
// gsettings, once, at startup.
// Cheap and idempotent, so it is called at startup AND again just before a file
// dialog is opened -- which is what keeps it correct after a theme change.
//
// Hooking it to the accent-change signal instead does NOT work: the portal
// fires that signal before GNOME has written the matching icon theme to dconf,
// so the read returns the PREVIOUS name and CB ends up exactly one change
// behind (observed 2026-07-22 -- the sub-150ms lag is invisible to sampling and
// specified nowhere). Reading it when the dialog is about to be built sidesteps
// the race completely: by then the value has long since landed.
void Cb_ApplyDesktopIconTheme();

// THE door every CB file dialog goes through: it picks the backend AND refreshes
// the icon theme, so neither can be forgotten at a call site. That is not
// theoretical -- the SVG exports kept their own copy of the options, so they
// missed the refresh and showed whatever theme the last Open dialog had left
// behind (JV 2026-07-22). Use this instead of QFileDialog::Options directly.
QFileDialog::Options Cb_FileDialogOptions();

// CB's file dialogs. Use these, not QFileDialog's static helpers: the statics
// return a path but keep the dialog itself out of reach, so its file list ends
// up painting selection the platform style's way -- with the file-name field
// focused, that is the flat GREY unfocused selection instead of CB's accent
// tint (JV 2026-07-22). These build the dialog, apply the shared options + the
// selection look, and hand back the chosen path ("" when cancelled).
QString Cb_OpenFileName(QWidget* parent, const QString& caption,
                        const QString& filter);
QString Cb_SaveFileName(QWidget* parent, const QString& caption,
                        const QString& initial, const QString& filter);
