// qt/CbPainter_QFontMetrics.h -- a measure-only CbPainter backend.
//
// The model's text-rect helpers (SignalShape::GetNameRect / GetLabelRect /
// GetReturnRect, SDNoteShape wrap, ...) take a CbPainter& and measure via
// SetFont + GetTextExtent / CalcText. The MFC path builds that painter from a
// CClientDC borrowed from the view; the Qt canvas has no view to lend. This
// backend measures with QFontMetrics and no-ops every drawing call, so those
// rects can be computed off-view -- signal text hit-test, sub-part drag, and
// the pipe no-view layout path. Same fontForRole() mapping as the on-screen
// CbPainter_QPainter, so measured rects match the painted ones.
#pragma once

#include "CbPainter.h"

#include <QFont>
#include <QList>

class CbPainter_QFontMetrics : public CbPainter
{
public:
    CbPainter_QFontMetrics();
    ~CbPainter_QFontMetrics() override;

    int  Save() override;
    void Restore(int savedId = -1) override;

    // Drawing primitives are all no-ops -- this backend only measures.
    void SetPen(int, int, CbColorRef) override {}
    void SetNullBrush() override {}
    void SetSolidBrush(CbColorRef) override {}

    void SetFont(int fontRole) override;
    void SetFontPx(int pixelSize) override;

    void     SetTextColor(CbColorRef) override {}
    void     SetBkColor(CbColorRef) override {}
    CbColorRef GetBkColor() override { return Cb_RGB(255, 255, 255); }
    void     SetTextAlign(UINT) override {}
    void     SetBkMode(int) override {}

    void DrawLine(CbPoint, CbPoint) override {}
    void Rectangle(CbRect) override {}
    void FillSolidRect(CbRect, CbColorRef) override {}
    void Ellipse(CbRect) override {}
    void Polygon(const CbPoint*, int) override {}

    void TextOut(int, int, const CbString&) override {}
    void ExtTextOut(int, int, UINT, const CbRect&, const CbString&) override {}
    void   CalcText(const CbString& text, CbRect& rect, UINT format) override;
    CbSize GetTextExtent(const CbString& text) override;

    bool IsScreen() override { return true; }

private:
    QFont        _font;
    QList<QFont> _stack;   // Save()/Restore() of the current font only
};
