// qt/QtMenuStyle.cpp -- compact right-click-menu stylesheet, theme-derived.

#include "QtMenuStyle.h"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QMenu>
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
    // Base (white), not Window (the darker grey): matches the app-wide QMenu
    // rule (QtApp.cpp) so the code-editor menus have the SAME background as the
    // tree / diagram context menus (JV 2026-07-18).
    const QColor bg   = pal.color(QPalette::Active,   QPalette::Base);
    const QColor text = pal.color(QPalette::Active,   QPalette::Text);

    // Disabled text: the palette's Disabled role -- EXCEPT on macOS, where
    // Qt leaves it (nearly) equal to the active text (the native style greys
    // by other means, which a stylesheet bypasses). If it doesn't visibly
    // differ, derive a grey from the theme instead.
    QColor dis = pal.color(QPalette::Disabled, QPalette::WindowText);
    const int diff = qAbs(dis.red()   - text.red()) +
                     qAbs(dis.green() - text.green()) +
                     qAbs(dis.blue()  - text.blue());
    if (diff < 48)
        dis = blend(bg, text, 0.45);

    // Separator + border: greys DERIVED from the menu background toward the text
    // colour, never a palette role. No role is reliable here -- measured on
    // Ubuntu 26.04 / GNOME (Fusion) the `mid` role is #ffffff while the menu
    // background (Base) is #fcfcfc, so a `palette(mid)` separator painted WHITE
    // on near-white: the gap was reserved but no line was visible (JV
    // 2026-07-21). Deriving from the background guarantees contrast in a light
    // AND a dark theme by construction.
    const QColor sep    = blend(bg, text, 0.28);
    const QColor border = blend(bg, text, 0.40);

    // The accent parts stay palette(...) REFS, not baked hex: Qt resolves them
    // live, so menus keep following an accent change even in the app-wide sheet
    // (built once at startup). Only the derived greys above have to be concrete.
    // `background: transparent` on ::item is deliberate -- the Windows 11 style
    // otherwise paints the item its own flat grey over the menu background.
    return QString(
        "QMenu { background:palette(base); border:1px solid %1; padding:2px; }"
        "QMenu::item { padding:2px 28px 2px 12px; color:palette(text);"
        " background:transparent; }"
        "QMenu::item:selected { background:palette(highlight);"
        " color:palette(highlighted-text); }"
        "QMenu::item:disabled { color:%2; }"
        "QMenu::separator { height:1px; background:%3; margin:3px 6px; }")
        .arg(border.name(), dis.name(), sep.name());
}

namespace {
// Repaints the whole menu on any hover movement. A styled QMenu on a Retina
// display repaints hover changes partially, and the update regions land on
// half-device-pixels -- 1px residue lines stay behind at row edges (WHERE
// depends on exact row geometry; nudging paddings only moves the spots).
// A full repaint per hover move erases them by construction; the popups are
// tiny, so the cost is nil.
class MenuRepaintFilter : public QObject
{
public:
    explicit MenuRepaintFilter(QMenu* menu)
        : QObject(menu), _menu(menu)
    {
        menu->installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        switch (event->type())
        {
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::HoverLeave:
        case QEvent::Leave:
            _menu->update();
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QMenu* _menu;
};
}

void Qt_ApplyCompactMenuStyle(QMenu* menu)
{
    menu->setStyleSheet(Qt_CompactMenuStyleSheet());
    new MenuRepaintFilter(menu);    // parented to the menu, dies with it
}
