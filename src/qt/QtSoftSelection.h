// qt/QtSoftSelection.h -- the soft selection tint for popup item views.
//
// The main tree paints selection as a translucent accent tint over the base
// background with the normal dark text kept (CbTreeWidget's macOS QSS). The
// editor's popup lists (code completion, who-calls-me) got whatever the
// platform style paints instead, and that never matched the tree: a
// Qt::Popup window is never the ACTIVE window, so Windows renders its grey
// inactive-selection look, while macOS and GNOME paint the full saturated
// accent with white text -- three different selection looks in one editor.
// One explicit rule, derived from the live theme accent, gives every popup
// the tree's soft tint on every platform, independent of the popup's
// active/inactive state.
#pragma once

#include <cmath>

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QProgressBar>
#include <QString>

// The accent to draw the tree's small SOLID GLYPHS in -- the expand/collapse
// triangles, the connector lines, the selected row's left stripe.
//
// The one rule, shared by every platform: take the accent as the desktop chose
// it and clamp its HSL LIGHTNESS. A dark accent passes through untouched; a
// light one -- the default macOS system blue, a pastel desktop accent -- is
// deepened along its own hue (hue and saturation kept, so it stays the user's
// colour) until it carries at glyph size. Only these marks need it: at a few
// pixels wide they wash out on the light tree where the very same accent works
// fine as a row-sized tint or a filled selection, which is why the accent
// itself is NOT corrected at the chokepoint (JV 2026-07-19 / 2026-07-21).
//
// One knob (the clamp), one place: the chokepoint (QtApp.cpp) fetches the
// accent per platform and everything from there -- including this -- is shared,
// so the same chosen accent gives the same glyphs everywhere.
inline QColor Qt_ChromeAccent()
{
    QColor c = QApplication::palette().color(QPalette::Active,
                                             QPalette::Highlight);
    if (c.lightnessF() > 0.40f)
        c.setHslF(qMax(0.0f, c.hslHueF()), c.hslSaturationF(), 0.40f);
    return c;
}
// The soft selection/hover tint, derived PURELY from the live theme accent
// (QPalette::Active/Highlight) blended OVER the Base background into an OPAQUE
// colour -- computed here rather than left as a translucent QSS wash.
//
// Why opaque matters: a translucent rgba() background lets the platform style's
// OWN selection fill show through underneath (on GNOME a saturated blue that is
// unrelated to the accent). That is exactly what split a selected tree row into
// "accent gutter + native-blue item", tinted the hover blue, and painted the
// completion/who-calls-me popups blue -- only Base-coloured areas came out in
// the accent. An opaque fill covers that native paint entirely, so the visible
// colour is 100% the accent on every platform (blue on Windows/Mac where the
// accent is blue, orange under an orange GNOME accent, etc.).
//
// Read the ACTIVE group explicitly: the Inactive Highlight is the grey
// unfocused-selection colour, and a popup is never the active window (same
// reasoning as CbTreeWidget::drawBranches). `alpha` is the blend weight
// (0.28 selection, 0.10 hover) -- identical to CbTreeWidget's gutter, so tree
// and popups read as one look.
inline QColor Qt_SoftSelectionColor(double alpha)
{
    const QPalette pal  = QApplication::palette();
    const QColor   acc  = pal.color(QPalette::Active, QPalette::Highlight);
    const QColor   base = pal.color(QPalette::Active, QPalette::Base);
    const auto mix = [alpha](int b, int a) {
        return int(b * (1.0 - alpha) + a * alpha + 0.5);
    };
    return QColor(mix(base.red(),   acc.red()),
                  mix(base.green(), acc.green()),
                  mix(base.blue(),  acc.blue()));
}

// Colour a progress bar's fill with the SAME tint as a selected tree row
// (Qt_SoftSelectionColor(0.28) -- the stronger-than-hover, not-full-accent
// variant JV asked for), over a plain base track, so Read/Write Source
// progress reads in the theme accent and matches the tree/popup selection.
inline void Qt_ApplyProgressAccent(QProgressBar* bar)
{
    const QColor chunk = Qt_SoftSelectionColor(0.28);
    bar->setStyleSheet(QString(
        "QProgressBar {"
        "  border: 1px solid palette(mid); border-radius: 3px;"
        "  background: palette(base); text-align: center;"
        "  color: palette(text);"
        "}"
        "QProgressBar::chunk { background-color: rgb(%1, %2, %3); }")
        .arg(chunk.red()).arg(chunk.green()).arg(chunk.blue()));
}

// Give a popup list the tree's soft selection look: the opaque accent tint
// above with the normal (dark) text colour kept, plus the same subtle hover
// tint on non-selected rows. Call once after the view is created. (A :hover
// QSS rule is also what switches hover tracking on for the view.)
inline void Qt_ApplySoftSelection(QAbstractItemView* view)
{
    // Mark the view so the app-wide accent watcher (QtApp.cpp) can find every
    // soft-selection popup and re-derive its tint when the theme accent changes
    // while CB is open -- without QtApp needing to know each popup's type.
    view->setProperty("cbSoftSelection", true);
    const QColor sel   = Qt_SoftSelectionColor(0.28);
    const QColor hover = Qt_SoftSelectionColor(0.10);
    // outline:0 -- no focus ring on the current row. The style draws a hard dark
    // rectangle around it, and in a popup the current row ALWAYS has focus, so it
    // sits there permanently as a black band over the soft tint. Exactly the same
    // ring CbTreeWidget switches off (its QSS carries the same rule since JV
    // asked for the left stripe instead, 2026-07-18); the popups simply never got
    // it (JV 2026-07-21). The selection is already marked by the tint below.
    view->setStyleSheet(QString(
        "QAbstractItemView { outline: 0px; }"
        "QAbstractItemView::item:selected {"
        "  background: rgb(%1, %2, %3);"
        "  color: palette(text);"
        "}"
        "QAbstractItemView::item:hover:!selected {"
        "  background: rgb(%4, %5, %6);"
        "}")
        .arg(sel.red()).arg(sel.green()).arg(sel.blue())
        .arg(hover.red()).arg(hover.green()).arg(hover.blue()));
}
