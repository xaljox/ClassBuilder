// qt/CbPainter_QPainter.cpp -- see header.
//
// First Qt rendering of the SequenceDiagram shapes (M1). The shape Draw
// methods are unchanged from the CDC path; this backend translates each
// CbPainter primitive to QPainter + Qt vocabulary.

#include "CbPainter_QPainter.h"
#include "QtHandleMetrics.h"

#include <QBrush>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QPolygon>
#include <QRectF>

#include <cmath>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// CbColorRef is 0x00BBGGRR (Win32 BGR); QColor takes RGB.
QColor toQColor(CbColorRef c)
{
    return QColor(GetRValue(c), GetGValue(c), GetBValue(c));
}

// Win32 PS_* -> Qt::PenStyle.
Qt::PenStyle toPenStyle(int psStyle)
{
    switch (psStyle)
    {
    case PS_DASH:       return Qt::DashLine;
    case PS_DOT:        return Qt::DotLine;
    case PS_DASHDOT:    return Qt::DashDotLine;
    case PS_DASHDOTDOT: return Qt::DashDotDotLine;
    case PS_NULL:       return Qt::NoPen;
    case PS_SOLID:
    default:            return Qt::SolidLine;
    }
}

QRectF toQRectF(const CbRect& r)
{
    return QRectF(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

QString toQ(const CbString& s)
{
    return QString::fromLocal8Bit(static_cast<const char*>(s));
}

} // namespace (helpers above are file-local; fontForRole / fontForPx below are
  //            exported so the headless measure painter can reuse the mapping)

// The static diagram fonts were sized 32 in CreateFont (logical cell height);
// Qt's setPixelSize is direct-to-pixel and QFontMetrics::descent is a touch
// larger than Win32's at the same nominal size, so pixelSize 28 (32 - 4) is a
// close visual match. The same 4px offset applies to the note shapes' runtime
// GetFontHeight() (see fontForPx).
static constexpr int CB_QFONT_HEIGHT_OFFSET = 4;

// Map a CbFontRole (CBF_*) to its fixed QFont. The 10 model fonts are all Arial
// size 32, differing only by bold / italic / underline -- exactly what the old
// ClassDiagram::Init / SequenceDiagram::Init CreateFont calls set.
QFont fontForRole(int fontRole)
{
    QFont f("Arial");
    f.setPixelSize(32 - CB_QFONT_HEIGHT_OFFSET);
    switch (fontRole)
    {
    case CBF_CLASS_NAME:      f.setBold(true);                    break;
    case CBF_ABSTRACT_CLASS:  f.setBold(true); f.setItalic(true); break;
    case CBF_STATIC_MEMBER:
    case CBF_STATIC_METHOD:
    case CBF_LIFELINE:        f.setUnderline(true);               break;
    case CBF_MEMBER:
    case CBF_METHOD:
    case CBF_RELATION:
    case CBF_SIGNAL:
    case CBF_LABEL:
    default:                  /* Arial normal */                 break;
    }
    return f;
}

// An Arial-normal QFont at an explicit pixel size, for the note shapes whose
// size is a per-note runtime value (GetFontHeight()). Same -4 offset as above.
QFont fontForPx(int pixelSize)
{
    QFont f("Arial");
    f.setPixelSize(qMax(8, pixelSize - CB_QFONT_HEIGHT_OFFSET));
    return f;
}

CbPainter_QPainter::CbPainter_QPainter(QPainter* p) : _p(p) {}
CbPainter_QPainter::~CbPainter_QPainter() = default;

int CbPainter_QPainter::Save()
{
    _p->save();
    _gdiStateStack.push({ _textColor, _bkColor, _textAlign, _bkMode, _bold });
    return 0;       // QPainter::restore() is LIFO; the id is unused.
}

void CbPainter_QPainter::Restore(int)
{
    _p->restore();
    if (!_gdiStateStack.isEmpty())
    {
        const GdiState s = _gdiStateStack.pop();
        _textColor = s.textColor;
        _bkColor   = s.bkColor;
        _textAlign = s.textAlign;
        _bkMode    = s.bkMode;
        _bold      = s.bold;
    }
}

void CbPainter_QPainter::SetPen(int style, int width, CbColorRef color)
{
    // Only the style (solid/dash/dot) is recorded on the pen; the actual dash
    // rendering is hand-rolled in DrawLine, because a *cosmetic* pen that is
    // both wide and dashed renders at ~1px in Qt (a thick dashed highlight
    // would come out thin). Colour goes through forced() for the selection
    // highlight override.
    QPen pen(toQColor(forced(color)), width, toPenStyle(style));
    pen.setCosmetic(true);    // constant width under the zoom transform
    _p->setPen(pen);
}

CbColorRef CbPainter_QPainter::GetPenColor()
{
    const QColor c = _p->pen().color();
    return Cb_RGB(c.red(), c.green(), c.blue());
}

void CbPainter_QPainter::SetPenSolid()
{
    QPen p = _p->pen();
    p.setStyle(Qt::SolidLine);
    _p->setPen(p);
}

void CbPainter_QPainter::SetNullBrush()
{
    _p->setBrush(Qt::NoBrush);
}

void CbPainter_QPainter::SetSolidBrush(CbColorRef color)
{
    _p->setBrush(QBrush(toQColor(forced(color))));
}

void CbPainter_QPainter::SetFont(int fontRole)
{
    QFont f = fontForRole(fontRole);
    if (_bold)
        f.setBold(true);     // sticky bold survives a font change (see SetBold)
    _p->setFont(f);
}

void CbPainter_QPainter::SetFontPx(int pixelSize)
{
    QFont f = fontForPx(pixelSize);
    if (_bold)
        f.setBold(true);
    _p->setFont(f);
}

void CbPainter_QPainter::SetBold(bool bold)
{
    // Sticky: recorded so SetFont re-applies it. Needed because some shapes
    // (SignalShape) select their font inside a draw helper (GetNameRect) called
    // AFTER SetBold, which would otherwise reset the weight back to normal.
    // Saved/restored by Save()/Restore() (part of GdiState), so it can't leak
    // past a shape's own Draw.
    _bold = bold;
    QFont f = _p->font();
    f.setBold(bold);
    _p->setFont(f);
}

void CbPainter_QPainter::SetTextColor(CbColorRef color)
{
    _textColor = toQColor(color);
}

void CbPainter_QPainter::SetBkColor(CbColorRef color)
{
    _bkColor = toQColor(color);
}

CbColorRef CbPainter_QPainter::GetBkColor()
{
    return Cb_RGB(_bkColor.red(), _bkColor.green(), _bkColor.blue());
}

void CbPainter_QPainter::SetTextAlign(UINT flags)
{
    _textAlign = flags;
}

void CbPainter_QPainter::SetBkMode(int mode)
{
    _bkMode = mode;     // TRANSPARENT (1) or OPAQUE (2)
}

void CbPainter_QPainter::DrawLine(CbPoint start, CbPoint end)
{
    const QPen cur = _p->pen();
    if (cur.style() == Qt::SolidLine)
    {
        _p->drawLine(QPointF(start.x, start.y), QPointF(end.x, end.y));
        return;
    }

    // Hand-roll the dash as solid sub-segments. A cosmetic pen that is both
    // wide and dashed renders at ~1px in Qt, so a thick dashed highlight comes
    // out thin; drawing the dashes ourselves with a solid copy of the pen keeps
    // the full width. Lengths are in model units (as CbPainter_Cdc::DrawLine
    // does), so a thin line and a thick highlight over it dash identically and
    // align -- the wide green dashes fully cover the thin black ones.
    const double dx  = double(end.x) - double(start.x);
    const double dy  = double(end.y) - double(start.y);
    const double len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.0)
        return;
    const double ux     = dx / len;
    const double uy     = dy / len;
    const double period = (cur.style() == Qt::DotLine) ? 4.0 : 12.0;

    QPen solid = cur;
    solid.setStyle(Qt::SolidLine);
    _p->save();
    _p->setPen(solid);
    for (double d = 0.0; d < len; d += 2.0 * period)
    {
        const double e = (d + period < len) ? (d + period) : len;
        _p->drawLine(QPointF(start.x + ux * d, start.y + uy * d),
                     QPointF(start.x + ux * e, start.y + uy * e));
    }
    _p->restore();
}

