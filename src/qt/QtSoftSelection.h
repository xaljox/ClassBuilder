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

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QString>

// Give a popup list the tree's soft selection look: a translucent accent
// wash with the normal (dark) text colour kept, plus the same subtle hover
// tint on non-selected rows. Call once after the view is created.
inline void Qt_ApplySoftSelection(QAbstractItemView* view)
{
    // Read the ACTIVE application palette explicitly: the Inactive Highlight
    // is the grey unfocused-selection colour, and a popup is never active
    // (same reasoning as CbTreeWidget::drawBranches).
    const QColor acc =
        QApplication::palette().color(QPalette::Active, QPalette::Highlight);
    // Same recipe as CbTreeWidget's macOS ::item:selected rule: rgba() over
    // the base background keeps the tint soft in light and dark mode. The
    // :hover rule mirrors the tree's hover feedback -- and a hover QSS rule
    // is also what switches hover tracking on for the view.
    view->setStyleSheet(QString(
        "QAbstractItemView::item:selected {"
        "  background: rgba(%1, %2, %3, 0.28);"
        "  color: palette(text);"
        "}"
        "QAbstractItemView::item:hover:!selected {"
        "  background: rgba(%1, %2, %3, 0.10);"
        "}")
        .arg(acc.red()).arg(acc.green()).arg(acc.blue()));
}
