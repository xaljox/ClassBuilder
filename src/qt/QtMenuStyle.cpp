// qt/QtMenuStyle.cpp -- compact right-click-menu stylesheet, theme-derived.

#include "QtMenuStyle.h"

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace {
// Linear blend of two colours: t=0 -> a, t=1 -> b.
QColor blend(const QColor& a, const QColor& b, double t)
{
    return QColor(int(a.red()   * (1 - t) + b.red()   * t),
                  int(a.green() * (1 - t) + b.green() * t),
                  int(a.blue()  * (1 - t) + b.blue()  * t));
}
}

QString Qt_CompactMenuStyleSheet()
{
    const QPalette pal = qApp->palette();
    const QColor bg   = pal.color(QPalette::Active,   QPalette::Window);
    const QColor text = pal.color(QPalette::Active,   QPalette::WindowText);
    const QColor dis  = pal.color(QPalette::Disabled, QPalette::WindowText);

    // Neutral, theme-relative shades derived from the menu background by
    // nudging it toward the text colour -- this stays a grey hover (not the
    // accent) and follows a light/dark theme automatically.
    const QColor hover  = blend(bg, text, 0.15);
    const QColor sep    = blend(bg, text, 0.28);
    const QColor border = blend(bg, text, 0.40);

    return QString(
        "QMenu { background:%1; border:1px solid %2; }"
        "QMenu::item { padding:2px 28px 2px 12px; color:%3; }"
        "QMenu::item:selected { background:%4; color:%3; }"
        "QMenu::item:disabled { color:%5; }"
        "QMenu::separator { height:1px; background:%6; margin:3px 6px; }")
        .arg(bg.name(), border.name(), text.name(),
             hover.name(), dis.name(), sep.name());
}