void CbPainter_QPainter::Rectangle(CbRect rect)
{
    // Outlined + filled with current pen / brush -- matches CDC Rectangle.
    _p->drawRect(toQRectF(rect));
}

void CbPainter_QPainter::FillSolidRect(CbRect rect, CbColorRef color)
{
    _p->fillRect(toQRectF(rect), toQColor(color));
}

void CbPainter_QPainter::Ellipse(CbRect rect)
{
    _p->drawEllipse(toQRectF(rect));
}

void CbPainter_QPainter::Polygon(const CbPoint* points, int count)
{
    QPolygonF poly;
    poly.reserve(count);
    for (int i = 0; i < count; ++i)
        poly << QPointF(points[i].x, points[i].y);
    _p->drawPolygon(poly);
}

void CbPainter_QPainter::DrawSelectionHandle(CbPoint modelPt, CbColorRef fill)
{
    // Map the model point through the current world transform (fit/zoom/Y-flip),
    // then draw with the world matrix disabled so the square is sized in device
    // pixels (the QPainter analogue of the CDC LPtoDP + MM_TEXT dance in
    // CbPainter_Cdc::DrawSelectionHandle). The side eases with the apparent scale
    // (abs of the transform's scale term) -- capped when zoomed in, floored when
    // zoomed out -- so it doesn't dominate the shrinking shapes. Visible size +
    // grab tolerance live together in QtHandleMetrics.h so they stay consistent.
    const QTransform wt  = _p->transform();
    const QPointF    dev = wt.map(QPointF(modelPt.x, modelPt.y));
    const double     hs  = QtHandle::visibleSizeDev(qAbs(wt.m11()));
    _p->save();
    _p->setWorldMatrixEnabled(false);
    _p->setPen(QPen(QColor(0, 0, 0), 1));
    _p->setBrush(toQColor(fill));
    _p->drawRect(QRectF(dev.x() - hs / 2.0, dev.y() - hs / 2.0, hs, hs));
    _p->restore();
}

