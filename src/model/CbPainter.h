#ifndef _CBPAINTER_H
#define _CBPAINTER_H
//
// CbPainter.h -- abstract paint interface, MFC-free and Qt-free.
//
// The diagram shapes draw through this rather than directly to a CDC, so the
// same Draw(CbPainter&) code paths render to either a Win32 CDC (the MFC
// view, print, EMF clipboard -- CbPainter_Cdc) or a Qt QPainter (the
// future Qt-side view -- CbPainter_QPainter). The interface is the slim
// subset of operations the shape Draw methods actually use; it mirrors GDI
// vocabulary (because that is what the shapes were written against) but
// without any MFC types -- pure value parameters built on CbGeometry +
// CbColorRef.
//
// First consumer: the SequenceDiagram shapes (M0 of the SD-view port; see
// project_classbuilder_qt_port_sequencediagram_plan in auto-memory). The
// existing static `Shape::DrawLine(CDC*, ...)` helper -- which works around
// CDC's buggy PS_DOT/PS_DASH dashing by drawing each segment by hand -- is
// pulled inside CbPainter_Cdc::DrawLine; CbPainter_QPainter::DrawLine will
// just use QPen styles, which Qt renders correctly.
//
// FONTS are selected by ROLE (a CbFontRole / CBF_* value, passed as int so
// this header stays free of the model enum). Each backend maps the role to a
// fixed Arial font (size + style bits); SetFontPx covers the note shapes whose
// size is a per-note runtime value. Replaced the earlier void* CFont* handle
// scheme when the MFC CFont statics were removed.
//
#include "CbGeometry.h"
#include "CbString.h"
#include "CbColor.h"      // CbColorRef + Cb_RGB/Cb_GetR/G/B (windows.h-free)

// WIN32_LEAN_AND_MEAN keeps GDI (wingdi.h: TA_*/ETO_*/TRANSPARENT) but excludes
// winspool.h, whose DocumentProperties->DocumentPropertiesW A/W macro otherwise
// clobbers Qt's QIcon::ThemeIcon::DocumentProperties in any qt TU that pulls
// this header. NOMINMAX keeps min/max macros out of std::min/std::max.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>      // UINT, TRANSPARENT/OPAQUE, TA_*, ETO_* (colour moved to CbColor.h)

// `<windows.h>` defines a DrawText macro (DrawTextA/W) that would mangle the
// CbPainter::CalcText helpers if we used the name DrawText; we already use
// CalcText so this is moot, but undef anyway to keep the header friendly.
#ifdef DrawText
#  undef DrawText
#endif

class CbPainter
{
public:
    virtual ~CbPainter() = default;

    // --- State stack (mirrors CDC SaveDC / RestoreDC) ---------------------
    // Save returns an id; Restore(-1) pops the most recent save.
    virtual int  Save() = 0;
    virtual void Restore(int savedId = -1) = 0;

    // --- Pen --------------------------------------------------------------
    // `style` follows the Win32 PS_* values (PS_SOLID, PS_DOT, PS_DASH).
    // Width 1 is the standard hairline pen the shapes always use.
    virtual void SetPen(int style, int width, CbColorRef color) = 0;

    // Current pen colour (the colour passed to the last SetPen). Lets a Draw
    // body force a pen STYLE (e.g. a solid arrowhead over a dashed line) while
    // keeping whatever colour the caller set -- so a selection re-draw that
    // recolours the pen recolours that sub-part too. Default black.
    virtual CbColorRef GetPenColor() { return Cb_RGB(0, 0, 0); }

    // Switch the current pen to solid style, KEEPING its colour and width.
    // Lets a Draw force a sub-part solid (e.g. an arrowhead over a dashed
    // line) while still using the pen the caller supplied -- so a selection
    // re-draw's thicker, recoloured pen carries through to that sub-part
    // instead of being thrown away. Default no-op.
    virtual void SetPenSolid() {}

    // Bold the CURRENT font, and stay sticky for subsequent SetFont calls
    // until cleared, so a selected shape can emphasise ALL its text even when
    // a draw helper re-selects a font mid-way (e.g. SignalShape::GetNameRect).
    // Saved/restored by Save()/Restore(). Default no-op (CDC path unaffected).
    virtual void SetBold(bool bold) {}

    // --- Brush ------------------------------------------------------------
    virtual void SetNullBrush() = 0;            // SelectStockObject(NULL_BRUSH)
    virtual void SetSolidBrush(CbColorRef color) = 0;

    // --- Font -------------------------------------------------------------
    // Select one of the fixed model fonts by ROLE -- a CbFontRole value (CBF_*),
    // passed as int so this low-level header needn't pull the model enum. The
    // backend maps the role to a fixed QFont (Arial + size + style bits).
    // SetFontPx picks an Arial-normal font at an explicit pixel size, for the
    // note shapes whose size is a per-note runtime value (GetFontHeight()).
    virtual void SetFont(int fontRole) = 0;
    virtual void SetFontPx(int pixelSize) = 0;

    // --- Text attributes --------------------------------------------------
    virtual void     SetTextColor(CbColorRef color) = 0;
    virtual void     SetBkColor(CbColorRef color) = 0;
    virtual CbColorRef GetBkColor() = 0;
    virtual void     SetTextAlign(UINT flags) = 0;
    virtual void     SetBkMode(int mode) = 0;   // TRANSPARENT / OPAQUE

