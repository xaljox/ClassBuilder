// qt/QtFieldStyle.cpp -- see QtFieldStyle.h.

#include "QtFieldStyle.h"

#include "CodeEditor.h"          // excluded: it has its own look

#include <QApplication>
#include <QEvent>
#include <QPlainTextEdit>
#include <QProxyStyle>
#include <QScopedValueRollback>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTextEdit>
#include <QWidget>
#ifdef __APPLE__
#include "QtSoftSelection.h"     // Qt_ThemeLineColor (the resting hairline)
#include <QAbstractScrollArea>
#include <QAbstractSpinBox>
#include <QColor>
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QPalette>
#include <QPen>
#endif

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

#ifdef __APPLE__
// A stand-alone single-line field -- a QLineEdit that draws its OWN frame, so
// NOT the one embedded in a combo box or spin box (those draw the field chrome
// on the container, and keep their native focus halo -- see the header note).
bool isSingleLineField(const QWidget* w)
{
    const auto* le = qobject_cast<const QLineEdit*>(w);
    if (!le)
        return false;
    const QWidget* p = le->parentWidget();
    return !qobject_cast<const QComboBox*>(p)
        && !qobject_cast<const QAbstractSpinBox*>(p);
}
#endif

// The fields this proxy is put on. On macOS that is BOTH text-field kinds (they
// need the drawn focus ring); elsewhere only the multi-line ones (for the frame).
bool isProxyField(const QWidget* w)
{
    if (isMultiLineField(w))
        return true;
#ifdef __APPLE__
    return isSingleLineField(w);
#else
    return false;
#endif
}

class CbFieldFrameStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt,
                       QPainter* p, const QWidget* w) const override
    {
        if (pe == PE_Frame && isMultiLineField(w) && !_inPanel)
        {
#ifdef __APPLE__
            // macOS: draw NO resting frame here. Routing the scroll-area frame
            // through the native panel drew a grey square line the single-line /
            // spin fields don't have (JV 2026-07-21). Just the focus ring; if a
            // grey line still shows it is Qt's own frame for the widget, killed at
            // the source (setFrameShape NoFrame in the installer).
            drawFieldFocus(opt, p, w);
            return;
#else
            // Linux / Windows: route the scroll-area frame through the line-edit
            // PANEL primitive so the multi-line field gets the SAME native frame
            // as a QLineEdit. Normalise the option to the plain control form.
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
#endif
        }
#ifdef __APPLE__
        // Single-line: native panel first, then the focus ring on top.
        if (pe == PE_PanelLineEdit && isSingleLineField(w))
        {
            QProxyStyle::drawPrimitive(pe, opt, p, w);
            drawFieldFocus(opt, p, w);
            return;
        }
#endif
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }

    int pixelMetric(PixelMetric m, const QStyleOption* opt,
                    const QWidget* w) const override
    {
        if (m == PM_DefaultFrameWidth && isMultiLineField(w))
        {
            // The frame margin doubles as the CLIP band the scroll area paints
            // its frame into. macOS: our 2.5px focus ring (inset 1.5 -> spans to
            // 2.75px) does not fit a 2px band, so its inner edge was clipped and
            // its rounded corners pinched -- the note ring read thinner/darker
            // than the single-line one. The ring is 3.5px (inset 1.75 -> reaches
            // 3.5px in), so widen the band to 4 to fit the WHOLE ring and let the
            // two field types match exactly (JV 2026-07-21). Linux/Windows route
            // through the native panel (no drawn ring) and keep 2.
#ifdef __APPLE__
            return 4;
#else
            return 2;
#endif
        }
        return QProxyStyle::pixelMetric(m, opt, w);
    }

