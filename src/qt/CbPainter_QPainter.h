// qt/CbPainter_QPainter.h -- CbPainter backend that wraps a QPainter.
//
// The Qt counterpart of CbPainter_Cdc; used by the Qt-side SequenceDiagram
// view (M1+ of the SD-view port). Pointer to the QPainter is borrowed, not
// owned -- the caller keeps it alive for the lifetime of the painter.
//
// SetFont(role) takes a CbFontRole (CBF_*) value (as int) and selects the
// matching fixed QFont; SetFontPx takes an explicit pixel size (the note
// shapes' runtime GetFontHeight()).
#pragma once

#include "CbPainter.h"

#include <QColor>
#include <QFont>
#include <QStack>

class QPainter;

// Maps a CbFontRole (CBF_*) value to its fixed QFont; fontForPx builds an
// Arial-normal QFont at an explicit pixel size. Shared with the headless
// measure painter (CbPainter_QFontMetrics) so both backends size text
// identically.
QFont fontForRole(int fontRole);
QFont fontForPx(int pixelSize);

class CbPainter_QPainter : public CbPainter
{
public:
    explicit CbPainter_QPainter(QPainter* p);
    ~CbPainter_QPainter() override;

    int  Save() override;
    void Restore(int = -1) override;

    void SetPen(int style, int width, CbColorRef color) override;
    CbColorRef GetPenColor() override;
    void     SetPenSolid() override;
    void SetNullBrush() override;
    void SetSolidBrush(CbColorRef color) override;

    void SetFont(int fontRole) override;
    void SetFontPx(int pixelSize) override;
    void SetBold(bool bold) override;

    void     SetTextColor(CbColorRef color) override;
    void     SetBkColor(CbColorRef color) override;
    CbColorRef GetBkColor() override;
    void     SetTextAlign(UINT flags) override;
    void     SetBkMode(int mode) override;

    void DrawLine(CbPoint start, CbPoint end) override;
    void Rectangle(CbRect rect) override;
    void FillSolidRect(CbRect rect, CbColorRef color) override;
    void Ellipse(CbRect rect) override;
    void Polygon(const CbPoint* points, int count) override;
    void DrawSelectionHandle(CbPoint modelPt, CbColorRef fill) override;

    void TextOut(int x, int y, const CbString& text) override;
    void ExtTextOut(int x, int y, UINT options,
                    const CbRect& clipRect,
                    const CbString& text) override;
    void   CalcText(const CbString& text, CbRect& rect, UINT format) override;
    CbSize GetTextExtent(const CbString& text) override;

    bool IsScreen() override { return _isScreen; }

    // Off for an off-screen render (SVG export / print / metafile): the shapes
    // gate selection highlights and other transient screen-only cues on
    // IsScreen(), so a clean export turns this off. On (screen) by default.
    void SetScreen(bool isScreen) { _isScreen = isScreen; }

private:
    void drawTextAtAnchor(int x, int y, const CbString& text);

    bool _isScreen = true;

    QPainter* _p;
    QColor    _textColor   = QColor(0, 0, 0);
    QColor    _bkColor     = QColor(255, 255, 255);
    UINT      _textAlign   = 0;       // TA_LEFT | TA_TOP
    int       _bkMode      = 1;       // TRANSPARENT
    bool      _bold        = false;   // sticky bold, applied on every SetFont

    // GDI emulation: these fields are NOT part of QPainter's own save()/restore()
    // stack, but the shape Draw bodies were written against CDC SaveDC/RestoreDC
    // semantics (which DO restore text/bk colour + align + mode). Save()/Restore()
    // push/pop them here so a shape that recolours for selection can't leak its
    // bk/text colour onto the next shape drawn (was: selection bled across shapes).
    struct GdiState { QColor textColor; QColor bkColor; UINT textAlign; int bkMode; bool bold; };
    QStack<GdiState> _gdiStateStack;
};