    // --- Drawing primitives ----------------------------------------------
    // DrawLine handles dashed/dotted lines correctly per backend (CDC needs
    // manual segmenting, QPainter does not). DrawRect is just 4 DrawLines.
    virtual void DrawLine(CbPoint start, CbPoint end) = 0;
    // Default: the rectangle outline as 4 DrawLine calls (so each backend's
    // own dash/dot handling applies). Inline here so the base class needs no
    // .cpp of its own.
    virtual void DrawRect(CbRect rect)
    {
        DrawLine(CbPoint(rect.left,  rect.top),    CbPoint(rect.right, rect.top));
        DrawLine(CbPoint(rect.right, rect.top),    CbPoint(rect.right, rect.bottom));
        DrawLine(CbPoint(rect.right, rect.bottom), CbPoint(rect.left,  rect.bottom));
        DrawLine(CbPoint(rect.left,  rect.bottom), CbPoint(rect.left,  rect.top));
    }

    // Rectangle: outlined with the current pen, filled with the current
    // brush (so NULL_BRUSH gives an outline only).
    virtual void Rectangle(CbRect rect) = 0;

    // FillSolidRect: GDI-style fill without an outline, takes the colour
    // directly (does not touch the current brush).
    virtual void FillSolidRect(CbRect rect, CbColorRef color) = 0;

    virtual void Ellipse(CbRect rect) = 0;
    virtual void Polygon(const CbPoint* points, int count) = 0;

    // Selection handle: a small fixed-size square centred on a MODEL point,
    // drawn in DEVICE coordinates so it stays a constant pixel size at any
    // zoom -- the shared primitive lifted out of the (formerly duplicated)
    // per-shape DrawSelectedRect bodies on both diagram families. The CDC
    // backend does the LPtoDP + MM_TEXT/viewport-reset dance; the QPainter
    // backend maps through the world transform then draws with it disabled.
    // 13x13 px (6 above/left, 7 below/right of the point), black 1px outline,
    // `fill` interior. Default no-op for non-drawing backends (e.g. the
    // headless text-measure painter).
    virtual void DrawSelectionHandle(CbPoint /*modelPt*/, CbColorRef /*fill*/) {}

    // --- Text rendering + measurement -------------------------------------
    // Plain text at (x, y) with the current font + text alignment.
    virtual void TextOut(int x, int y, const CbString& text) = 0;

    // ExtTextOut: `options` carries ETO_CLIPPED / ETO_OPAQUE. `clipRect` is
    // honoured only when ETO_CLIPPED is set.
    virtual void ExtTextOut(int x, int y, UINT options,
                            const CbRect& clipRect,
                            const CbString& text) = 0;

    // CalcText: DT_CALCRECT-style measurement -- `rect` is updated to the
    // text's bounds under `format` (DT_LEFT | DT_NOCLIP | ...). Used by
    // GetLabelRect / GetNameRect etc. before painting.
    virtual void CalcText(const CbString& text, CbRect& rect, UINT format) = 0;

    // GetTextExtent: width/height of `text` in the current font.
    virtual CbSize GetTextExtent(const CbString& text) = 0;

    // --- Backend introspection -------------------------------------------
    // True for an interactive screen surface; false for print / metafile,
    // where selection highlights and similar transient cues are suppressed.
    virtual bool IsScreen() = 0;

    // --- Colour override (selection highlight) ---------------------------
    // While active, SetPen / SetSolidBrush ignore their colour argument and
    // use this colour instead (style + width unchanged). Lets a selection
    // re-draw recolour a whole shape's own Draw -- including sub-parts that
    // hardcode their colour (e.g. the dependency arrowhead) -- without
    // touching the shape code. Off by default.
    void SetForceColor(CbColorRef c) { _forceColor = true; _forcedColor = c; }
    void ClearForceColor()         { _forceColor = false; }

    // App-wide selection colours, set once by the GUI layer (the Qt app from
    // its palette accent; the MFC side can leave the defaults). A shape drawing
    // itself selected uses the SOLID colour for pens / outlines and the FILL
    // colour for interiors. Function-local statics so no separate .cpp needed.
    static void     SetSelectColor(CbColorRef c)     { selectColorRef()     = c; }
    static CbColorRef GetSelectColor()               { return selectColorRef(); }
    static void     SetSelectFillColor(CbColorRef c) { selectFillColorRef() = c; }
    static CbColorRef GetSelectFillColor()           { return selectFillColorRef(); }

    // App-wide headless text-measure painter, installed once by the GUI layer
    // (the Qt app creates a long-lived CbPainter_QFontMetrics at startup). The
    // model-side layout methods that need font metrics without a view -- lifeline
    // auto-width, class auto-size, SequenceDiagram::OptimizePlacement, the signal
    // text hit-test -- measure through this instead of a view DC. Null until
    // installed, so callers fall back (current width / skip). Replaces the old
    // CClientDC(view) + CbPainter_Cdc measurement once the MFC views are gone.
    static void       SetMeasurePainter(CbPainter* p) { measurePainterRef() = p; }
    static CbPainter* GetMeasurePainter()             { return measurePainterRef(); }

protected:
    static CbPainter*& measurePainterRef()
    {
        static CbPainter* p = nullptr;             // installed by the Qt app
        return p;
    }

    static CbColorRef& selectColorRef()
    {
        static CbColorRef c = Cb_RGB(0, 128, 0);       // solid: pens / outlines
        return c;
    }
    static CbColorRef& selectFillColorRef()
    {
        static CbColorRef c = Cb_RGB(200, 255, 200);   // light: interiors
        return c;
    }

    CbColorRef forced(CbColorRef c) const { return _forceColor ? _forcedColor : c; }

    bool     _forceColor  = false;
    CbColorRef _forcedColor = 0;
};

#endif