#ifdef __APPLE__
private:
    // macOS only: draw the text-field focus ring. Root cause -- PROVEN with a
    // probe (JV 2026-07-21): once qApp has ANY stylesheet (CB always does:
    // menus / tooltips / group boxes), Qt wraps every widget in QStyleSheetStyle,
    // which DROPS the native macOS text-field focus halo (an AppKit NSView glow
    // drawn outside the paint path). A per-widget QMacStyle proxy does NOT bring
    // it back. Buttons / check / radio / spin / combo keep their halo because
    // theirs is drawn INSIDE the control cell -- so the text-entry widgets are
    // the ONLY controls that lose focus indication, and CB uses exactly two such
    // classes (QLineEdit, QPlainTextEdit), both covered here. Match the surviving
    // native halo: the accent at a soft alpha, ~2.5px, rounded. Live palette read.
    void drawFieldFocus(const QStyleOption* opt, QPainter* p,
                        const QWidget* w) const
    {
        Q_UNUSED(w);
        if (!(opt->state & State_HasFocus) || !(opt->state & State_Enabled))
            return;
        // The macOS focus halo is the accent drawn SEMI-TRANSPARENT (~50% alpha)
        // -- nothing more. Its visible colour is then the accent blended over
        // whatever is behind it, so it lightens over a white field and darkens
        // over a grey dialog: the natural behaviour, matching the native halo.
        // No darkening/saturation step: NSColor's keyboardFocusIndicatorColor is
        // a touch darker than controlAccentColor in sRGB (#007AFF -> #0067F4), but
        // Qt's QPalette::Highlight already reads the accent as ~#0A60FF, which
        // sits right on that halo colour -- so applying the darkening on top made
        // OUR ring visibly darker than the native one (measured, then corrected
        // against the running app; JV 2026-07-21). Straight accent @ 50% it is.
        // Qt reads the macOS accent a touch MORE saturated than the native
        // controls actually draw it (QPalette::Highlight #0A60FF vs the #0077F9
        // a native check box paints), so our ring came out slightly fuller than
        // the native halo. Measured over the same field backdrop: our ring
        // #8AACEC vs the native halo #90B8EF -- only ~12/255 greener needed.
        // Soften the accent's saturation a fraction to land on it; derived, so it
        // still tracks a changed accent (JV 2026-07-21).
        const QPalette pal = QApplication::palette();
        QColor accent = pal.color(QPalette::Active, QPalette::Highlight);
        float h = 0, s = 0, v = 0, a = 0;
        accent.getHsvF(&h, &s, &v, &a);
        QColor ring = QColor::fromHsvF(qMax(0.0f, h), s * 0.90f, v);

        // The multi-line ring sits fully over the grey dialog; the single-line
        // ring sits over the white field. A 40%-alpha ring composites DARKER over
        // grey, so the note read a touch darker than the single-line even at the
        // same colour. Pre-lighten the note's SOURCE colour by
        // (1-alpha)/alpha * (Base - Window) so that, once composited over its grey
        // backdrop, it lands on the same visible colour as the single-line over
        // white -- the two field kinds then read identically (JV 2026-07-21).
        if (isMultiLineField(w))
        {
            const QColor base = pal.color(QPalette::Active, QPalette::Base);
            const QColor win  = pal.color(QPalette::Active, QPalette::Window);
            // Half of the full (1-alpha)/alpha=1.5 compensation: full over-shot to
            // as-light-as-it-was-dark, so 0.75 centres the note on the native halo
            // (measured: #A8C5F2 at 1.5 vs native #90B8EF; JV 2026-07-21).
            const double k = 0.75;
            ring.setRed  (qBound(0, int(ring.red()   + k*(base.red()   - win.red())),   255));
            ring.setGreen(qBound(0, int(ring.green() + k*(base.green() - win.green())), 255));
            ring.setBlue (qBound(0, int(ring.blue()  + k*(base.blue()  - win.blue())),  255));
        }
        ring.setAlphaF(0.40);
        p->save();
        p->setRenderHint(QPainter::Antialiasing, true);
        p->setPen(QPen(ring, 3.5));      // the native halo's width
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(QRectF(opt->rect).adjusted(1.75, 1.75, -1.75, -1.75),
                           5, 5);
        p->restore();
    }
#endif

private:
    // Re-entrancy guard for the PE_Frame -> PE_PanelLineEdit routing: QMacStyle's
    // panel primitive calls back into PE_Frame, which would recurse without this.
    // Paint runs on the GUI thread only, so a plain member is safe.
    mutable bool _inPanel = false;
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
            if (isProxyField(w) && w->style() != fieldStyle())
                w->setStyle(fieldStyle());   // pointer check: no re-entry
#ifdef __APPLE__
            // Round the multi-line field's white area so its corners match the
            // rounded focus ring (and the single-line fields). The white is the
            // scroll area's VIEWPORT; a border-radius on it clips that background
            // rounded. Antialiased, unlike a QRegion mask (JV 2026-07-21).
            if (auto* sa = qobject_cast<QAbstractScrollArea*>(w))
                if (isMultiLineField(w))
                    sa->viewport()->setStyleSheet(
                        "border-radius: 5px; background: palette(base);");
#endif
        }
        return QObject::eventFilter(o, e);
    }
};

} // namespace

void Qt_InstallFieldFrameStyle()
{
    qApp->installEventFilter(new CbFieldStyleInstaller(qApp));
}