// Draw `text` at (x, y) under the current TA_* anchor.
//
// IMPORTANT: the caller's world transform usually has scale(1, -1) (to flip
// Y so the model's Y-up coords map to screen Y-down). Qt's QPainter applies
// the world transform to text glyphs -- under a negative Y scale the glyphs
// come out *mirrored*. Win32 GDI under MM_ISOTROPIC with a negative viewport
// extent doesn't do that; it flips text *positions* via the mapping, but
// draws each glyph in normal screen orientation. To match, we locally
// counter-flip the Y axis at the anchor point so the glyphs render upright.
// The X scale is preserved, so the font's pixel size still scales uniformly
// with the rest of the diagram.
//
// After the counter-flip, local Y points DOWN (Qt text convention): baseline
// is at local y=0, ascenders are above (negative local y), descenders below
// (positive local y). The TA_* anchor offsets are computed in that frame.
void CbPainter_QPainter::drawTextAtAnchor(int x, int y, const CbString& text)
{
    const QString q = toQ(text);
    const QFontMetricsF fm(_p->font());

    qreal offsetX = 0;
    if ((_textAlign & TA_CENTER) == TA_CENTER)
        offsetX = -fm.horizontalAdvance(q) / 2.0;
    else if ((_textAlign & TA_CENTER) == TA_RIGHT)
        offsetX = -fm.horizontalAdvance(q);

    // Win32 vertical-anchor bits live in TA_BASELINE's mask (== 24):
    //   TA_TOP      (0)  -> top of bounding box at the anchor
    //   TA_BOTTOM   (8)  -> bottom of bounding box at the anchor
    //   TA_BASELINE (24) -> baseline at the anchor
    // TA_BOTTOM is a SUBSET of TA_BASELINE's bits, so a plain `& TA_BASELINE`
    // matches both -- compare against the full mask instead.
    const UINT vAnchor = _textAlign & TA_BASELINE;

    qreal offsetY = fm.ascent();            // TA_TOP default
    if (vAnchor == TA_BASELINE)
        offsetY = 0;
    else if (vAnchor == TA_BOTTOM)
    {
        // CDC's tmDescent runs a hair tighter than Qt's QFontMetrics::
        // descent() on the same nominal Arial size, so a strict -descent
        // puts descenders ('p', 'g', ...) flush on the anchor line.
        // ~2 device pixels of lift (in font space, divided by the current
        // world scale so the gap stays constant at any zoom) keeps a
        // visible hairline gap below the text -- preferred over flush by
        // user feedback (top-side gap is a separate model-layout knob:
        // the activation start offset on the lifeline).
        const qreal devScale = qAbs(_p->worldTransform().m11());
        const qreal liftFontPx = devScale > 0 ? 2.0 / devScale : 0;
        offsetY = -fm.descent() - liftFontPx;
    }

    _p->save();
    _p->translate(x, y);
    _p->scale(1, -1);                       // counter-flip the world Y-flip
    _p->setPen(_textColor);
    _p->drawText(QPointF(offsetX, offsetY), q);
    _p->restore();
}

