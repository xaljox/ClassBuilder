// qt/QtMenuStyle.h -- shared stylesheet for the compact right-click menus.
//
// Restyling a QMenu's item rows (for tighter spacing than the touch-tall
// native Windows menu) makes Qt's stylesheet engine take over drawing, which
// drops the native theme colours. Rather than hardcode replacements, this
// builds the stylesheet from the *live application palette* -- so the menu
// still tracks whatever Windows theme is in effect.
#pragma once

#include <QString>

class QMenu;

// A compact-QMenu stylesheet whose colours are read from qApp->palette():
// background / text / disabled straight from the palette, hover + separator +
// border derived from them (the background nudged toward the text colour --
// darker in a light theme, lighter in a dark one). Rebuild it each time a
// menu is shown so a theme change is picked up.
QString Qt_CompactMenuStyleSheet();

// Apply the compact stylesheet to `menu` AND work around the macOS Retina
// repaint artifact of styled menus: partial hover repaints land on
// half-device-pixels and leave 1px residue lines at row edges, so the menu
// is fully repainted whenever the hover moves (menus are small -- free).
// Use this instead of setStyleSheet(Qt_CompactMenuStyleSheet()).
void Qt_ApplyCompactMenuStyle(QMenu* menu);
