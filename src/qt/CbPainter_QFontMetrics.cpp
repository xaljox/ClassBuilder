// qt/CbPainter_QFontMetrics.cpp -- see header. Measure-only CbPainter; the
// CalcText / GetTextExtent bodies mirror CbPainter_QPainter exactly (same
// QFontMetricsF formulas) so off-view rects match the painted ones.

#include "CbPainter_QFontMetrics.h"
#include "CbPainter_QPainter.h"   // fontForRole / fontForPx

#include <QFontMetricsF>
#include <QRectF>
#include <QString>

// windows.h for the DT_* / TA_* flags the model passes through.

namespace {
QString toQ(const CbString& s)
{
    return QString::fromLocal8Bit(static_cast<const char*>(s));
}
}

CbPainter_QFontMetrics::CbPainter_QFontMetrics()
    : _font(fontForPx(32))   // sensible Arial-normal default until SetFont
{}

CbPainter_QFontMetrics::~CbPainter_QFontMetrics() = default;

int CbPainter_QFontMetrics::Save()
{
    _stack.append(_font);
    return _stack.size();
}

void CbPainter_QFontMetrics::Restore(int /*savedId*/)
{
    if (!_stack.isEmpty())
    {
        _font = _stack.takeLast();
    }
}

void CbPainter_QFontMetrics::SetFont(int fontRole)
{
    _font = fontForRole(fontRole);
}

void CbPainter_QFontMetrics::SetFontPx(int pixelSize)
{
    _font = fontForPx(pixelSize);
}

void CbPainter_QFontMetrics::CalcText(const CbString& text, CbRect& rect,
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

    const QFontMetricsF fm(_font);
    const QRectF in(rect.left, rect.top,
                    rect.right - rect.left, rect.bottom - rect.top);
    QRectF bounds = fm.boundingRect(in, flags, toQ(text));

    rect.left   = int(bounds.left());
    rect.top    = int(bounds.top());
    rect.right  = int(bounds.right());
    rect.bottom = int(bounds.bottom());
}

CbSize CbPainter_QFontMetrics::GetTextExtent(const CbString& text)
{
    const QFontMetricsF fm(_font);
    return CbSize(int(fm.horizontalAdvance(toQ(text))),
                  int(fm.height()));
}
