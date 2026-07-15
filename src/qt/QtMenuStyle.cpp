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
    const QColor bg   = pal.color(QPalette::Active,   QPalette::Window);
    const QColor text = pal.color(QPalette::Active,   QPalette::WindowText);

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
