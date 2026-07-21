// qt/QtFieldStyle.cpp -- see QtFieldStyle.h.

#include "QtFieldStyle.h"

#include "CodeEditor.h"          // excluded: it has its own look

#include <QApplication>
#include <QEvent>
#include <QPlainTextEdit>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>

namespace {

// A multi-line INPUT FIELD -- the widgets that should read like a QLineEdit.
// CodeEditor is deliberately out: it is a QPlainTextEdit subclass with its own
// chrome (current-line wash, the //@CODE marker bands) and must keep it.
bool isMultiLineField(const QWidget* w)
{
    if (!w || qobject_cast<const CodeEditor*>(w))
        return false;
    return qobject_cast<const QTextEdit*>(w)
        || qobject_cast<const QPlainTextEdit*>(w);
}

class CbFieldFrameStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt,
                       QPainter* p, const QWidget* w) const override
    {
        if (pe == PE_Frame && isMultiLineField(w))
        {
            // The scroll-area frame becomes the line-edit PANEL. QFrame asks for
            // a sunken panel of its own frame width; PE_PanelLineEdit wants the
            // plain control option, so normalise those fields and let the style
            // do the rest -- the focus state is already in opt->state (Fusion
            // simply ignores it for PE_Frame, which is the whole bug).
            QStyleOptionFrame f;
            if (const auto* src = qstyleoption_cast<const QStyleOptionFrame*>(opt))
                f = *src;
            else
                f.QStyleOption::operator=(*opt);
            f.state       &= ~State_Sunken;
            f.lineWidth    = 1;
            f.midLineWidth = 0;
            f.features     = QStyleOptionFrame::None;
            QProxyStyle::drawPrimitive(PE_PanelLineEdit, &f, p, w);
            return;
        }
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }

    int pixelMetric(PixelMetric m, const QStyleOption* opt,
                    const QWidget* w) const override
    {
        // The panel paints TWO rows -- the outline and an inner contrast line.
        // A scroll area insets its viewport by the frame width, so at the
        // default 1 the viewport repainted straight over that second row and the
        // frame came out visibly thinner than a QLineEdit's. Reporting 2 keeps
        // both rows visible; measured identical to a focused QLineEdit
        // afterwards (JV 2026-07-21).
        if (m == PM_DefaultFrameWidth && isMultiLineField(w))
            return 2;
        return QProxyStyle::pixelMetric(m, opt, w);
    }
};

// The one shared instance, kept for the process lifetime (setStyle does NOT
// take ownership, so it must outlive every field).
QStyle* fieldStyle()
{
    static CbFieldFrameStyle* s = nullptr;
    if (!s)
    {
        // A FRESH base instance, not the live application style: a proxy owns
        // its base, and the running one is owned elsewhere. Same pattern as
        // ShellSeparatorStyle in QtShellWindow.
        s = new CbFieldFrameStyle(
                QStyleFactory::create(QApplication::style()->name()));
        s->setParent(qApp);
    }
    return s;
}

// Assign the style to each field as it is polished.
//
// It must go on the WIDGET, not on the application: QtShellWindow sets its own
// ShellSeparatorStyle on the shell, and Qt propagates a widget style down the
// whole child tree -- so an application-level style is never consulted for
// anything inside that window, and the fields kept their flat, focus-blind
// frame. A style set on the widget itself wins over the inherited one
// (measured, JV 2026-07-21).
class CbFieldStyleInstaller : public QObject
{
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject* o, QEvent* e) override
    {
        if (e->type() == QEvent::Polish)
        {
            auto* w = qobject_cast<QWidget*>(o);
            if (isMultiLineField(w) && w->style() != fieldStyle())
                w->setStyle(fieldStyle());   // pointer check: no re-entry
        }
        return QObject::eventFilter(o, e);
    }
};

} // namespace

void Qt_InstallFieldFrameStyle()
{
    qApp->installEventFilter(new CbFieldStyleInstaller(qApp));
}