void CbPainter_QPainter::TextOut(int x, int y, const CbString& text)
{
    drawTextAtAnchor(x, y, text);
}

void CbPainter_QPainter::ExtTextOut(int x, int y, UINT options,
                                    const CbRect& clipRect,
                                    const CbString& text)
{
    _p->save();

    if (options & ETO_OPAQUE)
        _p->fillRect(toQRectF(clipRect), _bkColor);

    if (options & ETO_CLIPPED)
        _p->setClipRect(toQRectF(clipRect), Qt::IntersectClip);

    drawTextAtAnchor(x, y, text);

    _p->restore();
}

// Win32 DrawText with DT_CALCRECT -- update `rect` to the bounds the text
// would draw to under `format`. We honour the common subset:
//   DT_LEFT/DT_CENTER/DT_RIGHT, DT_NOCLIP, DT_EXPANDTABS, DT_EXTERNALLEADING,
//   DT_NOPREFIX, DT_SINGLELINE. Anything else uses Qt defaults.
void CbPainter_QPainter::CalcText(const CbString& text, CbRect& rect,
                                  UINT format)
{
    int flags = 0;
    if (format & DT_CENTER)         flags |= Qt::AlignHCenter;
    else if (format & DT_RIGHT)     flags |= Qt::AlignRight;
    else                            flags |= Qt::AlignLeft;
    if (format & DT_VCENTER)        flags |= Qt::AlignVCenter;
    else if (format & DT_BOTTOM)    flags |= Qt::AlignBottom;
    else                            flags |= Qt::AlignTop;
    if (format & DT_SINGLELINE)     flags |= Qt::TextSingleLine;
    if (format & DT_EXPANDTABS)     flags |= Qt::TextExpandTabs;
    if (format & DT_NOPREFIX)       flags |= Qt::TextHideMnemonic;
    if (format & DT_WORDBREAK)      flags |= Qt::TextWordWrap;

    const QFontMetricsF fm(_p->font());
    QRectF bounds = fm.boundingRect(toQRectF(rect), flags, toQ(text));

    rect.left   = int(bounds.left());
    rect.top    = int(bounds.top());
    rect.right  = int(bounds.right());
    rect.bottom = int(bounds.bottom());
}

CbSize CbPainter_QPainter::GetTextExtent(const CbString& text)
{
    const QFontMetricsF fm(_p->font());
    return CbSize(int(fm.horizontalAdvance(toQ(text))),
                  int(fm.height()));
}
