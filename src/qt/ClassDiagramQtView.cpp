// qt/ClassDiagramQtView.cpp -- see header.
//
// M1: read-only, non-modal. The canvas builds a ClassDiagramViewModel that
// wires the model's update path back to a Qt repaint via a static C-style
// callback. The same shape Draw methods the MFC view calls run here; only the
// CbPainter backend changes. Mirrors the read-only subset of
// qt/SequenceDiagramQtView.cpp.

#include "ClassDiagramQtView.h"

#include "QtClassDiagramView.h"   // bridge declaration
#include "QtApp.h"                // Qt_EnsureApplication / Qt_ShowModeless
#include "QtModelText.h"          // toQ
#include "QtRelationDiagramOnlyDialog.h"  // Qt_CreateRelationDiagramOnlyDialog
#include "QtDependencyDialog.h"           // Qt_CreateDependencyDialog
#include "QtSelectClassesDialog.h"        // Qt_ShowSelectClassesDialog
#include "QtClassShapeDialog.h"           // Qt_ShowClassShapeDialog
#include "QtSelectMembersAndMethods.h"    // Qt_ShowSelectMembersAndMethodsDialog
#include "QtClassShapeOrderDialog.h"      // Qt_ShowClassShapeOrderDialog (Reorder)
#include "QtInheritTreeDialog.h"          // Qt_ShowInheritsFromDialog / ...InheritedBy
#include "QtUserSectionsDialog.h"         // Qt_ShowUserSectionsDialog (Edit User Sections)
#include "QtHandleMetrics.h"              // QtHandle::grabToleranceModel
#include "QtDiagramZoom.h"                // Zoom-toolbar routing registry
#include "QtModelIcons.h"                 // Qt_ModelIcon
#include "QtToolBarIcons.h"               // Qt_ToolBarIcon (real MFC toolbar glyphs)
#include "CbPainter_QPainter.h"
#include "CbPainter_QFontMetrics.h"      // headless text measurement (conn-text hit/ghost)

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QNativeGestureEvent>
#include <QRubberBand>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollBar>
#include <QtGlobal>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QToolButton>
#include <QDockWidget>
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QVector>
#include <QFileDialog>
#include <QMessageBox>
#include <QtMath>
#ifdef CB_HAVE_SVG
#include <QSvgGenerator>
#endif


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

// ---------------------------------------------------------------------------
// Document-level colour templates -- the "Color Templates" submenu of the
// context menu. Mirrors the MFC "ClassDiagram > Color Templates" popup
// (ClassBuilder.rc) and the SD canvas's kTemplateColors table. Each entry
// routes to a DataModelDoc getter/setter member-function pair.
// ---------------------------------------------------------------------------
namespace {
struct TemplateColorEntry {
    const char* title;
    CbColorRef (DataModelDoc::*get)() const;
    void     (DataModelDoc::*set)(CbColorRef);
    bool separatorBefore;   // group divider (class / member-method / relation / note)
};
const TemplateColorEntry kTemplateColors[] = {
    { "Change Class Line Color",                 &DataModelDoc::GetClassPenColor,                &DataModelDoc::SetClassPenColor,                false },
    { "Change Class Text Color",                 &DataModelDoc::GetClassTextColor,               &DataModelDoc::SetClassTextColor,               false },
    { "Change Member Text Color",                &DataModelDoc::GetMemberTextColor,              &DataModelDoc::SetMemberTextColor,              true  },
    { "Change Method Text Color",                &DataModelDoc::GetMethodTextColor,              &DataModelDoc::SetMethodTextColor,              false },
    { "Change Relation Line Color",              &DataModelDoc::GetRelationPenColor,             &DataModelDoc::SetRelationPenColor,             true  },
    { "Change Critical Relation Line Color",     &DataModelDoc::GetCriticalRelationPenColor,     &DataModelDoc::SetCriticalRelationPenColor,     false },
    { "Change Relation Text Color",              &DataModelDoc::GetRelationTextColor,            &DataModelDoc::SetRelationTextColor,            false },
    { "Change Inherit Line Color",               &DataModelDoc::GetInheritPenColor,              &DataModelDoc::SetInheritPenColor,              false },
    { "Change Relation Diagram Only Line Color", &DataModelDoc::GetRelationDiagramOnlyPenColor,  &DataModelDoc::SetRelationDiagramOnlyPenColor,  true  },
    { "Change Relation Diagram Only Text Color", &DataModelDoc::GetRelationDiagramOnlyTextColor, &DataModelDoc::SetRelationDiagramOnlyTextColor, false },
    { "Change Dependency Line Color",            &DataModelDoc::GetDependencyPenColor,           &DataModelDoc::SetDependencyPenColor,           true  },
    { "Change Dependency Text Color",            &DataModelDoc::GetDependencyTextColor,          &DataModelDoc::SetDependencyTextColor,          false },
    { "Change NoteShape Line Color",             &DataModelDoc::GetNoteShapePenColor,            &DataModelDoc::SetNoteShapePenColor,            true  },
    { "Change NoteShape Text Color",             &DataModelDoc::GetNoteShapeTextColor,           &DataModelDoc::SetNoteShapeTextColor,           false },
};
const int kTemplateColorCount = (int)(sizeof(kTemplateColors) / sizeof(kTemplateColors[0]));
} // namespace

// ---------------------------------------------------------------------------
// ClassDiagramCanvas
// ---------------------------------------------------------------------------
ClassDiagramCanvas::ClassDiagramCanvas(ClassDiagram* pClassDiagram,
                                       QWidget* parent)
    : QWidget(parent)
    , _pCD(pClassDiagram)
    , _pViewModel(nullptr)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    // Receive mouseMoveEvent on plain hover (no button held) so updateHoverCursor
    // can show the resize cursor over a selection handle (matches the SD canvas).
    setMouseTracking(true);

    // Keyboard focus -- needed for Ctrl+0 / Ctrl++ / Ctrl+- (focus on click).
    setFocusPolicy(Qt::StrongFocus);

    // Feed the app accent into CbPainter as the selection colours -- solid for
    // pens/outlines, a light tint for box interiors -- so the shapes' Draw can
    // recolour selected shapes without knowing the Qt palette.
    const QColor sc = QApplication::palette().color(QPalette::Active, QPalette::Highlight);
    CbPainter::SetSelectColor(Cb_RGB(sc.red(), sc.green(), sc.blue()));
    CbPainter::SetSelectFillColor(Cb_RGB(
        int(sc.red()   * 0.30 + 255 * 0.70),
        int(sc.green() * 0.30 + 255 * 0.70),
        int(sc.blue()  * 0.30 + 255 * 0.70)));

    // Construct the ViewModel representing "this open Qt view" in the model.
    // Lifetime is tied to this canvas either way:
    //  - canvas going down first -> dtor sets _destructing then deletes the
    //    ViewModel; the close callback fires but no-ops on the flag.
    //  - ClassDiagram going down first -> CB cascade deletes the ViewModel; its
    //    dtor invokes CloseCanvas which nulls our pointers and closes the window.
    if (_pCD)
    {
        _pViewModel = new ClassDiagramViewModel(
            _pCD,
            &ClassDiagramCanvas::RefreshCanvas,
            &ClassDiagramCanvas::CloseCanvas,
            this);
    }
}

ClassDiagramCanvas::~ClassDiagramCanvas()
{
    _destructing = true;
    if (_pViewModel)
    {
        delete _pViewModel;
        _pViewModel = nullptr;
    }
}

// Posted as a Qt repaint -- update() schedules, doesn't paint synchronously, so
// it is safe from any model code path.
void ClassDiagramCanvas::RefreshCanvas(void* ctx)
{
    if (auto* self = static_cast<ClassDiagramCanvas*>(ctx))
    {
        self->update();
        // The model changed (this is the once-per-coalesced-op refresh callback):
        // re-evaluate the toolbar action enables. Undo/Redo track the undo stack,
        // which can change with no selection change, so the signature-gated
        // paintEvent emit isn't enough -- fire it here unconditionally.
        emit self->selectionChanged();
        // Broadcast so the OTHER open views of this model refresh their undo/redo
        // enables too -- this canvas's repaint never reaches them. Routed through
        // DataModelDoc so only IT knows the framework doc.
        if (self->_pCD)
            self->_pCD->GetDataModelDoc()->NotifyStateChanged();
    }
}

// Fired from ~ClassDiagramViewModel when the model side tears it down. The
// canvas-going-down-first path sets _destructing first; on that path this no-ops.
void ClassDiagramCanvas::CloseCanvas(void* ctx)
{
    if (auto* self = static_cast<ClassDiagramCanvas*>(ctx))
        self->notifyModelGone();
}

void ClassDiagramCanvas::notifyModelGone()
{
    if (_destructing)
        return;
    _pViewModel = nullptr;
    _pCD        = nullptr;
    // Close the hosting QDockWidget if we live in one (docked OR floated as a
    // dock) -- window() would resolve to the SHELL when docked, which must not
    // be closed. Fall back to the top-level window for the standalone path.
    QWidget* w = this;
    while (w && !qobject_cast<QDockWidget*>(w))
        w = w->parentWidget();
    if (w)
        w->close();                       // the dock: WA_DeleteOnClose deletes us
    else if (QWidget* top = window())
        top->close();
}

// Single source of truth for page size + fit-to-window. The ClassDiagram uses
// the same Y-up convention as the SD (page runs from (0, -pageH) to (pageW, 0));
// fit it into the widget, centered.
ClassDiagramCanvas::FitInfo ClassDiagramCanvas::computeFit() const
{
    FitInfo f;
    const int pageW = _pCD ? _pCD->GetWidth()  : 0;
    const int pageH = _pCD ? _pCD->GetHeight() : 0;
    f.pageW = pageW > 0 ? pageW : 2100;     // A4 default (mm*10)
    f.pageH = pageH > 0 ? pageH : 2970;

    const qreal margin  = 8.0;
    const qreal usableW = qMax<qreal>(1, width()  - 2 * margin);
    const qreal usableH = qMax<qreal>(1, height() - 2 * margin);
    f.fitScale = qMin(usableW / f.pageW, usableH / f.pageH);

    const qreal drawnW = f.pageW * f.fitScale;
    const qreal drawnH = f.pageH * f.fitScale;
    f.originX = (width()  - drawnW) / 2.0;
    f.originY = (height() - drawnH) / 2.0;
    return f;
}

void ClassDiagramCanvas::emitSelectionChangedIfNeeded()
{
    // Signature = selected count folded with the single-selected shape ptr --
    // exactly what the Add-button enables depend on. Cheap; no per-edit hooks.
    quintptr sig = 0;
    if (_pViewModel)
    {
        sig = quintptr(_pViewModel->GetSelectedCount()) * 1000003u;
        sig ^= reinterpret_cast<quintptr>(singleSelectedShape());
    }
    if (sig != _addSelSig)
    {
        _addSelSig = sig;
        emit selectionChanged();
    }
}

void ClassDiagramCanvas::paintEvent(QPaintEvent* /*event*/)
{
    emitSelectionChangedIfNeeded();

    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing,     true);
    qp.setRenderHint(QPainter::TextAntialiasing, true);

    // Grey margin outside the page. Darkened enough to read against the white
    // page on low-contrast / washed-out external screens (was darker(108)).
    qp.fillRect(rect(), palette().color(QPalette::Window).darker(118));

    if (!_pCD)
        return;

    const FitInfo f = computeFit();

    // User zoom + pan, composed BEFORE the fit-to-window transform so they read
    // in widget coords. With the defaults (_zoom=1, _pan=0) these are no-ops and
    // the rendering reduces to plain fit-to-window.
    qp.translate(_pan);
    qp.scale(_zoom, _zoom);

    // Model coords (Y up) -> device coords. The MFC view uses MM_ISOTROPIC with
    // a negative vertical extent; the scale(1, -1) here is the same flip.
    qp.translate(f.originX, f.originY);
    qp.scale(f.fitScale, -f.fitScale);

    // Printable page: white fill; the model page spans (0, -pageH)..(pageW, 0).
    const QRectF pageRect(0, -f.pageH, f.pageW, f.pageH);
    qp.fillRect(pageRect, Qt::white);

    // Multi-page print-preview grid -- pale blue dotted page-break lines.
    // Mirrors CClassDiagramView::OnDraw: _multiPage is a power-of-2 exponent
    // (GetNumberOfPages = 1 << _multiPage); portrait doubles rows first,
    // landscape doubles columns first.
    if (_pCD->GetMultiPage() > 0)
    {
        int nx = 1, ny = 1;
        const int total = _pCD->GetNumberOfPages();
        if (_pCD->GetWidth() < _pCD->GetHeight())   // portrait
        {
            while (nx * ny != total)
            {
                ny *= 2;
                if (nx * ny != total) nx *= 2;
            }
        }
        else                                         // landscape (or square)
        {
            while (nx * ny != total)
            {
                nx *= 2;
                if (nx * ny != total) ny *= 2;
            }
        }

        qp.save();
        qp.setPen(QPen(QColor(170, 190, 255), 0, Qt::DotLine));  // cosmetic
        const qreal ox = f.pageW / qreal(nx);
        const qreal oy = f.pageH / qreal(ny);
        for (int ix = 1; ix < nx; ++ix)
            qp.drawLine(QPointF(ox * ix, 0), QPointF(ox * ix, -f.pageH));
        for (int iy = 1; iy < ny; ++iy)
            qp.drawLine(QPointF(0, -oy * iy), QPointF(f.pageW, -oy * iy));
        qp.restore();
    }

    // The shapes -- same Draw the MFC view calls. Passing the ViewModel makes the
    // shapes draw their own per-view selection highlight (ClassDiagram::Draw is
    // paint-neutral, it never mutates the model).
    CbPainter_QPainter painter(&qp);
    _pCD->Draw(painter, _pViewModel);

    // Selection highlight is drawn by the shapes themselves (in ClassDiagram::Draw,
    // per-view via the ViewModel) -- no overlay pass. The one thing overlaid here is
    // the alignment-reference marker: the LAST-selected alignable shape (Class or
    // Note; IsAlignShape) is the anchor the others align to. A relation can't be
    // aligned, so it's never marked -- lastSelectedAlignShape() skips non-alignables.
    // Drawn ON the box outline at 3px -- in line with a selected connection's thick
    // line -- in a darker shade of the select colour. (MFC's dropped "last-selected"
    // cue, restored but only where it means something.)
    if (ClassDiagramShape* pAnchor = lastSelectedAlignShape())
    {
        CbRect ar = pAnchor->GetRect();   // ON the box outline, not outset
        const qreal ax1 = qMin<qreal>(ar.left, ar.right);
        const qreal ax2 = qMax<qreal>(ar.left, ar.right);
        const qreal ay1 = qMin<qreal>(ar.top,  ar.bottom);
        const qreal ay2 = qMax<qreal>(ar.top,  ar.bottom);
        qp.save();
        // Same colour a selected shape/relation uses (CbPainter's select colour),
        // just a bit darker so the reference reads as distinct from the rest of
        // the selection.
        const CbColorRef selCol = CbPainter::GetSelectColor();
        QColor anchorColor(GetRValue(selCol), GetGValue(selCol), GetBValue(selCol));
        anchorColor = anchorColor.darker(140);
        QPen anchorPen(anchorColor);
        anchorPen.setCosmetic(true);
        anchorPen.setWidth(3);
        qp.setPen(anchorPen);
        qp.setBrush(Qt::NoBrush);
        qp.drawRect(QRectF(ax1, ay1, ax2 - ax1, ay2 - ay1));
        qp.restore();
    }

    // Dashed preview of a class/note move/resize in progress (model untouched
    // until release). Drawn in model coords, on top of the shapes.
    paintShapeDragGhost(qp);
    paintHandleDragGhost(qp);
    paintConnTextDragGhost(qp); // dashed rect preview of a connection-text relocate
    paintMemberReorderGhost(qp);// insertion line for an in-class member/method reorder
    paintPlacementGhost(qp);    // footprint hint while placing a new Class/Note
    paintDropGhost(qp);         // footprint hint while Ctrl-dragging a tree class in
    paintCreateDragGhost(qp);   // dashed source->cursor line + target-class highlight

    // Thin page border (cosmetic 1px so it stays one device pixel at any scale).
    qp.save();
    qp.setPen(QPen(QColor(0x80, 0x80, 0x80), 0));
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(pageRect);
    qp.restore();
}

void ClassDiagramCanvas::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    updateScrollBars();
    update();
}

// ---------------------------------------------------------------------------
// Zoom (Ctrl+wheel) + pan (middle-drag) -- read-only inspection aids, same
// gestures as the SequenceDiagram canvas. Anchored zoom keeps the point under
// the cursor fixed.
// ---------------------------------------------------------------------------
void ClassDiagramCanvas::zoomAt(qreal factor, QPointF anchor)
{
    const qreal newZoom = qBound(0.1, _zoom * factor, 32.0);
    if (qFuzzyCompare(newZoom, _zoom))
        return;
    const qreal eff = newZoom / _zoom;          // clamped-adjusted factor
    _pan  = anchor + eff * (_pan - anchor);
    _zoom = newZoom;
    updateScrollBars();
    update();
}

void ClassDiagramCanvas::resetView()
{
    _zoom = 1.0;
    _pan  = QPointF();
    updateScrollBars();
    update();
}

// Scrollbar sync -- mirrors the SD canvas. The page's left edge maps to widget
// X = _pan.x + _zoom*originX (the paint transform translate(_pan)+scale(_zoom)
// then translate(originX)), so value = -(that), range = scaledPage - viewport.
void ClassDiagramCanvas::bindScrollBars(QScrollBar* h, QScrollBar* v)
{
    _hbar = h;
    _vbar = v;
    if (_hbar)
        connect(_hbar, &QScrollBar::valueChanged, this, &ClassDiagramCanvas::onScrollH);
    if (_vbar)
        connect(_vbar, &QScrollBar::valueChanged, this, &ClassDiagramCanvas::onScrollV);
    updateScrollBars();
}

void ClassDiagramCanvas::updateScrollBars()
{
    if (!_hbar && !_vbar)
        return;
    const FitInfo f = computeFit();
    const qreal scaledW = _zoom * f.fitScale * f.pageW;
    const qreal scaledH = _zoom * f.fitScale * f.pageH;
    const int rangeH = qMax<int>(0, qRound(scaledW - width()));
    const int rangeV = qMax<int>(0, qRound(scaledH - height()));

    _ignoreScrollSignals = true;
    if (_hbar)
    {
        _hbar->setRange(0, rangeH);
        _hbar->setPageStep(width());
        _hbar->setSingleStep(qMax(1, width() / 20));
        _hbar->setValue(qBound(0, qRound(-(_pan.x() + _zoom * f.originX)), rangeH));
        _hbar->setEnabled(rangeH > 0);
    }
    if (_vbar)
    {
        _vbar->setRange(0, rangeV);
        _vbar->setPageStep(height());
        _vbar->setSingleStep(qMax(1, height() / 20));
        _vbar->setValue(qBound(0, qRound(-(_pan.y() + _zoom * f.originY)), rangeV));
        _vbar->setEnabled(rangeV > 0);
    }
    _ignoreScrollSignals = false;
}

void ClassDiagramCanvas::onScrollH(int v)
{
    if (_ignoreScrollSignals)
        return;
    const FitInfo f = computeFit();
    _pan.setX(-v - _zoom * f.originX);
    update();
}

void ClassDiagramCanvas::onScrollV(int v)
{
    if (_ignoreScrollSignals)
        return;
    const FitInfo f = computeFit();
    _pan.setY(-v - _zoom * f.originY);
    update();
}

void ClassDiagramCanvas::applyToolbarZoom(int op)
{
    const QPointF center(width() / 2.0, height() / 2.0);
    if (op > 0)
        zoomAt(1.15, center);
    else if (op < 0)
        zoomAt(1.0 / 1.15, center);
    else
        resetView();
}

bool ClassDiagramCanvas::event(QEvent* e)
{
    // macOS trackpad pinch arrives as a native zoom gesture (not a wheel event).
    // value() is the incremental scale delta per step (e.g. +0.02 / -0.02); map
    // it to a multiplicative zoom factor about the gesture point.
    if (e->type() == QEvent::NativeGesture)
    {
        auto* ng = static_cast<QNativeGestureEvent*>(e);
        if (ng->gestureType() == Qt::ZoomNativeGesture)
        {
            zoomAt(1.0 + ng->value(), ng->position());
            ng->accept();
            return true;
        }
    }
    return QWidget::event(e);
}

void ClassDiagramCanvas::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() & Qt::ControlModifier)
    {
        // Proportional zoom: one wheel notch (120 eighths-of-a-degree) == 1.15x.
        // Scale by the ACTUAL delta -- a Magic Mouse / trackpad sends many small
        // high-resolution deltas per swipe, so a fixed 1.15 per event zoomed
        // wildly ("trigger happy"). pow() makes a swipe sum to a sane amount.
        const int dy = e->angleDelta().y();
        if (dy != 0)
            zoomAt(qPow(1.15, dy / 120.0), e->position());
        e->accept();
        return;
    }
    // Plain scroll/swipe = pan (no modifier). A Magic Mouse / trackpad has no
    // middle button, so this is the only natural way to move a zoomed diagram
    // there; a wheel mouse pans too. pixelDelta is set for touch devices (smooth,
    // both axes); fall back to angleDelta/8 (one notch ~= 15deg) for a plain
    // wheel. Same _pan/repaint path as the middle-drag pan above.
    QPointF delta = !e->pixelDelta().isNull() ? QPointF(e->pixelDelta())
                                              : QPointF(e->angleDelta()) / 8.0;
    if (!delta.isNull())
    {
        _pan += delta;
        updateScrollBars();
        update();
        e->accept();
        return;
    }
    QWidget::wheelEvent(e);
}

void ClassDiagramCanvas::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MiddleButton)
    {
        _panning    = true;
        _lastPanPos = e->position();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }
    // Add Class / Note placement armed: the next left-click drops the shape.
    if (e->button() == Qt::LeftButton && placementActive())
    {
        finishPlacementAt(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && _pCD && _pViewModel)
    {
        const QPointF modelPt = widgetToModel(e->position());
        ClassDiagramShape* pHit = hitTest(modelPt);

        // Drag-to-create armed (R/I/D/O held): start dragging a new connection from
        // the class under the cursor. Consumes the press (no select/box-select).
        if (_createKey != CreateKind::None)
        {
            ClassShape* pSrc = pHit ? pHit->GetClassShape() : nullptr;
            if (pSrc)
            {
                _createDragActive     = true;
                _createSource         = pSrc;
                _createCurrentModelPt = modelPt;
            }
            e->accept();
            return;
        }

        const bool ctrl  = (e->modifiers() & Qt::ControlModifier) != 0;
        const bool shift = (e->modifiers() & Qt::ShiftModifier) != 0;

        // Resize-handle priority on an already-selected class/note: the handle
        // straddles the box edge, so its outer half is OUTSIDE the rect where
        // hitTest would miss it. Grab it -> start a resize, keep the selection.
        ShapeDragPart resizePart;
        ClassDiagramShape* pResize = nullptr;
        if (!ctrl && !shift && selectedShapeResizeAt(modelPt, resizePart, pResize))
        {
            _shapeDragShape          = pResize;
            _shapeDragPart           = resizePart;
            _shapeDragPressRect      = pResize->GetRect();
            _shapeDragPressWidgetPos = e->position();
            _shapeDragStartModelPt   = modelPt;
            _shapeDragCurrentModelPt = modelPt;
            _shapeDragPotential      = true;
            _shapeDragActive         = false;
            e->accept();
            return;
        }

        // Connection middle-segment handle / note connector-point of an already-
        // selected shape -> arm an interior-edit drag (before the body-move arm, so
        // a note's connector point no longer move-drags the whole note).
        if (!ctrl && !shift && selectedHandleAt(modelPt))
        {
            _handleDragPressWidgetPos = e->position();
            _handleDragCurrentModelPt = modelPt;
            _handleDragPotential      = true;
            _handleDragActive         = false;
            e->accept();
            return;
        }

        // Connection text label of an already-selected connection -> arm a text
        // drag. After the segment/note-point handle arm; the text sits off the line
        // so the two never overlap. Mirrors the SD signal-text drag.
        ClassDiagramShape* pTextShape = nullptr;
        ConnTextPart       textPart   = ConnTextPart::None;
        if (!ctrl && !shift && selectedConnTextAt(modelPt, pTextShape, textPart))
        {
            _connTextShape          = pTextShape;
            _connTextPart           = textPart;
            _connTextStartModelPt   = modelPt;
            _connTextCurrentModelPt = modelPt;
            _connTextPressWidgetPos = e->position();
            _connTextPotential      = true;
            _connTextActive         = false;
            e->accept();
            return;
        }

        if (ctrl || shift)
        {
            // Additive / toggle. Empty-space click with a modifier is a no-op
            // (matches MFC: Shift/Ctrl on empty doesn't clear).
            if (pHit)
            {
                if (ClassDiagramViewModelSelection* pSel = findSelection(pHit))
                {
                    if (ctrl)           // Ctrl removes (toggle); Shift keeps
                        delete pSel;
                }
                else
                {
                    (void)new ClassDiagramViewModelSelection(_pViewModel, pHit);
                }
                update();
            }
            e->accept();
            return;
        }

        // Plain click: if the hit shape is already selected, leave the set
        // alone; otherwise clear and select just it. Empty click clears all
        // and arms a box-select.
        const bool hitAlreadySelected = pHit && findSelection(pHit) != nullptr;
        if (!hitAlreadySelected)
        {
            _pViewModel->DeleteAllSelected();
            if (pHit)
                (void)new ClassDiagramViewModelSelection(_pViewModel, pHit);
            update();
        }

        // Member/method of a class -> arm an in-class reorder drag, but only if it
        // was ALREADY selected (a first click just selects/highlights it; the next
        // drag reorders -- so the SizeVer cursor and the action stay consistent).
        // Before the body-move arm so the row isn't treated as a draggable shape.
        if (!ctrl && !shift && pHit && hitAlreadySelected &&
            (pHit->IsMemberShape() || pHit->IsMethodShape()))
        {
            _memReorderShape          = pHit;
            _memReorderClass          = pHit->GetClassShape();
            _memReorderIsMethod       = pHit->IsMethodShape();
            _memReorderPressWidgetPos = e->position();
            _memReorderCurrentModelPt = modelPt;
            _memReorderPotential      = true;
            _memReorderActive         = false;
            e->accept();
            return;
        }

        // Press on a class HEADER (the class shape itself, not a member/method row)
        // or a note body -> arm a move drag (committed on release; a sub-threshold
        // release is just the click/select above). A member/method row falls through
        // to here only when NOT yet selected -> no drag, just the select above.
        if (pHit && (pHit->IsClassShape() || pHit->GetNoteShape()))
        {
            _shapeDragShape          = pHit;
            _shapeDragPart           = ShapeDragPart::Body;
            _shapeDragPressRect      = pHit->GetRect();
            _shapeDragPressWidgetPos = e->position();
            _shapeDragStartModelPt   = modelPt;
            _shapeDragCurrentModelPt = modelPt;
            _shapeDragPotential      = true;
            _shapeDragActive         = false;
            e->accept();
            return;
        }

        if (!pHit)
        {
            _boxSelectPress     = e->position();
            _boxSelectPotential = true;
            _boxSelectActive    = false;
            _boxSelectAdditive  = false;
        }
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void ClassDiagramCanvas::mouseMoveEvent(QMouseEvent* e)
{
    if (_panning)
    {
        const QPointF cur = e->position();
        _pan += (cur - _lastPanPos);
        _lastPanPos = cur;
        updateScrollBars();
        update();
        e->accept();
        return;
    }
    if (placementActive())
    {
        _placementModelPt = widgetToModel(e->position());
        _placementHasPos  = true;
        update();   // footprint ghost follows the cursor
        e->accept();
        return;
    }
    if (_createDragActive)
    {
        _createCurrentModelPt = widgetToModel(e->position());
        update();   // dashed line to the cursor + target-class highlight
        e->accept();
        return;
    }
    if (_shapeDragPotential)
    {
        beginShapeDragIfReady(e->position());
        if (_shapeDragActive)
        {
            _shapeDragCurrentModelPt = widgetToModel(e->position());
            update();
            e->accept();
            return;
        }
    }
    if (_handleDragPotential)
    {
        beginHandleDragIfReady(e->position());
        if (_handleDragActive)
        {
            _handleDragCurrentModelPt = widgetToModel(e->position());
            if (_handleDragKind == HandleDragKind::ConnEndpoint)
                computeEndpointGhost();   // full rerouted preview (model left untouched)
            update();   // ghost repaints; the real shapes stay put until release
            e->accept();
            return;
        }
    }
    if (_connTextPotential)
    {
        beginConnTextDragIfReady(e->position());
        if (_connTextActive)
        {
            _connTextCurrentModelPt = widgetToModel(e->position());
            update();   // dashed-rect ghost; the model is untouched until release
            e->accept();
            return;
        }
    }
    if (_memReorderPotential)
    {
        beginMemberReorderIfReady(e->position());
        if (_memReorderActive)
        {
            _memReorderCurrentModelPt = widgetToModel(e->position());
            update();   // insertion-line ghost; the shapes stay put until release
            e->accept();
            return;
        }
    }
    if (_boxSelectPotential)
    {
        beginBoxSelectIfReady(e->position());
        if (_boxSelectActive && _rubberBand)
            _rubberBand->setGeometry(
                QRect(_boxSelectPress.toPoint(), e->position().toPoint()).normalized());
        e->accept();
        return;
    }
    updateHoverCursor(e->position());
    QWidget::mouseMoveEvent(e);
}

// Hover-cursor feedback over selection handles -- mirrors the SD canvas's
// updateHoverCursor. resolveHandleCursor picks the cursor; nothing under it ->
// default arrow. No body->move cursor yet (drag-move isn't implemented).
void ClassDiagramCanvas::updateHoverCursor(QPointF widgetPt)
{
    if (!_pCD || !_pViewModel || _panning || _boxSelectActive)
        return;
    if (placementActive())
        return;   // keep the cross cursor while Add Class/Note placement is armed
    if (_createKey != CreateKind::None || _createDragActive)
        return;   // keep the cross cursor while drag-to-create is armed/active
    // Connection text label of a selected connection -> SizeAll (free move). Checked
    // before the handle cursor; the text sits off the line so there's no conflict.
    {
        ClassDiagramShape* pTextShape = nullptr;
        ConnTextPart       textPart   = ConnTextPart::None;
        if (selectedConnTextAt(widgetToModel(widgetPt), pTextShape, textPart))
        {
            setCursor(Qt::SizeAllCursor);
            return;
        }
    }
    Qt::CursorShape shape = Qt::ArrowCursor;
    if (resolveHandleCursor(widgetToModel(widgetPt), shape))
    {
        setCursor(shape);
        return;
    }

    // Body-move / reorder cursors -- chosen by what's actually under the cursor so
    // hover matches the drag action: a selected member/method row -> SizeVer
    // (reorder); a class HEADER (the class shape, when that class or one of its
    // members/methods is selected) -> SizeAll (move); everything else -> arrow.
    const QPointF model = widgetToModel(widgetPt);
    if (ClassDiagramShape* pHit = hitTest(model))
    {
        if (pHit->IsMemberShape() || pHit->IsMethodShape())
        {
            if (findSelection(pHit))   // only a SELECTED row reorders
            {
                setCursor(Qt::SizeVerCursor);
                return;
            }
        }
        else if (pHit->IsClassShape())
        {
            if (classOrMemberSelected(pHit->GetClassShape()))
            {
                setCursor(Qt::SizeAllCursor);
                return;
            }
        }
    }
    unsetCursor();
}

// True if the class pc -- or any of its members/methods -- is currently selected.
// (A selected member/method counts: dragging the class header still moves the
// whole class, so the 4-way affordance should show.)
bool ClassDiagramCanvas::classOrMemberSelected(ClassShape* pc) const
{
    if (!pc || !_pViewModel)
        return false;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (p && p->GetClassShape() == pc)   // class itself, or its member/method
            return true;
    }
    return false;
}

// Picks the cursor for a selected shape's handle under modelPt (grab tolerance
// ~6 device px -> model, same idea as the SD hit-tests). Class/note L/R resize
// handle -> SizeHor (via the now-public Get{Left,Right}SelectedPoint, the same
// points the shapes draw). Connection segment handles mirror
// ConnectionSegment::SetCursor's decision -- kept Qt-side per the
// two-separate-methods approach (the MFC SetCursor just gets deleted when MFC
// goes): first/last segment is the endpoint, which slides around the class
// perimeter -> SizeAll; a middle segment moves only perpendicular to its axis ->
// vertical seg (cy != 0) SizeHor, horizontal seg (cx != 0) SizeVer; a degenerate
// zero-length middle segment defers to its neighbours' axis.
bool ClassDiagramCanvas::resolveHandleCursor(QPointF modelPt,
                                             Qt::CursorShape& shapeOut) const
{
    if (!_pViewModel)
        return false;
    const FitInfo f = computeFit();
    const int tol = QtHandle::grabToleranceModel(f.fitScale * _zoom);
    const CbPoint m(qRound(modelPt.x()), qRound(modelPt.y()));
    const auto closeTo = [&](const CbPoint& h)
    {
        return qAbs(m.x - h.x) <= tol && qAbs(m.y - h.y) <= tol;
    };

    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (!p)
            continue;

        if (p->IsClassShape())   // the class itself (not a selected member/method)
        {
            ClassShape* pc = p->GetClassShape();
            if (closeTo(pc->GetLeftSelectedPoint()) ||
                closeTo(pc->GetRightSelectedPoint()))
            {
                shapeOut = Qt::SizeHorCursor;   // L/R edge -> horizontal resize
                return true;
            }
            // The body-move (SizeAll, header) and member/method-reorder (SizeVer)
            // cursors are decided in updateHoverCursor via hitTest, so they line up
            // with the actual drag target (header -> move; selected row -> reorder).
        }
        else if (NoteShape* pn = p->GetNoteShape())
        {
            if (closeTo(pn->GetLeftSelectedPoint()) ||
                closeTo(pn->GetRightSelectedPoint()))
            {
                shapeOut = Qt::SizeHorCursor;   // L/R edge -> horizontal resize
                return true;
            }
            // Note connector points (the "draw a line from here" handles, e.g. the
            // top-right corner) -> free move.
            NoteShape::NoteShapePointIterator iPt(pn);
            while (++iPt)
                if (closeTo(iPt->GetPoint()))
                {
                    shapeOut = Qt::SizeAllCursor;
                    return true;
                }
            if (pn->GetRect().PtInRect(m))
            {
                shapeOut = Qt::SizeAllCursor;   // anywhere on the note body -> move
                return true;
            }
        }
        else if (ConnectionShape* pConn = p->GetConnectionShape())
        {
            QVector<ConnectionSegment*> segs;
            ConnectionShape::ConnectionSegmentIterator iSeg(pConn);
            while (++iSeg)
                segs.append(iSeg.Get());
            for (int i = 0; i < segs.size(); ++i)
            {
                ConnectionSegment* s = segs[i];
                if (!s || !closeTo(s->GetSelectedPoint()))
                    continue;
                if (i == 0 || i == segs.size() - 1)
                    shapeOut = Qt::SizeAllCursor;   // endpoint -> slides round the class
                else if (s->GetSize().cy != 0)
                    shapeOut = Qt::SizeHorCursor;   // vertical segment -> L/R
                else if (s->GetSize().cx != 0)
                    shapeOut = Qt::SizeVerCursor;   // horizontal segment -> U/D
                else
                {
                    const CbSize pv = segs[i - 1]->GetSize();
                    const CbSize nx = segs[i + 1]->GetSize();
                    if (pv.cx != 0 && nx.cx != 0)      shapeOut = Qt::SizeHorCursor;
                    else if (pv.cy != 0 && nx.cy != 0) shapeOut = Qt::SizeVerCursor;
                    else                                continue;   // degenerate
                }
                return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Class/Note move + resize drag. Mirrors the SD note/lifeline drag: the model is
// untouched during the drag, paintShapeDragGhost previews the would-be rect, and
// commitShapeDrag mutates once on release (ClassShape/NoteShape::SetRect both
// self-SaveState and reroute attached connections). One undo step per drag.
// ---------------------------------------------------------------------------

// Hit-test a SELECTED class/note's L/R resize handle (the handle straddles the
// box edge -- its outer half is outside the rect, so hitTest would miss it).
// Same grab tolerance as the hover hit-test (QtHandle).
bool ClassDiagramCanvas::selectedShapeResizeAt(QPointF modelPt,
                                               ShapeDragPart& partOut,
                                               ClassDiagramShape*& shapeOut) const
{
    if (!_pViewModel)
        return false;
    const FitInfo f = computeFit();
    const int tol = QtHandle::grabToleranceModel(f.fitScale * _zoom);
    const CbPoint m(qRound(modelPt.x()), qRound(modelPt.y()));
    const auto closeTo = [&](const CbPoint& h)
    {
        return qAbs(m.x - h.x) <= tol && qAbs(m.y - h.y) <= tol;
    };

    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (!p)
            continue;
        CbPoint lp, rp;
        if (p->IsClassShape())   // resize handles belong to the class, not a member
        {
            ClassShape* pc = p->GetClassShape();
            lp = pc->GetLeftSelectedPoint();
            rp = pc->GetRightSelectedPoint();
        }
        else if (NoteShape* pn = p->GetNoteShape())
        {
            lp = pn->GetLeftSelectedPoint();
            rp = pn->GetRightSelectedPoint();
        }
        else
            continue;
        if (closeTo(lp)) { partOut = ShapeDragPart::ResizeLeft;  shapeOut = p; return true; }
        if (closeTo(rp)) { partOut = ShapeDragPart::ResizeRight; shapeOut = p; return true; }
    }
    return false;
}

// Would-be rect for the current drag (shared by ghost + commit). Move offsets the
// whole rect; resize moves one edge, clamped to a 100-unit min width (mirrors the
// SD note/lifeline resize + MFC's right-100 / left+100).
CbRect ClassDiagramCanvas::shapeDragRect() const
{
    CbRect rect = _shapeDragPressRect;
    if (_shapeDragPart == ShapeDragPart::Body)
    {
        CbSize off(qRound(_shapeDragCurrentModelPt.x() - _shapeDragStartModelPt.x()),
                   qRound(_shapeDragCurrentModelPt.y() - _shapeDragStartModelPt.y()));
        Shape::Round(off);
        rect.OffsetRect(off.cx, off.cy);
    }
    else if (_shapeDragPart == ShapeDragPart::ResizeLeft)
    {
        CbPoint p(qRound(_shapeDragCurrentModelPt.x()), rect.top);
        Shape::Round(p);
        rect.left = qMin(p.x, rect.right - 100);
    }
    else   // ResizeRight
    {
        CbPoint p(qRound(_shapeDragCurrentModelPt.x()), rect.top);
        Shape::Round(p);
        rect.right = qMax(p.x, rect.left + 100);
    }
    return rect;
}

void ClassDiagramCanvas::beginShapeDragIfReady(QPointF widgetPos)
{
    if (!_shapeDragPotential || _shapeDragActive)
        return;
    if ((widgetPos - _shapeDragPressWidgetPos).manhattanLength() < 4)
        return;
    _shapeDragActive = true;
    setCursor(_shapeDragPart == ShapeDragPart::Body ? Qt::SizeAllCursor
                                                    : Qt::SizeHorCursor);
}

void ClassDiagramCanvas::commitShapeDrag()
{
    ClassDiagramShape* pShape = _shapeDragShape;
    const ShapeDragPart part  = _shapeDragPart;
    const CbRect newRect   = shapeDragRect();
    const CbRect pressRect = _shapeDragPressRect;

    _shapeDragActive    = false;
    _shapeDragPotential = false;
    _shapeDragShape     = nullptr;
    unsetCursor();

    if (!_pCD || !pShape || newRect == pressRect)
    {
        update();
        return;
    }
    DataModelDoc* doc = _pCD->GetDataModelDoc();

    if (part == ShapeDragPart::Body)
    {
        // Multi-select move is a MODEL op: ClassDiagram::MoveSelectedShapes slides
        // every shape selected in this view-model by the offset, translating
        // connections whose both ends are selected and rerouting the rest, carrying
        // attached note points. The view only supplies the offset.
        const CbSize offset(newRect.left - pressRect.left, newRect.top - pressRect.top);
        _pCD->MoveSelectedShapes(_pViewModel, offset);
    }
    else if (ClassShape* pc = pShape->GetClassShape())
    {
        // Resize: the dragged class only. Manual resize -> AutoWidth off.
        const CbRect oldRect = pc->GetRect();
        pc->SetAutoWidth(false);
        pc->SaveState(1);
        pc->SetRect(newRect);
        const int edgeOld = (part == ShapeDragPart::ResizeLeft) ? oldRect.left
                                                                : oldRect.right;
        const int edgeNew = (part == ShapeDragPart::ResizeLeft) ? newRect.left
                                                                : newRect.right;
        CbSize offset(edgeNew - edgeOld, 0);
        CbRect nb(edgeOld, oldRect.top, edgeOld, oldRect.bottom);
        nb.NormalizeRect();
        nb.InflateRect(10, 10, 11, 11);
        _pCD->MoveNoteShapePoints(nb, offset);
    }
    else if (NoteShape* pn = pShape->GetNoteShape())
    {
        pn->SetRect(newRect);   // resize: the dragged note only
        CbPainter_QFontMetrics m;
        pn->RecalcHeight(m);    // width may have changed -> re-derive height off-view.
                                // Pure move = same width -> SetRect's guard skips (no-op).
    }

    if (doc)
        doc->MarkLastUndo();
    _pCD->UpdateClassDiagramViews();
    update();
}

void ClassDiagramCanvas::cancelShapeDrag()
{
    if (!_shapeDragPotential && !_shapeDragActive)
        return;
    _shapeDragActive    = false;
    _shapeDragPotential = false;
    _shapeDragShape     = nullptr;
    unsetCursor();
    update();
}

// Dashed cosmetic outline of the would-be rect, painted under the model transform
// (caller leaves it active) on top of the shapes. Mirrors paintNoteDragGhost.
void ClassDiagramCanvas::paintShapeDragGhost(QPainter& qp)
{
    if (!_shapeDragActive || !_shapeDragShape)
        return;
    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);

    const auto dashRect = [&](const CbRect& r)
    {
        const qreal x1 = qMin<qreal>(r.left, r.right);
        const qreal x2 = qMax<qreal>(r.left, r.right);
        const qreal y1 = qMin<qreal>(r.top,  r.bottom);
        const qreal y2 = qMax<qreal>(r.top,  r.bottom);
        qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    };

    if (_shapeDragPart == ShapeDragPart::Body && _pViewModel)
    {
        // Preview every selected class/note at +offset (multi-select move). Use the
        // type test (IsClassShape/IsNoteShape) so member/method sub-shapes aren't
        // ghosted separately -- they follow their class.
        const CbRect nr = shapeDragRect();
        const CbSize offset(nr.left - _shapeDragPressRect.left,
                            nr.top  - _shapeDragPressRect.top);
        ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            ClassDiagramShape* p = iSel->GetClassDiagramShape();
            if (!p || !(p->IsClassShape() || p->IsNoteShape()))
                continue;
            CbRect r = p->GetRect();
            r.OffsetRect(offset.cx, offset.cy);
            dashRect(r);
        }
    }
    else
    {
        dashRect(shapeDragRect());   // resize: the dragged shape only
    }
    qp.restore();
}

// ---------------------------------------------------------------------------
// Connection middle-segment + note connector-point drag (the "interior" edits).
// Same potential/active + ghost + one-undo-step pattern as the box drag; the
// reference for the move is the grabbed handle's ORIGINAL position (model is
// untouched during the drag). Middle segment -> ConnectionSegment::Move (perp,
// self-SaveStates neighbours); note point -> NoteShapePoint::SetPoint (free move,
// self-SaveStates, spawns/deletes corners). Connection endpoint slide is a later
// slice -- selectedHandleAt deliberately skips the first/last segment.
// ---------------------------------------------------------------------------
bool ClassDiagramCanvas::selectedHandleAt(QPointF modelPt)
{
    if (!_pViewModel)
        return false;
    const FitInfo f = computeFit();
    const int tol = QtHandle::grabToleranceModel(f.fitScale * _zoom);
    const CbPoint m(qRound(modelPt.x()), qRound(modelPt.y()));
    const auto closeTo = [&](const CbPoint& h)
    {
        return qAbs(m.x - h.x) <= tol && qAbs(m.y - h.y) <= tol;
    };

    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (!p)
            continue;

        if (NoteShape* pn = p->GetNoteShape())
        {
            NoteShape::NoteShapePointIterator iPt(pn);
            while (++iPt)
                if (closeTo(iPt->GetPoint()))
                {
                    _handleDragKind  = HandleDragKind::NotePoint;
                    _handleDragNote  = pn;
                    _handleDragPoint = iPt.Get();
                    _handleDragConn  = nullptr;
                    _handleDragSeg   = nullptr;
                    return true;
                }
        }
        else if (ConnectionShape* pc = p->GetConnectionShape())
        {
            QVector<ConnectionSegment*> segs;
            ConnectionShape::ConnectionSegmentIterator iSeg(pc);
            while (++iSeg)
                segs.append(iSeg.Get());
            for (int i = 0; i < segs.size(); ++i)
            {
                ConnectionSegment* s = segs[i];
                if (!s || !closeTo(s->GetSelectedPoint()))
                    continue;
                if (i == 0 || i == segs.size() - 1)
                {
                    // Endpoint handle -> slide round the From/To class perimeter.
                    _handleDragKind    = HandleDragKind::ConnEndpoint;
                    _handleDragConn    = pc;
                    _handleDragSeg     = s;
                    _handleDragAtStart = (i == 0);
                    _handleDragNote    = nullptr;
                    _handleDragPoint   = nullptr;
                    _endpointGhostValid = false;   // force a rebuild on the first move
                    return true;
                }
                // Middle segment -> perpendicular move.
                const CbSize sz = s->GetSize();
                if (sz.cx == 0 && sz.cy == 0)
                    continue;   // degenerate zero-length -> no clear move axis
                _handleDragKind = HandleDragKind::ConnSegment;
                _handleDragConn = pc;
                _handleDragSeg  = s;
                _handleDragNote  = nullptr;
                _handleDragPoint = nullptr;
                return true;
            }
        }
    }
    return false;
}

void ClassDiagramCanvas::beginHandleDragIfReady(QPointF widgetPos)
{
    if (!_handleDragPotential || _handleDragActive)
        return;
    if ((widgetPos - _handleDragPressWidgetPos).manhattanLength() < 4)
        return;
    _handleDragActive = true;
    if (_handleDragKind == HandleDragKind::NotePoint
 || _handleDragKind == HandleDragKind::ConnEndpoint)
        setCursor(Qt::SizeAllCursor);   // free move (note point / endpoint slide)
    else
        setCursor(_handleDragSeg && _handleDragSeg->GetSize().cy != 0
                      ? Qt::SizeHorCursor : Qt::SizeVerCursor);
}

bool ClassDiagramCanvas::moveAllModifierActive() const
{
    return (QApplication::keyboardModifiers() & Qt::AltModifier) != 0;
}

// Sibling connections from the same From class whose routing COINCIDES with the
// dragged connection's up to (and including) the dragged segment -> their segment
// at that index is collinear with the dragged one and should move with it under
// "move all" (Alt). Mirrors MFC TrackMiddleX's Ctrl branch, evaluated on the
// untouched model (coincident start points instead of post-move == point+delta).
QVector<ConnectionSegment*> ClassDiagramCanvas::collinearSiblingSegments() const
{
    QVector<ConnectionSegment*> out;
    if (!_handleDragConn || !_handleDragSeg)
        return out;
    ClassShape* from = _handleDragConn->GetFromClassShape();
    if (!from)
        return out;

    QVector<ConnectionSegment*> ref;
    { ConnectionShape::ConnectionSegmentIterator it(_handleDragConn);
      while (++it) ref.append(it.Get()); }
    const int idx = ref.indexOf(_handleDragSeg);
    if (idx < 0)
        return out;

    ClassShape::FromConnectionShapeIterator iSib(from);
    while (++iSib)
    {
        ConnectionShape* sib = iSib.Get();
        if (!sib || sib == _handleDragConn)
            continue;
        QVector<ConnectionSegment*> ss;
        { ConnectionShape::ConnectionSegmentIterator it(sib);
          while (++it) ss.append(it.Get()); }
        if (ss.size() <= idx)
            continue;
        bool coincident = true;
        for (int i = 0; i <= idx; ++i)
            if (ref[i]->GetStartPoint() != ss[i]->GetStartPoint())
            {
                coincident = false;
                break;
            }
        if (coincident)
            out.append(ss[idx]);
    }
    return out;
}

// Grid-snapped perpendicular move of the grabbed segment toward the cursor, CLAMPED
// to the largest legal step (so the ghost stops at the reach limit). Relative to
// the segment's ORIGINAL GetSelectedPoint -- the model is untouched during the drag.
// Under "move all" (Alt) the clamp is the GROUP minimum, so every collinear segment
// can take the same step and they stay together.
CbSize ClassDiagramCanvas::connSegmentClampedDelta() const
{
    CbSize d(0, 0);
    if (!_handleDragSeg)
        return d;
    const CbPoint h  = _handleDragSeg->GetSelectedPoint();
    const CbSize  sz = _handleDragSeg->GetSize();
    if (sz.cy != 0)         // vertical segment -> moves horizontally
        d.cx = qRound(_handleDragCurrentModelPt.x()) - h.x;
    else if (sz.cx != 0)    // horizontal segment -> moves vertically
        d.cy = qRound(_handleDragCurrentModelPt.y()) - h.y;
    Shape::Round(d);

    QVector<ConnectionSegment*> group;
    group.append(_handleDragSeg);
    if (moveAllModifierActive())
        group += collinearSiblingSegments();

    const auto allCanMove = [&](CbSize sz2)
    {
        for (ConnectionSegment* s : group)
            if (!s->CanMove(sz2))   // model predicate (ConnectionSegment::CanMove)
                return false;
        return true;
    };
    if (allCanMove(d))
        return d;
    // Step back toward 0 (units of 10) to the largest move the whole group allows.
    const int stepX = (d.cx > 0) ? -10 : (d.cx < 0) ? 10 : 0;
    const int stepY = (d.cy > 0) ? -10 : (d.cy < 0) ? 10 : 0;
    while (d.cx != 0 || d.cy != 0)
    {
        d.cx += stepX;
        d.cy += stepY;
        if ((stepX < 0 && d.cx < 0) || (stepX > 0 && d.cx > 0)) d.cx = 0;
        if ((stepY < 0 && d.cy < 0) || (stepY > 0 && d.cy > 0)) d.cy = 0;
        if (allCanMove(d))
            return d;
    }
    return CbSize(0, 0);
}

// The 3-line dashed ghost for one segment moved by `delta`: the moved segment plus
// its two neighbours redrawn from their FIXED far ends to the segment's new ends.
void ClassDiagramCanvas::drawConnSegmentGhost(QPainter& qp, ConnectionSegment* seg,
                                              CbSize delta) const
{
    if (!seg)
        return;
    ConnectionShape* conn = seg->GetConnectionShape();
    const CbPoint s = seg->GetStartPoint();
    const CbPoint e = seg->GetEndPoint();
    const QPointF ns(s.x + delta.cx, s.y + delta.cy);
    const QPointF ne(e.x + delta.cx, e.y + delta.cy);
    qp.drawLine(ns, ne);
    if (!conn)
        return;
    // The far end (not shared with the dragged segment) is drawn to the neighbour's
    // *drawn-line* endpoint (ConnectionSegment::GetLineStart/EndPoint) -- past any
    // decoration like the inherit triangle -- so the dashed ghost stops where the
    // solid line does instead of running through the triangle to the class edge.
    if (ConnectionSegment* pPrev = conn->GetPrevConnectionSegment(seg))
    {
        const CbPoint fixed = (pPrev->GetEndPoint() == s) ? pPrev->GetLineStartPoint()
                                                          : pPrev->GetLineEndPoint();
        qp.drawLine(QPointF(fixed.x, fixed.y), ns);
    }
    if (ConnectionSegment* pNext = conn->GetNextConnectionSegment(seg))
    {
        const CbPoint fixed = (pNext->GetStartPoint() == e) ? pNext->GetLineEndPoint()
                                                            : pNext->GetLineStartPoint();
        qp.drawLine(ne, QPointF(fixed.x, fixed.y));
    }
}

namespace {
// Recording CbPainter: captures the DrawLine / Polygon calls the connection segments
// make (route + the triangle / diamond / arrow decorations) so the rerouted
// connection's drawing can be replayed dashed as the endpoint-slide ghost. Every
// other primitive is a no-op -- no text/brush/measurement is needed for the preview.
class GhostRecorder : public CbPainter
{
public:
    QVector<QLineF>    lines;
    QVector<QPolygonF> polys;
    int  Save() override { return 0; }
    void Restore(int) override {}
    void SetPen(int, int, CbColorRef) override {}
    void SetNullBrush() override {}
    void SetSolidBrush(CbColorRef) override {}
    void SetFont(int) override {}
    void SetFontPx(int) override {}
    void SetTextColor(CbColorRef) override {}
    void SetBkColor(CbColorRef) override {}
    CbColorRef GetBkColor() override { return 0; }
    void SetTextAlign(UINT) override {}
    void SetBkMode(int) override {}
    void DrawLine(CbPoint a, CbPoint b) override
        { lines.append(QLineF(a.x, a.y, b.x, b.y)); }
    void Rectangle(CbRect) override {}
    void FillSolidRect(CbRect, CbColorRef) override {}
    void Ellipse(CbRect) override {}
    void Polygon(const CbPoint* p, int n) override
    {
        QPolygonF poly;
        for (int i = 0; i < n; ++i)
            poly << QPointF(p[i].x, p[i].y);
        polys.append(poly);
    }
    void TextOut(int, int, const CbString&) override {}
    void ExtTextOut(int, int, UINT, const CbRect&, const CbString&) override {}
    void CalcText(const CbString&, CbRect&, UINT) override {}
    CbSize GetTextExtent(const CbString&) override { return CbSize(0, 0); }
    bool IsScreen() override { return false; }
};
} // namespace

void ClassDiagramCanvas::computeEndpointGhost()
{
    if (!_pCD || !_handleDragConn)
        return;
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    if (!doc)
        return;
    // Throttle: the endpoint snaps to the class perimeter, so most moves land on the
    // same point -- only rebuild when that snapped point changes. Rebuilding each
    // mouse-move (MarkLastUndo + SlideEndpoint reroute + RollBack) is what makes
    // Windows flash the wait cursor.
    ClassShape* cls = _handleDragAtStart ? _handleDragConn->GetFromClassShape()
                                         : _handleDragConn->GetToClassShape();
    if (!cls)
        return;
    const CbPoint target(qRound(_handleDragCurrentModelPt.x()),
                         qRound(_handleDragCurrentModelPt.y()));
    const CbPoint snapped = Shape::TrackCrossPoint(cls->GetRect(), target);
    if (_endpointGhostValid && snapped == _endpointGhostTarget)
        return;                       // unchanged -> keep the current ghost
    _endpointGhostTarget = snapped;
    _endpointGhostValid  = true;

    _endpointGhostLines.clear();
    _endpointGhostPolys.clear();

    // "Move all" (Alt) applies only to the START endpoint: every From-class sibling
    // sharing the old start point slides too (the inheritance-trunk case). Capture
    // those siblings BEFORE the slide so the ghost previews them moving as well.
    const bool moveSiblings = _handleDragAtStart
 && (QApplication::keyboardModifiers() & Qt::AltModifier) != 0;
    QVector<ConnectionShape*> conns;
    conns.append(_handleDragConn);
    if (moveSiblings)
    {
        ClassShape* from = _handleDragConn->GetFromClassShape();
        const CbPoint oldStart = _handleDragConn->GetStartPoint();
        if (from)
        {
            ClassShape::FromConnectionShapeIterator i(from);
            while (++i)
                if (i.Get() != _handleDragConn && i->GetStartPoint() == oldStart)
                    conns.append(i.Get());
        }
    }

    // Apply the slide on the real connection(s), capture their drawing (route +
    // decorations) via the recorder, then RollBack so the model is untouched. The
    // captured primitives are replayed dashed; the originals stay solid.
    UndoBase* mark = doc->MarkLastUndo();
    if (_handleDragConn->SlideEndpoint(_handleDragAtStart, target, moveSiblings))
    {
        GhostRecorder rec;
        for (ConnectionShape* c : conns)
        {
            ConnectionShape::ConnectionSegmentIterator it(c);
            while (++it)
                it->Draw(rec);
        }
        _endpointGhostLines = rec.lines;
        _endpointGhostPolys = rec.polys;
    }
    doc->RollBack(mark, /*silent*/true);   // no wait-cursor, no MFC view repaint
}

void ClassDiagramCanvas::commitHandleDrag()
{
    const HandleDragKind kind = _handleDragKind;
    NoteShapePoint*    pPoint = _handleDragPoint;
    ConnectionSegment* pSeg   = _handleDragSeg;
    ConnectionShape*   pConn  = _handleDragConn;
    const bool         atStart = _handleDragAtStart;
    const CbSize  segDelta    = (kind == HandleDragKind::ConnSegment) ? connSegmentClampedDelta()
                                                                      : CbSize(0, 0);
    CbPoint newPoint(qRound(_handleDragCurrentModelPt.x()),
                     qRound(_handleDragCurrentModelPt.y()));
    const CbPoint rawCursor = newPoint;            // endpoint slide snaps to the perimeter itself
    Shape::Round(newPoint);                        // note points snap to the grid too

    // "Move all collinear" = Alt held at release; the model method does the work.
    const bool moveAll = (kind == HandleDragKind::ConnSegment)
 && (QApplication::keyboardModifiers() & Qt::AltModifier) != 0;

    _handleDragActive    = false;
    _handleDragPotential = false;
    _handleDragNote      = nullptr;
    _handleDragPoint     = nullptr;
    _handleDragConn      = nullptr;
    _handleDragSeg       = nullptr;
    unsetCursor();

    if (!_pCD)
    {
        update();
        return;
    }
    DataModelDoc* doc = _pCD->GetDataModelDoc();

    if (kind == HandleDragKind::NotePoint && pPoint)
    {
        if (newPoint == pPoint->GetPoint())
        {
            update();
            return;
        }
        pPoint->SetPoint(newPoint);   // self-SaveStates; spawns/deletes corners
        if (doc)
            doc->MarkLastUndo();
    }
    else if (kind == HandleDragKind::ConnSegment && pSeg)
    {
        // The mutation is a MODEL operation: ConnectionSegment::MoveAndAdjust moves
        // this segment (plus the collinear From-class siblings when moveAll) and
        // carries attached note points. The view only supplies the clamped delta.
        if ((segDelta.cx == 0 && segDelta.cy == 0)
 || !pSeg->MoveAndAdjust(segDelta, moveAll))
        {
            update();
            return;
        }
        if (doc)
            doc->MarkLastUndo();
    }
    else if (kind == HandleDragKind::ConnEndpoint && pConn)
    {
        // Model op: ConnectionShape::SlideEndpoint snaps the dragged end to the
        // From/To class perimeter, carries attached note points, and reroutes. A
        // fresh mark first so the per-move ghost's note "just moved" flag doesn't
        // suppress the real note-carry here.
        _endpointGhostLines.clear();
        _endpointGhostPolys.clear();
        // Alt at release -> move From-class siblings sharing the start point too
        // (start endpoint only; SlideEndpoint ignores it for the end).
        const bool moveSiblings = atStart
 && (QApplication::keyboardModifiers() & Qt::AltModifier) != 0;
        if (doc)
            doc->MarkLastUndo();
        if (!pConn->SlideEndpoint(atStart, rawCursor, moveSiblings))
        {
            update();
            return;
        }
        if (doc)
            doc->MarkLastUndo();
    }
    _pCD->UpdateClassDiagramViews();
    update();
}

void ClassDiagramCanvas::cancelHandleDrag()
{
    // Ghost drag -- the model was never touched (the endpoint preview rolls back
    // each move), so cancel is pure state cleanup.
    _endpointGhostLines.clear();
    _endpointGhostPolys.clear();
    _handleDragActive    = false;
    _handleDragPotential = false;
    _handleDragNote      = nullptr;
    _handleDragPoint     = nullptr;
    _handleDragConn      = nullptr;
    _handleDragSeg       = nullptr;
    unsetCursor();
    update();
}

// Dashed accent preview (both kinds are ghosts; the real shapes stay put until
// release). Note point: a line from the note edge to the dragged point + a marker
// square. Middle segment: the segment translated to its clamped new position.
void ClassDiagramCanvas::paintHandleDragGhost(QPainter& qp)
{
    if (!_handleDragActive)
        return;
    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);

    if (_handleDragKind == HandleDragKind::NotePoint && _handleDragNote)
    {
        CbPoint cur(qRound(_handleDragCurrentModelPt.x()),
                    qRound(_handleDragCurrentModelPt.y()));
        Shape::Round(cur);            // match the grid-snapped commit position
        const CbPoint cross = Shape::CrossPoint(_handleDragNote->GetRect(), cur);
        qp.drawLine(QPointF(cross.x, cross.y), QPointF(cur.x, cur.y));
        const FitInfo f = computeFit();
        const qreal scale = f.fitScale * _zoom;
        const qreal hs = scale > 1e-6 ? 5.0 / scale : 5.0;
        qp.drawRect(QRectF(cur.x - hs / 2, cur.y - hs / 2, hs, hs));
    }
    else if (_handleDragKind == HandleDragKind::ConnSegment && _handleDragSeg
 && _handleDragConn)
    {
        // The moved segment AND its two neighbours stretched to follow it (a U-shape
        // dragged past its base needs the side legs redrawn, not just the base).
        // Under "move all" (Alt) every collinear sibling segment previews too.
        const CbSize d = connSegmentClampedDelta();
        drawConnSegmentGhost(qp, _handleDragSeg, d);
        if (moveAllModifierActive())
            for (ConnectionSegment* sib : collinearSiblingSegments())
                drawConnSegmentGhost(qp, sib, d);
    }
    else if (_handleDragKind == HandleDragKind::ConnEndpoint)
    {
        // The rerouted connection's own drawing (route + decorations), captured in
        // computeEndpointGhost and replayed dashed. Original connection stays solid.
        for (const QLineF& l : _endpointGhostLines)
            qp.drawLine(l);
        for (const QPolygonF& poly : _endpointGhostPolys)
            qp.drawPolygon(poly);
    }
    qp.restore();
}

// ---------------------------------------------------------------------------
// Connection text drag -- relocate a relation/dependency/diagram-only label.
// Headless rects measured with CbPainter_QFontMetrics + the relation font, built
// exactly as the model's Draw does (CbRect(point, GetTextExtent(text)) then
// NormalizeRect), so they line up with the painted text. The move commits through
// the model's own Set*Point setter (the same field the MFC Track* methods nudge).
// ---------------------------------------------------------------------------
CbRect ClassDiagramCanvas::connTextRect(ClassDiagramShape* p, ConnTextPart part) const
{
    if (!p || part == ConnTextPart::None)
        return CbRect();
    CbPainter_QFontMetrics m;
    m.SetFont(CBF_RELATION);   // relation/dependency label font
    CbString text;
    CbPoint  pt;
    if (RelationShape* pr = p->GetRelationShape())
    {
        switch (part)
        {
        case ConnTextPart::FromName: text = pr->GetRelation()->GetFromName(); pt = pr->GetFromNamePoint(); break;
        case ConnTextPart::ToName:   text = pr->GetRelation()->GetToName();   pt = pr->GetToNamePoint();   break;
        // UML cardinality is computed (RelationShape::GetUmlFrom/GetUmlTo are
        // private): owned -> "1" else "0..1"; multi -> "*" else "0..1".
        case ConnTextPart::FromUml:  text = pr->GetRelation()->GetOwned() ? CbString("1") : CbString("0..1"); pt = pr->GetFromUmlPoint(); break;
        case ConnTextPart::ToUml:    text = pr->GetRelation()->GetMulti() ? CbString("*") : CbString("0..1"); pt = pr->GetToUmlPoint();   break;
        default: return CbRect();
        }
    }
    else if (RelationDiagramOnlyShape* pd = p->GetRelationDiagramOnlyShape())
    {
        switch (part)
        {
        case ConnTextPart::FromName: text = pd->GetFromName(); pt = pd->GetFromNamePoint(); break;
        case ConnTextPart::ToName:   text = pd->GetToName();   pt = pd->GetToNamePoint();   break;
        case ConnTextPart::FromUml:  text = pd->GetUmlFrom();  pt = pd->GetFromUmlPoint();  break;
        case ConnTextPart::ToUml:    text = pd->GetUmlTo();    pt = pd->GetToUmlPoint();    break;
        default: return CbRect();
        }
    }
    else if (DependencyShape* pdep = p->GetDependencyShape())
    {
        switch (part)
        {
        case ConnTextPart::DepName:       text = pdep->GetName();             pt = pdep->GetNamePoint();       break;
        case ConnTextPart::DepStereotype: text = pdep->GetStereotypeString(); pt = pdep->GetStereotypePoint(); break;
        default: return CbRect();
        }
    }
    else
        return CbRect();

    if (text.IsEmpty())
        return CbRect();
    CbRect r(pt, m.GetTextExtent(text));
    r.NormalizeRect();
    return r;
}

// Which text label (if any) the point hits, tested in the same order/with the
// same visibility gates the Draw uses (relation names only when verbose; the
// other shapes gate on empty text, which connTextRect already returns empty for).
ClassDiagramCanvas::ConnTextPart
ClassDiagramCanvas::connTextPartAt(ClassDiagramShape* p, const CbPoint& modelPt) const
{
    if (!p)
        return ConnTextPart::None;
    const auto hit = [&](ConnTextPart part)
    {
        const CbRect r = connTextRect(p, part);
        return !r.IsRectEmpty() && r.PtInRect(modelPt);
    };
    if (RelationShape* pr = p->GetRelationShape())
    {
        if (pr->GetVerbosity())
        {
            if (hit(ConnTextPart::FromName)) return ConnTextPart::FromName;
            if (hit(ConnTextPart::ToName))   return ConnTextPart::ToName;
        }
        if (hit(ConnTextPart::FromUml)) return ConnTextPart::FromUml;
        if (hit(ConnTextPart::ToUml))   return ConnTextPart::ToUml;
    }
    else if (p->GetRelationDiagramOnlyShape())
    {
        if (hit(ConnTextPart::FromName)) return ConnTextPart::FromName;
        if (hit(ConnTextPart::ToName))   return ConnTextPart::ToName;
        if (hit(ConnTextPart::FromUml))  return ConnTextPart::FromUml;
        if (hit(ConnTextPart::ToUml))    return ConnTextPart::ToUml;
    }
    else if (p->GetDependencyShape())
    {
        if (hit(ConnTextPart::DepStereotype)) return ConnTextPart::DepStereotype;
        if (hit(ConnTextPart::DepName))       return ConnTextPart::DepName;
    }
    return ConnTextPart::None;
}

// Find a SELECTED connection whose text label the point hits (the CD interaction
// model: select the connection first, then its text is grabbable). First match
// wins.
bool ClassDiagramCanvas::selectedConnTextAt(QPointF modelPt,
                                            ClassDiagramShape*& shapeOut,
                                            ConnTextPart& partOut) const
{
    shapeOut = nullptr;
    partOut  = ConnTextPart::None;
    if (!_pViewModel)
        return false;
    const CbPoint m(qRound(modelPt.x()), qRound(modelPt.y()));
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (!p || !p->GetConnectionShape())
            continue;
        const ConnTextPart part = connTextPartAt(p, m);
        if (part != ConnTextPart::None)
        {
            shapeOut = p;
            partOut  = part;
            return true;
        }
    }
    return false;
}

void ClassDiagramCanvas::beginConnTextDragIfReady(QPointF widgetPos)
{
    if (!_connTextPotential || _connTextActive)
        return;
    if ((widgetPos - _connTextPressWidgetPos).manhattanLength() < 4)
        return;
    _connTextActive = true;
    setCursor(Qt::SizeAllCursor);   // free move
}

void ClassDiagramCanvas::commitConnTextDrag()
{
    ClassDiagramShape* p   = _connTextShape;
    const ConnTextPart part = _connTextPart;
    CbSize off(qRound(_connTextCurrentModelPt.x() - _connTextStartModelPt.x()),
               qRound(_connTextCurrentModelPt.y() - _connTextStartModelPt.y()));
    Shape::Round(off);

    _connTextActive    = false;
    _connTextPotential = false;
    _connTextShape     = nullptr;
    _connTextPart      = ConnTextPart::None;
    unsetCursor();

    if (!_pCD || !p || part == ConnTextPart::None || (off.cx == 0 && off.cy == 0))
    {
        update();
        return;
    }

    ConnectionShape* pConn = p->GetConnectionShape();
    if (!pConn)
    {
        update();
        return;
    }

    // Mirror the MFC Track* commit: SaveState, nudge the stored point (rounded),
    // SetInitial(false) so it isn't re-auto-placed, one undo step.
    pConn->SaveState();
    const auto nudge = [&](CbPoint cur) { cur += off; Shape::Round(cur); return cur; };
    if (RelationShape* pr = p->GetRelationShape())
    {
        switch (part)
        {
        case ConnTextPart::FromName: pr->SetFromNamePoint(nudge(pr->GetFromNamePoint())); break;
        case ConnTextPart::ToName:   pr->SetToNamePoint(nudge(pr->GetToNamePoint()));     break;
        case ConnTextPart::FromUml:  pr->SetFromUmlPoint(nudge(pr->GetFromUmlPoint()));   break;
        case ConnTextPart::ToUml:    pr->SetToUmlPoint(nudge(pr->GetToUmlPoint()));       break;
        default: break;
        }
    }
    else if (RelationDiagramOnlyShape* pd = p->GetRelationDiagramOnlyShape())
    {
        switch (part)
        {
        case ConnTextPart::FromName: pd->SetFromNamePoint(nudge(pd->GetFromNamePoint())); break;
        case ConnTextPart::ToName:   pd->SetToNamePoint(nudge(pd->GetToNamePoint()));     break;
        case ConnTextPart::FromUml:  pd->SetFromUmlPoint(nudge(pd->GetFromUmlPoint()));   break;
        case ConnTextPart::ToUml:    pd->SetToUmlPoint(nudge(pd->GetToUmlPoint()));       break;
        default: break;
        }
    }
    else if (DependencyShape* pdep = p->GetDependencyShape())
    {
        switch (part)
        {
        case ConnTextPart::DepName:       pdep->SetNamePoint(nudge(pdep->GetNamePoint()));             break;
        case ConnTextPart::DepStereotype: pdep->SetStereotypePoint(nudge(pdep->GetStereotypePoint())); break;
        default: break;
        }
    }
    pConn->SetInitial(false);

    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    _pCD->UpdateClassDiagramViews();
    update();
}

void ClassDiagramCanvas::cancelConnTextDrag()
{
    if (!_connTextPotential && !_connTextActive)
        return;
    _connTextActive    = false;
    _connTextPotential = false;
    _connTextShape     = nullptr;
    _connTextPart      = ConnTextPart::None;
    unsetCursor();
    update();
}

void ClassDiagramCanvas::paintConnTextDragGhost(QPainter& qp)
{
    if (!_connTextActive || !_connTextShape)
        return;
    const CbRect r = connTextRect(_connTextShape, _connTextPart);
    if (r.IsRectEmpty())
        return;
    CbSize off(qRound(_connTextCurrentModelPt.x() - _connTextStartModelPt.x()),
               qRound(_connTextCurrentModelPt.y() - _connTextStartModelPt.y()));
    Shape::Round(off);

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(QRectF(r.left + off.cx, r.top + off.cy,
                       r.right - r.left, r.bottom - r.top));
    qp.restore();
}

// ---------------------------------------------------------------------------
// Member/method in-class reorder drag. The dragged shape stays painted in place;
// a horizontal insertion line shows the landing slot. On release the order is
// changed via the model's intrusive-list Move primitive (the same one the Reorder
// dialog uses), one undo step. Members and methods are separate sections, so a
// member only reorders among members (and a method among methods).
// ---------------------------------------------------------------------------
bool ClassDiagramCanvas::memberReorderTarget(QVector<ClassDiagramShape*>& others,
                                             int& k) const
{
    others.clear();
    k = 0;
    if (!_memReorderShape || !_memReorderClass)
        return false;
    if (_memReorderIsMethod)
    {
        ClassShape::MethodShapeIterator it(_memReorderClass);
        while (++it)
            if (it.Get() != _memReorderShape)
                others.append(it.Get());
    }
    else
    {
        ClassShape::MemberShapeIterator it(_memReorderClass);
        while (++it)
            if (it.Get() != _memReorderShape)
                others.append(it.Get());
    }
    // k = number of siblings whose vertical centre is above the cursor (higher Y,
    // i.e. displayed above it) -- the slot the dragged item would drop into.
    const qreal cursorY = _memReorderCurrentModelPt.y();
    for (ClassDiagramShape* p : others)
    {
        const CbRect r = p->GetRect();
        if (0.5 * (r.top + r.bottom) > cursorY)
            ++k;
    }
    return true;
}

void ClassDiagramCanvas::beginMemberReorderIfReady(QPointF widgetPos)
{
    if (!_memReorderPotential || _memReorderActive)
        return;
    if ((widgetPos - _memReorderPressWidgetPos).manhattanLength() < 4)
        return;
    _memReorderActive = true;
    setCursor(Qt::SizeVerCursor);   // reorder is vertical-only
}

void ClassDiagramCanvas::commitMemberReorder()
{
    ClassDiagramShape* dragged  = _memReorderShape;
    ClassShape*        pClass   = _memReorderClass;
    const bool         isMethod = _memReorderIsMethod;
    QVector<ClassDiagramShape*> others;
    int k = 0;
    const bool ok = memberReorderTarget(others, k);

    // dragged's own current slot (siblings above its centre) -- dropping there is
    // a no-op (it would land right where it already is).
    int curSlot = 0;
    if (dragged)
    {
        const CbRect dr = dragged->GetRect();
        const qreal dcy = 0.5 * (dr.top + dr.bottom);
        for (ClassDiagramShape* p : others)
        {
            const CbRect r = p->GetRect();
            if (0.5 * (r.top + r.bottom) > dcy)
                ++curSlot;
        }
    }

    _memReorderActive    = false;
    _memReorderPotential = false;
    _memReorderShape     = nullptr;
    _memReorderClass     = nullptr;
    unsetCursor();

    if (!ok || !_pCD || !dragged || !pClass || k == curSlot)
    {
        update();
        return;
    }

    dragged->SaveState(1);
    if (isMethod)
    {
        MethodShape* m = dragged->GetMethodShape();
        if (k >= others.size())
            pClass->MoveMethodShapeLast(m);
        else
            pClass->MoveMethodShapeBefore(m, others[k]->GetMethodShape());
    }
    else
    {
        MemberShape* m = dragged->GetMemberShape();
        if (k >= others.size())
            pClass->MoveMemberShapeLast(m);
        else
            pClass->MoveMemberShapeBefore(m, others[k]->GetMemberShape());
    }
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    _pCD->UpdateClassDiagramViews();
    update();
}

void ClassDiagramCanvas::cancelMemberReorder()
{
    if (!_memReorderPotential && !_memReorderActive)
        return;
    _memReorderActive    = false;
    _memReorderPotential = false;
    _memReorderShape     = nullptr;
    _memReorderClass     = nullptr;
    unsetCursor();
    update();
}

void ClassDiagramCanvas::paintMemberReorderGhost(QPainter& qp)
{
    if (!_memReorderActive || !_memReorderShape || !_memReorderClass)
        return;
    QVector<ClassDiagramShape*> others;
    int k = 0;
    if (!memberReorderTarget(others, k))
        return;
    const CbRect cr = _memReorderClass->GetRect();
    const qreal x1 = qMin<qreal>(cr.left, cr.right);
    const qreal x2 = qMax<qreal>(cr.left, cr.right);
    qreal y;
    if (others.isEmpty())
    {
        const CbRect r = _memReorderShape->GetRect();
        y = 0.5 * (r.top + r.bottom);
    }
    else if (k == 0)
    {
        const CbRect r = others.first()->GetRect();
        y = qMax<qreal>(r.top, r.bottom);   // above the first sibling
    }
    else if (k >= others.size())
    {
        const CbRect r = others.last()->GetRect();
        y = qMin<qreal>(r.top, r.bottom);   // below the last sibling
    }
    else
    {
        const CbRect r = others[k]->GetRect();
        y = qMax<qreal>(r.top, r.bottom);   // just above the sibling we insert before
    }

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(3);
    qp.setPen(pen);
    qp.drawLine(QPointF(x1, y), QPointF(x2, y));
    qp.restore();
}

// ---------------------------------------------------------------------------
// Add Class / Add Note interactive placement (mirrors the SD canvas's placement
// mode -- see SequenceDiagramCanvas::beginPlacement/finishPlacementAt). MFC's
// ClassShape::TrackAdd / NoteShape::TrackAdd run a modal ::GetMessage loop; the
// Qt port arms a mode, then drops the shape on the next click.
// ---------------------------------------------------------------------------
void ClassDiagramCanvas::beginPlacement(PlacementKind kind)
{
    _placementKind   = kind;
    _placementHasPos = false;
    setCursor(Qt::CrossCursor);
    update();
}

// Toolbar Add actions -- dispatch to the exact handlers the context menu's
// "Add" submenu uses (see onContextMenu).
void ClassDiagramCanvas::runAddItem(int item)
{
    switch (item)
    {
    case AddClass:                beginPlacement(PlacementKind::Class); break;
    case AddNote:                 beginPlacement(PlacementKind::Note);  break;
    case AddInheritance:          addBareInheritance(); break;
    case AddRelation:             addBareRelation();    break;
    case AddRelationDiagramOnly:  addConnectionFromSelection(AddConnectionKind::DiagramOnlyRelation); break;
    case AddDependency:           addConnectionFromSelection(AddConnectionKind::Dependency); break;
    case AddMember:               addMember();          break;
    case AddMethod:             addMethod();  break;
    case AddConstructor:          addConstructor();     break;
    case AddArgument:             addArgument();        break;
    case AddVirtualMethods:       addVirtualMethods();  break;
    case AddIsClassMethods:       addIsClassMethods();  break;
    default: break;
    }
}

QString ClassDiagramCanvas::diagramName() const
{
    return _pCD ? toQ(_pCD->GetName()) : QString();
}

bool ClassDiagramCanvas::exportSvg(const QString& path)
{
#ifdef CB_HAVE_SVG
    if (!_pCD)
        return false;
    const FitInfo f = computeFit();

    QSvgGenerator gen;
    gen.setFileName(path);
    gen.setSize(QSize(qCeil(f.pageW), qCeil(f.pageH)));
    gen.setViewBox(QRectF(0, 0, f.pageW, f.pageH));
    gen.setTitle(diagramName());
    gen.setDescription(QStringLiteral("ClassBuilder class diagram"));

    QPainter qp(&gen);
    qp.setRenderHint(QPainter::Antialiasing, true);
    qp.setRenderHint(QPainter::TextAntialiasing, true);
    // Model coords are Y-up; the page spans (0,-pageH)..(pageW,0). The scale
    // (1,-1) flip maps it onto the SVG viewBox (0,0)..(pageW,pageH) -- the same
    // flip paintEvent applies (minus the fit-to-window scale + user zoom/pan).
    qp.scale(1.0, -1.0);
    qp.fillRect(QRectF(0, -f.pageH, f.pageW, f.pageH), Qt::white);

    CbPainter_QPainter painter(&qp);
    painter.SetScreen(false);   // suppress selection highlights, like print/EMF
    _pCD->Draw(painter, _pViewModel);
    return qp.end();
#else
    Q_UNUSED(path);
    return false;
#endif
}

// Enable gate, mirroring the context-menu computation (OnAddXxx(true) on the
// single-selected shape's Gti; the bare connectors also accept a 1-2 class
// selection; Class/Note/diagram-only connectors are always available).
bool ClassDiagramCanvas::addItemEnabled(int item)
{
    if (!_pCD || !_pViewModel)
        return false;
    if (item == AddClass || item == AddNote ||
        item == AddRelationDiagramOnly || item == AddDependency)
        return true;

    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti*           pAddGti = pSingle ? pSingle->GetGti()           : nullptr;
    RelationShape* pAddRel = pSingle ? pSingle->GetRelationShape() : nullptr;
    ClassShape*    pFrom = nullptr;
    ClassShape*    pTo   = nullptr;
    const bool bareConn = selectedFromToClassShapes(pFrom, pTo);

    switch (item)
    {
    case AddInheritance: return (pAddGti && pAddGti->OnAddInherit(true)) || bareConn;
    case AddRelation:    return (pAddGti && pAddGti->OnAddRelation(true)) || bareConn;
    case AddMember:      return pAddGti && pAddGti->OnAddMember(true);
    case AddMethod:    return (pAddGti && pAddGti->OnAddMethod(true)) ||
                                (pAddRel && pAddRel->GetRelation()->GetFromRelation()->OnAddMethod(true));
    case AddConstructor: return pAddGti && pAddGti->OnAddConstructor(true);
    case AddArgument:    return pAddGti && pAddGti->OnAddArgument(true);
    case AddVirtualMethods: return pAddGti && pAddGti->OnAddVirtuals(true);
    case AddIsClassMethods: return pAddGti && pAddGti->OnAddIsClassMethods(true);
    default: return false;
    }
}

void ClassDiagramCanvas::cancelPlacement()
{
    if (_placementKind == PlacementKind::None)
        return;
    _placementKind   = PlacementKind::None;
    _placementHasPos = false;
    unsetCursor();
    update();
}

bool ClassDiagramCanvas::showDropGhost(Gti* pGti, QPoint globalPos)
{
    // Only preview when the class could actually land here (same gate as the drop).
    if (!_pCD || !pGti || !pGti->DropTarget(true, _pCD))
    {
        clearDropGhost();
        return false;
    }
    _dropGhostModelPt = widgetToModel(mapFromGlobal(globalPos));
    _dropGhostActive  = true;
    update();
    return true;
}

void ClassDiagramCanvas::clearDropGhost()
{
    if (_dropGhostActive)
    {
        _dropGhostActive = false;
        update();
    }
}

void ClassDiagramCanvas::paintDropGhost(QPainter& qp)
{
    if (!_dropGhostActive)
        return;
    // 250x40 ClassShape footprint anchored with the cursor at the bottom edge
    // (model Y-up), matching paintPlacementGhost's Class branch + the drop point.
    const QPointF a = _dropGhostModelPt;
    const QRectF foot(a.x(), a.y() - 40, 250, 40);
    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(foot);
    qp.restore();
}

bool ClassDiagramCanvas::dropFromTree(Gti* pGti, QPoint globalPos)
{
    if (!_pCD || !_pViewModel || !pGti)
        return false;
    // Same gate the MFC tree drag uses (ctrl = the add-to-diagram gesture).
    if (!pGti->DropTarget(true, _pCD))
        return false;
    BaseClass* pBaseClass = dynamic_cast<BaseClass*>(pGti);
    if (!pBaseClass)
        return false;

    DataModelDoc* doc = _pCD->GetDataModelDoc();
    const QPointF mm = widgetToModel(mapFromGlobal(globalPos));
    CbPoint point(qRound(mm.x()), qRound(mm.y()));
    Shape::Round(point);

    ClassShape* pCS = new ClassShape(_pCD, pBaseClass, point);
    _pViewModel->DeleteAllSelected();
    (void)new ClassDiagramViewModelSelection(_pViewModel, pCS);
    doc->MarkLastUndo();
    raiseToFront();
    return true;
}

void ClassDiagramCanvas::finishPlacementAt(QPointF widgetPt)
{
    const PlacementKind kind = _placementKind;
    // Leave placement mode BEFORE the dialogs spin their own event loop.
    _placementKind   = PlacementKind::None;
    _placementHasPos = false;
    unsetCursor();

    if (!_pCD || !_pViewModel || kind == PlacementKind::None)
    {
        update();
        return;
    }
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    const QPointF mm = widgetToModel(widgetPt);
    CbPoint point(qRound(mm.x()), qRound(mm.y()));
    Shape::Round(point);

    if (kind == PlacementKind::Note)
    {
        UndoBase* pLastUndo = doc->MarkLastUndo();
        NoteShape* pNote = new NoteShape(_pCD, point);
        // The ctor builds a 100-tall rect; Draw only shrinks it to the no-text
        // height (2*(GetFontHeight()+2) = 60) on the first paint, which shows as a
        // 100->60 jump behind the dialog. Settle it NOW so it matches the ghost and
        // never visibly shrinks (keep bottom = the click anchor + width, raise top).
        {
            CbRect r = pNote->GetRect();
            r.top = r.bottom - 2 * (pNote->GetFontHeight() + 2);
            pNote->SetRect(r);
        }
        if (pNote->OnOpen())   // note text dialog; cancel -> rollback
        {
            _pViewModel->DeleteAllSelected();
            (void)new ClassDiagramViewModelSelection(_pViewModel, pNote);
        }
        else
            doc->RollBack(pLastUndo);
    }
    else // PlacementKind::Class -- add-class dialog, then place a ClassShape
    {
        BaseClass* pPrev = doc->GetLastBaseClass();
        _pCD->GetParent()->OnAddClass();
        BaseClass* pBaseClass = doc->GetLastBaseClass();
        if (pBaseClass && pBaseClass != pPrev)   // a class was actually added
        {
            ClassShape* pCS = new ClassShape(_pCD, pBaseClass, point);
            _pViewModel->DeleteAllSelected();
            (void)new ClassDiagramViewModelSelection(_pViewModel, pCS);
        }
    }
    doc->MarkLastUndo();
    raiseToFront();
    update();
}

void ClassDiagramCanvas::paintPlacementGhost(QPainter& qp)
{
    if (_placementKind == PlacementKind::None || !_placementHasPos)
        return;
    // Footprint hint matching each shape's actual placed rect. Both are anchored
    // with the click at the BOTTOM edge (model Y-up) -> drawn from a.y()-h up to
    // a.y(), i.e. the box appears below the cursor on screen. Class = 250x40
    // (ClassShape default). Note = 400x60: NoteShape::Draw recomputes the height to
    // fit the text with a minimum of (GetFontHeight()+2)*2 = 60 for an empty note,
    // overwriting the ctor's `CbRect(0,100,400,0)` 100 on the first paint.
    const QPointF a = _placementModelPt;
    const qreal w = (_placementKind == PlacementKind::Class) ? 250 : 400;
    const qreal h = (_placementKind == PlacementKind::Class) ? 40  : 60;
    const QRectF foot(a.x(), a.y() - h, w, h);
    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(foot);
    qp.restore();
}

void ClassDiagramCanvas::mouseReleaseEvent(QMouseEvent* e)
{
    if (_panning && e->button() == Qt::MiddleButton)
    {
        _panning = false;
        unsetCursor();
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && _createDragActive)
    {
        ClassDiagramShape* pHit = hitTest(widgetToModel(e->position()));
        ClassShape* pTarget = pHit ? pHit->GetClassShape() : nullptr;
        ClassShape* pSource = _createSource;
        _createDragActive = false;
        _createSource = nullptr;
        // One drag-to-create gesture; disarm the key so a stray key-up isn't needed.
        CreateKind kind = _createKey;
        _createKey = CreateKind::None;
        unsetCursor();
        update();
        if (pSource && pTarget)
            createConnectionByDrag(kind, pSource, pTarget);   // source != target enforced inside
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_shapeDragActive || _shapeDragPotential))
    {
        if (_shapeDragActive)
            commitShapeDrag();      // move / resize the class or note
        else
            cancelShapeDrag();      // no movement -> the press was just a click
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_handleDragActive || _handleDragPotential))
    {
        if (_handleDragActive)
            commitHandleDrag();     // move the segment / note point
        else
            cancelHandleDrag();     // no movement -> just a click
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_connTextActive || _connTextPotential))
    {
        if (_connTextActive)
            commitConnTextDrag();   // relocate the connection text label
        else
            cancelConnTextDrag();   // no movement -> just a click
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_memReorderActive || _memReorderPotential))
    {
        if (_memReorderActive)
            commitMemberReorder();  // change the member/method order
        else
            cancelMemberReorder();  // no movement -> just a click/select
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && _boxSelectPotential)
    {
        if (_boxSelectActive)
            finishBoxSelect(_boxSelectAdditive);
        else
            cancelBoxSelect();
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}

void ClassDiagramCanvas::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        // Clear pending box-select before the modal dialog spins its own event
        // loop (the release may not arrive cleanly afterwards). Reset-to-fit is
        // on Ctrl+0.
        cancelBoxSelect();
        const bool ctrl = (e->modifiers() & Qt::ControlModifier) != 0;
        openShapeAt(e->position(), ctrl);
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

// Ctrl+Shift+<letter> Add* accelerators -- fired from keyPressEvent when the canvas
// has focus (the context-menu items carry the same shortcut for discoverability).
// Gated by the current selection EXACTLY like the menu (the OnAddX(true) checks);
// returns true if it handled the key. Class/Note placement + the diagram-only
// connections are always available (mirrors the menu, which never disables them).
bool ClassDiagramCanvas::triggerAddShortcut(int key)
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pAddGti = pSingle ? pSingle->GetGti() : nullptr;
    ClassShape* pBareFrom = nullptr;
    ClassShape* pBareTo   = nullptr;
    const bool bareConn = selectedFromToClassShapes(pBareFrom, pBareTo);

    switch (key)
    {
    case Qt::Key_C: beginPlacement(PlacementKind::Class); return true;
    case Qt::Key_N: beginPlacement(PlacementKind::Note);  return true;
    case Qt::Key_O: addConnectionFromSelection(AddConnectionKind::DiagramOnlyRelation); return true;
    case Qt::Key_D: addConnectionFromSelection(AddConnectionKind::Dependency);          return true;
    case Qt::Key_I:
        if ((pAddGti && pAddGti->OnAddInherit(true)) || bareConn)  { addBareInheritance(); return true; }
        return false;
    case Qt::Key_R:
        if ((pAddGti && pAddGti->OnAddRelation(true)) || bareConn) { addBareRelation();    return true; }
        return false;
    case Qt::Key_B:
        if (pAddGti && pAddGti->OnAddMember(true))         { addMember();         return true; }
        return false;
    case Qt::Key_M:
        if (pAddGti && pAddGti->OnAddMethod(true))         { addMethod(); return true; }
        return false;
    case Qt::Key_U:
        if (pAddGti && pAddGti->OnAddConstructor(true))    { addConstructor();    return true; }
        return false;
    case Qt::Key_A:
        if (pAddGti && pAddGti->OnAddArgument(true))       { addArgument();       return true; }
        return false;
    case Qt::Key_V:
        if (pAddGti && pAddGti->OnAddVirtuals(true))       { addVirtualMethods(); return true; }
        return false;
    case Qt::Key_S:
        if (pAddGti && pAddGti->OnAddIsClassMethods(true)) { addIsClassMethods(); return true; }
        return false;
    }
    return false;
}

void ClassDiagramCanvas::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (placementActive())
            cancelPlacement();
        else if (_createDragActive || _createKey != CreateKind::None)
        {
            _createDragActive = false;
            _createSource     = nullptr;
            _createKey        = CreateKind::None;
            unsetCursor();
            update();
        }
        else if (_shapeDragActive || _shapeDragPotential)
            cancelShapeDrag();
        else if (_handleDragActive || _handleDragPotential)
            cancelHandleDrag();
        else if (_connTextActive || _connTextPotential)
            cancelConnTextDrag();
        else if (_memReorderActive || _memReorderPotential)
            cancelMemberReorder();
        else if (_boxSelectActive || _boxSelectPotential)
            cancelBoxSelect();
        else
            clearSelection();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Delete)
    {
        deleteSelected();
        e->accept();
        return;
    }
    // Hold R / I / D / O to arm drag-to-create (then drag from class to class).
    if (e->modifiers() == Qt::NoModifier && !e->isAutoRepeat())
    {
        CreateKind k = CreateKind::None;
        switch (e->key())
        {
        case Qt::Key_R: k = CreateKind::Relation;    break;
        case Qt::Key_I: k = CreateKind::Inherit;     break;
        case Qt::Key_D: k = CreateKind::Dependency;  break;
        case Qt::Key_O: k = CreateKind::DiagramOnly; break;
        default: break;
        }
        if (k != CreateKind::None)
        {
            _createKey = k;
            setCursor(Qt::CrossCursor);
            e->accept();
            return;
        }
    }
    if (e->modifiers() & Qt::ControlModifier)
    {
        // Ctrl+Shift+<letter> = the Add* accelerators (same scheme as the tree, shown
        // in the context menu). triggerAddShortcut gates each by the current selection,
        // exactly like the menu. Plain R/I/D/O above remain the drag-create gestures;
        // Ctrl+Shift+Z (redo) is NOT an add letter, so it falls through to the switch.
        if ((e->modifiers() & Qt::ShiftModifier) && triggerAddShortcut(e->key()))
        {
            e->accept();
            return;
        }
        const QPointF center(width() / 2.0, height() / 2.0);
        switch (e->key())
        {
        case Qt::Key_0:     resetView();                 e->accept(); return;
        case Qt::Key_Plus:
        case Qt::Key_Equal: zoomAt(1.15, center);        e->accept(); return;
        case Qt::Key_Minus: zoomAt(1.0 / 1.15, center);  e->accept(); return;
        case Qt::Key_Z:
            if (e->modifiers() & Qt::ShiftModifier) redo(); else undo();
            e->accept(); return;
        case Qt::Key_Y: redo(); e->accept(); return;
        case Qt::Key_A: selectAll(); e->accept(); return;   // select all (reposition)
        }
    }
    // Up/Down move the selection between rows WITHIN the selected class box
    // (header <-> members <-> methods) for keyboard row-select. No wrap at the
    // ends; only active when a class row is selected (else falls through). Left/
    // Right between classes is intentionally NOT done -- spatial nav is fuzzy.
    // NB: macOS flags the arrow keys with Qt::KeypadModifier, so mask it out --
    // a plain `== NoModifier` check never matches the arrows there (Windows has
    // no such flag, so this is a no-op on Windows).
    if ((e->modifiers() & ~Qt::KeypadModifier) == Qt::NoModifier &&
        (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down))
    {
        if (navigateClassRow(e->key() == Qt::Key_Down))
        {
            e->accept();
            return;
        }
    }
    QWidget::keyPressEvent(e);
}

// Up/Down within-class row navigation (keyPressEvent). Moves the single selection
// to the previous/next row of the SAME class box, in visual order
// (header -> members -> methods). Returns true if handled (a class row was
// selected); false otherwise so the arrow falls through. No wrap at the ends.
bool ClassDiagramCanvas::navigateClassRow(bool down)
{
    ClassDiagramShape* pCur = singleSelectedShape();
    if (!pCur)
        return false;
    ClassShape* pClass = pCur->GetClassShape();
    if (!pClass)
        return false;   // a note / connection / nothing -- not a class row

    // Visual top-to-bottom order is exactly: class header, then members, then
    // methods (the iterators already yield each group in display order).
    QList<ClassDiagramShape*> rows;
    rows.append(static_cast<ClassDiagramShape*>(pClass));   // the header row
    { ClassShape::MemberShapeIterator it(pClass); while (++it) rows.append(it.Get()); }
    { ClassShape::MethodShapeIterator it(pClass); while (++it) rows.append(it.Get()); }

    const int idx = rows.indexOf(pCur);
    if (idx < 0)
        return false;
    const int next = idx + (down ? 1 : -1);
    if (next < 0 || next >= rows.size())
        return true;    // at the top/bottom edge: consume, don't wrap

    _pViewModel->DeleteAllSelected();
    (void)new ClassDiagramViewModelSelection(_pViewModel, rows[next]);
    update();
    return true;
}

// Releasing the held R/I/D/O key leaves drag-to-create mode (unless a create-drag
// is in progress -- that finishes on mouse release).
void ClassDiagramCanvas::keyReleaseEvent(QKeyEvent* e)
{
    if (!e->isAutoRepeat() && _createKey != CreateKind::None && !_createDragActive
 && (e->key() == Qt::Key_R || e->key() == Qt::Key_I
 || e->key() == Qt::Key_D || e->key() == Qt::Key_O))
    {
        _createKey = CreateKind::None;
        unsetCursor();
        e->accept();
        return;
    }
    QWidget::keyReleaseEvent(e);
}

// ---------------------------------------------------------------------------
// Selection (M2) -- per-Qt-view, via ClassDiagramViewModelSelection. No
// geometry mutation here (drag/delete are a later, supervised slice).
// ---------------------------------------------------------------------------
// Inverse of the paintEvent transform stack: widget coords -> model (Y-up).
QPointF ClassDiagramCanvas::widgetToModel(QPointF widgetPt) const
{
    const FitInfo f = computeFit();
    const qreal x =  ((widgetPt.x() - _pan.x()) / _zoom - f.originX) / f.fitScale;
    const qreal y = -((widgetPt.y() - _pan.y()) / _zoom - f.originY) / f.fitScale;
    return QPointF(x, y);
}

// CD hit-test is purely rect-based (ClassDiagramShape::PointInShape ->
// Shape::PointInShape -> PtInRect), so a NULL view is safe -- no headless
// measure painter needed (unlike the SD signal-text hit-test).
ClassDiagramShape* ClassDiagramCanvas::hitTest(QPointF modelPt) const
{
    if (!_pCD)
        return nullptr;
    const CbPoint pt(qRound(modelPt.x()), qRound(modelPt.y()));
    return _pCD->GetHitShape(_pViewModel, pt, true);   // Qt ViewModel hit-test path
}

ClassDiagramViewModelSelection*
ClassDiagramCanvas::findSelection(ClassDiagramShape* pShape) const
{
    if (!_pViewModel || !pShape)
        return nullptr;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
        if (iSel->GetClassDiagramShape() == pShape)
            return iSel.Get();
    return nullptr;
}

void ClassDiagramCanvas::clearSelection()
{
    if (!_pViewModel || _pViewModel->GetSelectedCount() == 0)
        return;
    _pViewModel->DeleteAllSelected();
    update();
}

// Ctrl+A -- select every top-level shape (classes, notes, connections), the same
// set box-select accepts; the in-class member/method sub-shapes are not selected.
// Handy for repositioning the whole diagram.
void ClassDiagramCanvas::selectAll()
{
    if (!_pCD || !_pViewModel)
        return;
    _pViewModel->DeleteAllSelected();
    ClassDiagram::ClassDiagramShapeIterator iShape(_pCD);
    while (++iShape)
    {
        ClassDiagramShape* pShape = iShape.Get();
        if (pShape && (pShape->IsClassShape() || pShape->IsNoteShape()
 || pShape->IsConnectionShape()))
            (void)new ClassDiagramViewModelSelection(_pViewModel, pShape);
    }
    update();
}

// Translucent fill + 2-device-pixel cosmetic outline around each selected
// shape's bounding rect, painted after the model (still under the transform).
void ClassDiagramCanvas::paintSelectionOverlay(QPainter& qp, CbPainter& painter)
{
    if (!_pViewModel)
        return;
    const QColor accent = palette().color(QPalette::Active, QPalette::Highlight);
    QColor fill = accent;  fill.setAlpha(56);
    QColor edge = accent;  edge.setAlpha(220);

    qp.save();

    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* pShape = iSel->GetClassDiagramShape();
        if (!pShape)
            continue;

        // Connections now highlight themselves in their own Draw (per-view, via
        // ClassDiagramViewModelSelection); the overlay only handles box shapes.
        if (dynamic_cast<ConnectionShape*>(pShape))
            continue;

        // Box-like shapes (class / member / method / note): translucent fill +
        // cosmetic outline around the bounding rect.
        QPen pen(edge);
        pen.setCosmetic(true);
        pen.setWidth(2);
        qp.setPen(pen);
        qp.setBrush(fill);

        const CbRect r = pShape->GetBoundingRect();
        const qreal x1 = qMin<qreal>(r.left, r.right);
        const qreal x2 = qMax<qreal>(r.left, r.right);
        const qreal y1 = qMin<qreal>(r.top,  r.bottom);
        const qreal y2 = qMax<qreal>(r.top,  r.bottom);
        qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    }
    qp.restore();
}

// Box-select: a shape is selected iff its bounding rect is FULLY enclosed by
// the band. Rendered via a QRubberBand child widget.
void ClassDiagramCanvas::beginBoxSelectIfReady(QPointF widgetPos)
{
    if (!_boxSelectPotential || _boxSelectActive)
        return;
    if ((widgetPos - _boxSelectPress).manhattanLength() < 4)
        return;
    _boxSelectActive = true;
    if (!_rubberBand)
        _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    _rubberBand->setGeometry(
        QRect(_boxSelectPress.toPoint(), widgetPos.toPoint()).normalized());
    _rubberBand->show();
}

void ClassDiagramCanvas::finishBoxSelect(bool additive)
{
    if (!_pCD || !_pViewModel)
    {
        cancelBoxSelect();
        return;
    }
    const QRect band = _rubberBand ? _rubberBand->geometry()
                                   : QRect(_boxSelectPress.toPoint(),
                                           _boxSelectPress.toPoint());
    const QPointF a = widgetToModel(QPointF(band.topLeft()));
    const QPointF b = widgetToModel(QPointF(band.bottomRight()));
    const qreal mx1 = qMin(a.x(), b.x());
    const qreal mx2 = qMax(a.x(), b.x());
    const qreal my1 = qMin(a.y(), b.y());
    const qreal my2 = qMax(a.y(), b.y());

    if (!additive)
        _pViewModel->DeleteAllSelected();

    ClassDiagram::ClassDiagramShapeIterator iShape(_pCD);
    while (++iShape)
    {
        ClassDiagramShape* pShape = iShape.Get();
        if (!pShape)
            continue;
        // Only top-level shapes are box-selectable -- classes, notes, connections.
        // The member/method shapes inside a class are NOT selected (matches MFC; they
        // move with their class). Without this, GetClassShape() on a selected member
        // re-moved its parent class once per member (the "x4" multi-move bug).
        if (!pShape->IsClassShape() && !pShape->IsNoteShape()
 && !pShape->IsConnectionShape())
            continue;
        qreal sx1, sy1, sx2, sy2;
        if (ConnectionShape* pConn = dynamic_cast<ConnectionShape*>(pShape))
        {
            // Use the poly-line extent only, NOT GetBoundingRect -- the latter
            // grows to enclose the connection's text labels, which can sit far
            // from the line and would force an over-large band that grabs other
            // shapes. (Click hit-test already ignores the text region headless.)
            bool any = false;
            ConnectionShape::ConnectionSegmentIterator iSeg(pConn);
            while (++iSeg)
            {
                const CbPoint s = iSeg->GetStartPoint();
                const CbPoint e = iSeg->GetEndPoint();
                const qreal lox = qMin<qreal>(s.x, e.x), hix = qMax<qreal>(s.x, e.x);
                const qreal loy = qMin<qreal>(s.y, e.y), hiy = qMax<qreal>(s.y, e.y);
                if (!any) { sx1 = lox; sx2 = hix; sy1 = loy; sy2 = hiy; any = true; }
                else
                {
                    sx1 = qMin(sx1, lox); sx2 = qMax(sx2, hix);
                    sy1 = qMin(sy1, loy); sy2 = qMax(sy2, hiy);
                }
            }
            if (!any)
                continue;
        }
        else
        {
            const CbRect r = pShape->GetBoundingRect();
            sx1 = qMin<qreal>(r.left, r.right);
            sx2 = qMax<qreal>(r.left, r.right);
            sy1 = qMin<qreal>(r.top,  r.bottom);
            sy2 = qMax<qreal>(r.top,  r.bottom);
        }
        if (sx1 >= mx1 && sx2 <= mx2 && sy1 >= my1 && sy2 <= my2)
            if (!findSelection(pShape))
                (void)new ClassDiagramViewModelSelection(_pViewModel, pShape);
    }
    cancelBoxSelect();
    update();
}

void ClassDiagramCanvas::cancelBoxSelect()
{
    if (_rubberBand)
        _rubberBand->hide();
    _boxSelectPotential = false;
    _boxSelectActive    = false;
}

// ---------------------------------------------------------------------------
// Open (M3) -- double-click routes to the shape's own OnOpen / OnEditAttributes,
// which already wire to the ported Qt dialogs (owned by the MFC main window).
// The dialog mutates the model + UpdateAllViews; our OnDraw mirror hook then
// repaints this canvas. raiseToFront pulls this window back above the dialog.
// ---------------------------------------------------------------------------
void ClassDiagramCanvas::openShapeAt(QPointF widgetPt, bool ctrlHeld)
{
    if (!_pCD || !_pViewModel)
        return;
    ClassDiagramShape* pHit = hitTest(widgetToModel(widgetPt));
    if (!pHit)
        return;
    // Force the double-clicked shape into the sole selection (matches MFC).
    if (!findSelection(pHit))
    {
        _pViewModel->DeleteAllSelected();
        (void)new ClassDiagramViewModelSelection(_pViewModel, pHit);
        update();
    }
    if (ctrlHeld)
        pHit->OnEditAttributes(false);
    else
        pHit->OnOpen(false);
    raiseToFront();
}

void ClassDiagramCanvas::raiseToFront()
{
    if (QWidget* top = window())
    {
        top->raise();
        top->activateWindow();
    }
}

// ---------------------------------------------------------------------------
// Context menu (M4) -- right-click. Ports the universally-applicable core of
// the MFC "ClassDiagram" popup (ClassBuilder.rc) + the SD canvas context menu:
// Open / Edit Attributes / Delete, per-shape Line / Text colour, and the
// document-level Colour Templates submenu. Routes to the shapes' own
// OnOpen / OnEditAttributes / OnDelete and the DataModelDoc colour setters.
// CD-specific items (Add submenu, Align, Select / Reorder / Optimize, view
// toggles, Hide / Show Hidden, Base/Derived, New Sub Window) are a later
// supervised slice.
// ---------------------------------------------------------------------------
ClassDiagramShape* ClassDiagramCanvas::singleSelectedShape() const
{
    if (!_pViewModel)
        return nullptr;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    if (!++iSel)
        return nullptr;
    ClassDiagramShape* p = iSel->GetClassDiagramShape();
    if (++iSel)                                  // more than one -> not single
        return nullptr;
    return p;
}

// Open / Edit the sole selected shape via its own consent-gated handlers (the
// menu only enables these when OnOpen(true) / OnEditAttributes(true) consents).
// The dialog mutates the model + UpdateAllViews; our OnDraw mirror repaints.
void ClassDiagramCanvas::openSelected(bool ctrlHeld)
{
    ClassDiagramShape* p = singleSelectedShape();
    if (!p)
        return;
    if (ctrlHeld)
        p->OnEditAttributes(false);
    else
        p->OnOpen(false);
    raiseToFront();
}

bool ClassDiagramCanvas::hasSelection() const
{
    return _pViewModel && _pViewModel->GetSelectedCount() > 0;
}

// Del / menu Delete -- mirror CClassDiagramView::OnDelete: ask each selected
// shape via OnDelete(checkOnly=true) and, if it consents, delete it. Snapshot
// the shapes first -- OnDelete cascades through the model and disturbs the
// selection junctions the iterator walks. One undo step at the end.
void ClassDiagramCanvas::deleteSelected()
{
    if (!_pViewModel || _pViewModel->GetSelectedCount() == 0)
        return;
    QVector<ClassDiagramShape*> shapes;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
        shapes.append(iSel->GetClassDiagramShape());
    for (ClassDiagramShape* p : shapes)
        if (p && p->OnDelete(true))
            p->OnDelete(false);
    if (_pCD && _pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// Per-shape line colour -- seed the picker from the common colour of the
// selected pen-using shapes (black if they differ), then apply to each with a
// SaveState so it's one undo step.
void ClassDiagramCanvas::changeLineColor()
{
    if (!_pCD || !_pViewModel)
        return;
    CbColorRef seed = Cb_RGB(0, 0, 0);
    bool first = true;
    {
        ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            ClassDiagramShape* p = iSel->GetClassDiagramShape();
            if (p && p->UsesPenColor())
            {
                if (first) { seed = p->GetPenColor(); first = false; }
                else if (p->GetPenColor() != seed) seed = Cb_RGB(0, 0, 0);
            }
        }
    }
    const QColor picked = QColorDialog::getColor(
        QColor(GetRValue(seed), GetGValue(seed), GetBValue(seed)),
        this, tr("Change Line Color"));
    if (!picked.isValid())
        return;     // cancelled
    const CbColorRef chosen = Cb_RGB(picked.red(), picked.green(), picked.blue());

    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (p && p->UsesPenColor())
        {
            p->SaveState(1);
            p->SetPenColor(chosen);
        }
    }
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::changeTextColor()
{
    if (!_pCD || !_pViewModel)
        return;
    CbColorRef seed = Cb_RGB(0, 0, 0);
    bool first = true;
    {
        ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            ClassDiagramShape* p = iSel->GetClassDiagramShape();
            if (p && p->UsesTextColor())
            {
                if (first) { seed = p->GetTextColor(); first = false; }
                else if (p->GetTextColor() != seed) seed = Cb_RGB(0, 0, 0);
            }
        }
    }
    const QColor picked = QColorDialog::getColor(
        QColor(GetRValue(seed), GetGValue(seed), GetBValue(seed)),
        this, tr("Change Text Color"));
    if (!picked.isValid())
        return;     // cancelled
    const CbColorRef chosen = Cb_RGB(picked.red(), picked.green(), picked.blue());

    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (p && p->UsesTextColor())
        {
            p->SaveState(1);
            p->SetTextColor(chosen);
        }
    }
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// Document-level colour template -- pick a colour and store it on the doc via
// the entry's getter/setter pair. Affects every shape that reads that template.
void ClassDiagramCanvas::changeTemplateColor(int entryIndex)
{
    if (!_pCD || entryIndex < 0 || entryIndex >= kTemplateColorCount)
        return;
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    if (!doc)
        return;
    const TemplateColorEntry& en = kTemplateColors[entryIndex];

    const CbColorRef current = (doc->*en.get)();
    const QColor picked = QColorDialog::getColor(
        QColor(GetRValue(current), GetGValue(current), GetBValue(current)),
        this, tr(en.title));
    if (!picked.isValid())
        return;     // cancelled

    (doc->*en.set)(Cb_RGB(picked.red(), picked.green(), picked.blue()));
    doc->MarkLastUndo();
    update();
}

// "Add" submenu -- diagram-only canvas connections. Returns the from/to
// ClassShapes for the current selection: from = first selected, to = last
// selected. 1 or 2 classes only; a single selection yields from == to (a
// self-connection, matching MFC's GetFirstSelected/GetLastSelected). False if
// the selection isn't 1-2 classes (e.g. a member/method/note/connection is
// selected, or nothing is).
bool ClassDiagramCanvas::selectedFromToClassShapes(ClassShape*& pFrom,
                                                   ClassShape*& pTo) const
{
    pFrom = pTo = nullptr;
    if (!_pViewModel)
        return false;
    const int n = _pViewModel->GetSelectedCount();
    if (n != 1 && n != 2)
        return false;
    ClassDiagramShape* pFirst = nullptr;
    ClassDiagramShape* pLast  = nullptr;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (!pFirst) pFirst = p;
        pLast = p;
    }
    if (!pFirst || !pLast)
        return false;
    pFrom = pFirst->GetClassShape();   // null if the shape isn't a class box
    pTo   = pLast->GetClassShape();
    return pFrom && pTo;
}

// Create a diagram-only connection via the dialog's "create" mode: the shape is
// built ONLY when the user clicks OK (inside the dialog), so nothing appears /
// is committed on Cancel -- the root-cause fix for the old "flashes in before
// OK" behaviour, and what lets the gate go away. The selection (1-2 classes)
// just pre-seeds the From/To combos; with any other selection the user picks in
// the dialog (its combos list only on-diagram classes, so any pick is legal).
// MarkLastUndo either side brackets the OK-path creation into one undo step.
void ClassDiagramCanvas::addConnectionFromSelection(AddConnectionKind kind)
{
    if (!_pCD || !_pViewModel)
        return;
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    if (!doc)
        return;

    ClassShape* pInitFrom = nullptr;
    ClassShape* pInitTo   = nullptr;
    selectedFromToClassShapes(pInitFrom, pInitTo);   // best-effort seed; may be null

    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());

    doc->MarkLastUndo();
    ClassDiagramShape* pNew =
        (kind == AddConnectionKind::Dependency)
            ? static_cast<ClassDiagramShape*>(
                  Qt_CreateDependencyDialog(_pCD, pInitFrom, pInitTo, ownerHwnd))
            : static_cast<ClassDiagramShape*>(
                  Qt_CreateRelationDiagramOnlyDialog(_pCD, pInitFrom, pInitTo,
                                                     ownerHwnd));

    if (pNew)   // OK: a shape was created -- make it the sole selection
    {
        _pViewModel->DeleteAllSelected();
        (void)new ClassDiagramViewModelSelection(_pViewModel, pNew);
    }
    doc->MarkLastUndo();
    raiseToFront();
    update();
}

// ---------------------------------------------------------------------------
// Drag-to-create (hold R / I / D / O, drag one class onto another). Mirrors the
// 2-selected branch of CClassDiagramView::OnEditAddrelation / OnEditAddinherit
// for R/I, and reuses the diagram-only/dependency create dialogs for D/O. The
// drag's SOURCE class is the "from" (relation owner / inherit base), the TARGET
// class the "to" (owned / derived) -- same first/last mapping the menu uses.
// Created on OK; RollBack on Cancel. Nothing is committed until then.
// ---------------------------------------------------------------------------
void ClassDiagramCanvas::createConnectionByDrag(CreateKind kind,
                                                ClassShape* from, ClassShape* to)
{
    if (kind == CreateKind::None || !_pCD || !_pViewModel || !from || !to || from == to)
        return;
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    if (!doc)
        return;

    // The R/I/D/O key is still physically held while the create dialog is modal.
    // Swallow its auto-repeat (app-wide, dialog lifetime only) so it can't type
    // into the dialog's name field. Single exit below removes the filter.
    _swallowCreateAutoRepeat = true;
    qApp->installEventFilter(this);

    const auto selectNew = [&](ClassDiagramShape* pShape)
    {
        if (!pShape)
            return;
        _pViewModel->DeleteAllSelected();
        (void)new ClassDiagramViewModelSelection(_pViewModel, pShape);
    };

    if (kind == CreateKind::Dependency || kind == CreateKind::DiagramOnly)
    {
        // Same create-on-OK dialogs the Add submenu uses; from/to pre-seed combos.
        void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
        doc->MarkLastUndo();
        ClassDiagramShape* pNew =
            (kind == CreateKind::Dependency)
                ? static_cast<ClassDiagramShape*>(
                      Qt_CreateDependencyDialog(_pCD, from, to, ownerHwnd))
                : static_cast<ClassDiagramShape*>(
                      Qt_CreateRelationDiagramOnlyDialog(_pCD, from, to, ownerHwnd));
        selectNew(pNew);
        doc->MarkLastUndo();
    }
    else if (kind == CreateKind::Relation)
    {
        Class* pFrom = dynamic_cast<Class*>(from->GetGti());
        Class* pTo   = dynamic_cast<Class*>(to->GetGti());
        if (pFrom && pTo)
        {
            doc->MarkLastUndo();
            Relation* pRelation = new Relation(pFrom, pTo, "", "", 0, 1, 0, 1, 0);
            if (pRelation->GetFromRelation()->OnEditAttributes())
            {
                selectNew(pRelation->FindRelationShape(_pCD));
                pRelation->GetToClass()->GetConstructorIncludeMethod()->UpdateArguments();
                pRelation->Add();
            }
            else
                doc->RollBack();
            doc->MarkLastUndo();
        }
    }
    else // CreateKind::Inherit
    {
        BaseClass*   pBaseClass   = dynamic_cast<BaseClass*>(from->GetGti());   // base
        ExternClass* pExternClass = dynamic_cast<ExternClass*>(to->GetGti());   // derived
        if (pBaseClass && pExternClass)
        {
            Class* pClass = dynamic_cast<Class*>(pExternClass);
            if (pClass && pClass->GetSerialize())
            {
                // Serialize-on classes carry a single fixed inherit -- edit it
                // rather than add a second base (mirrors the menu rule).
                pClass->GetFirstInherit()->OnEditAttributes();
            }
            else
            {
                doc->MarkLastUndo();
                Inherit* pInherit = new Inherit(pExternClass, pBaseClass);
                if (pInherit->OnEditAttributes())
                {
                    selectNew(pInherit->FindInheritShape(_pCD));
                    pInherit->Add();
                }
                else
                    doc->RollBack();
                doc->MarkLastUndo();
            }
        }
    }

    qApp->removeEventFilter(this);
    _swallowCreateAutoRepeat = false;

    raiseToFront();
    update();
}

// Eats the held R/I/D/O key's auto-repeat while a create dialog is modal (see
// _swallowCreateAutoRepeat). The genuine press/release (isAutoRepeat()==false)
// passes through untouched.
bool ClassDiagramCanvas::eventFilter(QObject* obj, QEvent* ev)
{
    if (_swallowCreateAutoRepeat && ev->type() == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(ev);
        if (ke->isAutoRepeat())
        {
            switch (ke->key())
            {
            case Qt::Key_R: case Qt::Key_I:
            case Qt::Key_D: case Qt::Key_O:
                return true;   // swallow -- don't let it type into the dialog
            default: break;
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}

// Dashed source-class-centre -> cursor preview, plus a highlight outline around
// the class under the cursor (the would-be target). Drawn in model coords on
// top of the shapes -- the painter already carries the model transform here.
void ClassDiagramCanvas::paintCreateDragGhost(QPainter& qp)
{
    if (!_createDragActive || !_createSource)
        return;
    const CbRect src = _createSource->GetRect();
    const QPointF a(0.5 * (src.left + src.right), 0.5 * (src.top + src.bottom));
    const QPointF b = _createCurrentModelPt;

    qp.save();
    const QColor hi = palette().color(QPalette::Active, QPalette::Highlight);

    // Highlight the class box under the cursor (the prospective target).
    ClassDiagramShape* pHit = hitTest(b);
    ClassShape* pTarget = pHit ? pHit->GetClassShape() : nullptr;
    if (pTarget && pTarget != _createSource)
    {
        const CbRect r = pTarget->GetRect();
        QPen tpen(hi);
        tpen.setCosmetic(true);
        tpen.setWidth(3);
        qp.setPen(tpen);
        qp.setBrush(Qt::NoBrush);
        const qreal x1 = qMin<qreal>(r.left, r.right);
        const qreal x2 = qMax<qreal>(r.left, r.right);
        const qreal y1 = qMin<qreal>(r.top,  r.bottom);
        const qreal y2 = qMax<qreal>(r.top,  r.bottom);
        qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    }

    QPen pen(hi);
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawLine(a, b);
    qp.restore();
}

// --- View toggles (mirror CClassDiagramView::OnClassdiagram* + OnUpdate*) -----
// Each: SaveState, flip the shape flag, MarkLastUndo, refresh. Single-shape ones
// no-op unless the sole selection is the right shape kind.

void ClassDiagramCanvas::toggleAutoWidth()
{
    ClassDiagramShape* p = singleSelectedShape();
    ClassShape* pClassShape = p ? p->GetClassShape() : nullptr;
    if (!pClassShape || !_pCD)
        return;
    pClassShape->SaveState(1);
    pClassShape->SetAutoWidth(pClassShape->GetAutoWidth() ? false : true);
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::toggleShowMethodArguments()
{
    ClassDiagramShape* p = singleSelectedShape();
    ClassShape* pClassShape = p ? p->GetClassShape() : nullptr;
    if (!pClassShape || !_pCD)
        return;
    pClassShape->SaveState(1);
    pClassShape->SetVerbosity(pClassShape->GetVerbosity() ? 0 : 1);
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::toggleShowRelationNames()
{
    ClassDiagramShape* p = singleSelectedShape();
    RelationShape* pRelationShape = p ? p->GetRelationShape() : nullptr;
    if (!pRelationShape || !_pCD)
        return;
    pRelationShape->SaveState();
    pRelationShape->SetVerbosity(pRelationShape->GetVerbosity() ? 0 : 1);
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::hideSelectedConnections()
{
    if (!_pCD || !_pViewModel)
        return;
    // Snapshot first -- Hide() moves the connection into the diagram's Hidden
    // list, disturbing the selection iterator.
    QVector<ConnectionShape*> conns;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (p && p->GetConnectionShape())
            conns.append(p->GetConnectionShape());
    }
    if (conns.isEmpty())
        return;
    for (ConnectionShape* pConn : conns)
        pConn->Hide();
    // A hidden shape must not stay selected: you can't see it, and a stale
    // selection of an invisible shape is bound to bite later (a colour change /
    // delete silently hitting it). Mirrors delete, where removed items drop out
    // of the selection. (The Show-Hidden enable no longer depends on this -- it's
    // purely hygiene now.)
    _pViewModel->DeleteAllSelected();
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::showHiddenConnections()
{
    if (!_pCD || !_pViewModel)
        return;
    // Snapshot first -- Show() removes from the Hidden list, invalidating the
    // iterator. Collect ALL hidden, plus those "focused" by a selected class
    // endpoint. Reveal the focused set if any (select a class -> reveal just its
    // hidden connections); otherwise reveal everything -- so the command always
    // does something when enabled, regardless of what (if anything) is selected.
    QVector<ConnectionShape*> hidden;
    QVector<ConnectionShape*> focused;
    ClassDiagram::HiddenIterator iHidden(_pCD);
    while (++iHidden)
    {
        ConnectionShape* pConn = iHidden.Get();
        if (!pConn)
            continue;
        hidden.append(pConn);
        if (findSelection(pConn->GetFromClassShape()) ||
            findSelection(pConn->GetToClassShape()))
            focused.append(pConn);
    }
    const QVector<ConnectionShape*>& toShow = focused.isEmpty() ? hidden : focused;
    if (toShow.isEmpty())
        return;
    for (ConnectionShape* pConn : toShow)
        pConn->Show();
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// --- Select Classes / Members&Methods (mirror CClassDiagramView handlers) -----

void ClassDiagramCanvas::selectClasses()
{
    if (!_pCD)
        return;
    // Count existing class shapes -- if the diagram was empty, the newly-added
    // classes need auto-placement (Grid), exactly as the MFC handler does.
    int classCnt = 0;
    ClassDiagram::ClassDiagramShapeIterator
        iClassShape(_pCD, &ClassDiagramShape::IsClassShape);
    while (++iClassShape)
        classCnt++;

    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
    if (Qt_ShowSelectClassesDialog(_pCD, ownerHwnd))
    {
        if (!classCnt)            // first classes dropped onto an empty diagram
        {
            Grid grid(_pCD);      // RecalculateRectWidth uses a desktop DC headless
            grid.Place();
        }
        if (_pCD->GetDataModelDoc())
            _pCD->GetDataModelDoc()->MarkLastUndo();
        update();
    }
}

// Refresh callback for the diagram-wide Select Members&Methods dialog's in-place
// Apply (mirrors the MFC Cb_RefreshAllViews + CDocument* context). Context is the
// ClassDiagram*; refresh its Qt views so an Apply repaints while the dialog stays.
static void cdSelectMembersApply(void* context)
{
    if (ClassDiagram* pCD = static_cast<ClassDiagram*>(context))
        pCD->UpdateClassDiagramViews();
}

void ClassDiagramCanvas::selectMembersAndMethods()
{
    if (!_pCD || !_pViewModel)
        return;
    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
    ClassDiagramShape* pSingle = singleSelectedShape();
    ClassShape* pClassShape = pSingle ? pSingle->GetClassShape() : nullptr;
    if (pClassShape)            // one class selected -> its members/methods dialog
    {
        if (Qt_ShowClassShapeDialog(pClassShape, ownerHwnd))
        {
            update();
        }
    }
    else                        // diagram-wide picker (Apply commits, Cancel rolls back)
    {
        Qt_ShowSelectMembersAndMethodsDialog(_pCD, ownerHwnd,
                                             &cdSelectMembersApply, _pCD);
        update();
    }
}

// --- Layout / navigation (mirror the remaining MFC CD handlers) --------------

// CClassDiagramView::OnClassdiagramOptimizeplacement -- Grid auto-layout of the
// whole diagram. The MFC handler doesn't SaveState explicitly; Grid::Place's own
// moves snapshot, and MarkLastUndo closes the batch.
void ClassDiagramCanvas::optimizePlacement()
{
    if (!_pCD)
        return;
    Grid grid(_pCD);          // RecalculateRectWidth uses a desktop DC headless
    grid.Place();
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// The last selected alignable shape -- the alignment anchor (MFC GetLastSelected,
// which is the most-recently-added selection = last in the Selected list).
ClassDiagramShape* ClassDiagramCanvas::lastSelectedAlignShape() const
{
    if (!_pViewModel)
        return nullptr;
    ClassDiagramShape* pLast = nullptr;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (p && p->IsAlignShape())
            pLast = p;
    }
    return pLast;
}

int ClassDiagramCanvas::alignableSelectedCount() const
{
    if (!_pViewModel)
        return 0;
    int n = 0;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (p && p->IsAlignShape())
            ++n;
    }
    return n;
}

// Align needs an anchor + at least one other shape to move (mirrors the context-
// menu Align submenu's enable gate; also gates the toolbar Align dropdown).
bool ClassDiagramCanvas::canAlign() const
{
    return alignableSelectedCount() >= 2;
}

// CClassDiagramView::OnAlign{Left..Bottom} -- align every selected alignable
// shape to the anchor (last selected). One undo step (the Align* setters snapshot;
// MarkLastUndo closes the batch).
void ClassDiagramCanvas::alignSelected(AlignKind which)
{
    if (!_pCD || !_pViewModel)
        return;
    ClassDiagramShape* pAnchor = lastSelectedAlignShape();
    if (!pAnchor)
        return;
    ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        ClassDiagramShape* p = iSel->GetClassDiagramShape();
        if (!p || !p->IsAlignShape())
            continue;
        switch (which)
        {
        case AlignKind::Left:   p->AlignLeft(pAnchor);   break;
        case AlignKind::Center: p->AlignCenter(pAnchor); break;
        case AlignKind::Right:  p->AlignRight(pAnchor);  break;
        case AlignKind::Top:    p->AlignTop(pAnchor);    break;
        case AlignKind::Middle: p->AlignMiddle(pAnchor); break;
        case AlignKind::Bottom: p->AlignBottom(pAnchor); break;
        }
    }
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// CClassDiagramView::OnClassdiagramReordermembersmethods -- single class shape's
// member/method ordering dialog (Qt bridge owns its own undo).
void ClassDiagramCanvas::reorderMembersMethods()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    ClassShape* pClassShape = pSingle ? pSingle->GetClassShape() : nullptr;
    if (!pClassShape || !_pCD)
        return;
    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
    if (Qt_ShowClassShapeOrderDialog(pClassShape, ownerHwnd))
    {
        update();
    }
}

// CClassDiagramView::OnViewInheritsfrom -- read-only navigation dialog of the
// single selected class's base classes.
void ClassDiagramCanvas::inheritsFrom()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    if (ExternClass* pExternClass = dynamic_cast<ExternClass*>(pGti))
    {
        void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
        Qt_ShowInheritsFromDialog(pExternClass, ownerHwnd);
    }
}

// CClassDiagramView::OnViewInheritedby -- read-only navigation dialog of the
// classes derived from the single selected base class.
void ClassDiagramCanvas::inheritedBy()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    if (BaseClass* pBaseClass = dynamic_cast<BaseClass*>(pGti))
    {
        void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
        Qt_ShowInheritedByDialog(pBaseClass, ownerHwnd);
    }
}

// --- Add submenu: model-level adds (mirror CClassDiagramView::OnEditAdd*) ------
// Each captures the class's last member/method, calls the Gti's OnAddXxx (which
// opens the Qt edit dialog and creates the model object, or rolls back on cancel),
// then creates a MemberShape/MethodShape for each newly-appended model object in
// THIS diagram's class box. Argument adds no shape. The OnAddXxx(true) check is the
// enable gate (computed in contextMenuEvent).

// Create MemberShapes for every Member appended to pBaseClass after pAfter.
static void cdAddNewMemberShapes(ClassShape* pClassShape, BaseClass* pBaseClass,
                                 Member* pAfter)
{
    BaseClass::MemberIterator iMember(pBaseClass, 0, pAfter);
    while (++iMember)
        (void)new MemberShape(pClassShape, iMember.Get());
}

// Create MethodShapes for every Method appended to pBaseClass after pAfter.
static void cdAddNewMethodShapes(ClassShape* pClassShape, BaseClass* pBaseClass,
                                 Method* pAfter)
{
    BaseClass::MethodIterator iMethod(pBaseClass, 0, pAfter);
    while (++iMethod)
        (void)new MethodShape(pClassShape, iMethod.Get());
}

void ClassDiagramCanvas::addMember()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    ClassShape* pClassShape = pSingle ? pSingle->GetClassShape() : nullptr;
    if (!pGti || !pClassShape || !_pCD)
        return;
    BaseClass* pBaseClass = pClassShape->GetBaseClass();
    Member* pLastMember = pBaseClass->GetLastMember();
    pGti->OnAddMember();   // opens the Qt member dialog; rolls back on cancel
    cdAddNewMemberShapes(pClassShape, pBaseClass, pLastMember);
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::addMethod()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    if (!pSingle || !_pCD)
        return;
    Gti* pGti = pSingle->GetGti();
    if (pGti)   // class (or member/method) selected -> add to its class
    {
        ClassShape* pClassShape = pSingle->GetClassShape();
        if (!pClassShape)
            return;
        BaseClass* pBaseClass = pClassShape->GetBaseClass();
        Method* pLastMethod = pBaseClass->GetLastMethod();
        pGti->OnAddMethod();
        cdAddNewMethodShapes(pClassShape, pBaseClass, pLastMethod);
    }
    else if (RelationShape* pRel = pSingle->GetRelationShape())
    {
        // A relation selected -> add a method to its FROM class (mirrors MFC).
        ClassShape* pClassShape = pRel->GetFromClassShape();
        if (!pClassShape)
            return;
        BaseClass* pBaseClass = pClassShape->GetBaseClass();
        Method* pLastMethod = pBaseClass->GetLastMethod();
        pRel->GetRelation()->GetFromRelation()->OnAddMethod();
        cdAddNewMethodShapes(pClassShape, pBaseClass, pLastMethod);
    }
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::addConstructor()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    ClassShape* pClassShape = pSingle ? pSingle->GetClassShape() : nullptr;
    if (!pGti || !pClassShape || !_pCD)
        return;
    BaseClass* pBaseClass = pClassShape->GetBaseClass();
    Method* pLastMethod = pBaseClass->GetLastMethod();
    pGti->OnAddConstructor();
    cdAddNewMethodShapes(pClassShape, pBaseClass, pLastMethod);
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::addIsClassMethods()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    ClassShape* pClassShape = pSingle ? pSingle->GetClassShape() : nullptr;
    if (!pGti || !pClassShape || !_pCD)
        return;
    BaseClass* pBaseClass = pClassShape->GetBaseClass();
    Method* pLastMethod = pBaseClass->GetLastMethod();
    pGti->OnAddIsClassMethods();
    cdAddNewMethodShapes(pClassShape, pBaseClass, pLastMethod);
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::addVirtualMethods()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    if (!pGti || !_pCD)
        return;
    // Virtuals may land in several classes; mirror MFC -- diff the doc object list
    // and add a MethodShape for each new Method whose class is shown in THIS diagram.
    DataModelDoc* doc = pGti->GetDataModelDoc();
    DataModelDocObject* pLastObj = doc->GetLastDataModelDocObject();
    pGti->OnAddVirtuals();
    DataModelDoc::DataModelDocObjectIterator iObj(doc, 0, pLastObj);
    while (++iObj)
    {
        Method* pMethod = dynamic_cast<Method*>(iObj.Get());
        if (!pMethod)
            continue;
        BaseClass::ClassShapeIterator iClassShape(pMethod->GetBaseClass());
        while (++iClassShape)
            if (iClassShape->GetClassDiagram() == _pCD)
                (void)new MethodShape(iClassShape.Get(), pMethod);
    }
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::addArgument()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    if (!pGti || !_pCD)
        return;
    pGti->OnAddArgument();   // arguments aren't diagram shapes -- no shape to add
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// --- Add submenu: bare model Relation / Inheritance (mirror OnEditAddrelation /
// OnEditAddinherit; 1 selected class -> self, 2 -> from/to). These create from a
// SELECTED class, unlike the diagram-only pair -- hence gated on a class selection.
void ClassDiagramCanvas::addBareRelation()
{
    if (!_pCD || !_pViewModel)
        return;
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    if (!doc)
        return;
    ClassShape* pFromShape = nullptr;
    ClassShape* pToShape   = nullptr;
    if (!selectedFromToClassShapes(pFromShape, pToShape))
        return;
    Class* pFrom = dynamic_cast<Class*>(pFromShape->GetGti());
    Class* pTo   = dynamic_cast<Class*>(pToShape->GetGti());
    if (!pFrom || !pTo)
        return;
    doc->MarkLastUndo();
    Relation* pRelation = new Relation(pFrom, pTo, "", "", 0, 1, 0, 1, 0);
    if (pRelation->GetFromRelation()->OnEditAttributes())
    {
        if (RelationShape* pShape = pRelation->FindRelationShape(_pCD))
        {
            _pViewModel->DeleteAllSelected();
            (void)new ClassDiagramViewModelSelection(_pViewModel, pShape);
        }
        pRelation->GetToClass()->GetConstructorIncludeMethod()->UpdateArguments();
        pRelation->Add();
    }
    else
        doc->RollBack();
    doc->MarkLastUndo();
    raiseToFront();
    update();
}

void ClassDiagramCanvas::addBareInheritance()
{
    if (!_pCD || !_pViewModel)
        return;
    DataModelDoc* doc = _pCD->GetDataModelDoc();
    if (!doc)
        return;
    ClassShape* pFromShape = nullptr;   // base
    ClassShape* pToShape   = nullptr;   // derived
    if (!selectedFromToClassShapes(pFromShape, pToShape))
        return;
    BaseClass*   pBaseClass   = dynamic_cast<BaseClass*>(pFromShape->GetGti());
    ExternClass* pExternClass = dynamic_cast<ExternClass*>(pToShape->GetGti());
    if (!pBaseClass || !pExternClass)
        return;
    Class* pClass = dynamic_cast<Class*>(pExternClass);
    if (pClass && pClass->GetSerialize())
    {
        pClass->GetFirstInherit()->OnEditAttributes();   // serialize: edit fixed base
        raiseToFront();
        update();
        return;
    }
    doc->MarkLastUndo();
    Inherit* pInherit = new Inherit(pExternClass, pBaseClass);
    if (pInherit->OnEditAttributes())
    {
        if (InheritShape* pShape = pInherit->FindInheritShape(_pCD))
        {
            _pViewModel->DeleteAllSelected();
            (void)new ClassDiagramViewModelSelection(_pViewModel, pShape);
        }
        pInherit->Add();
    }
    else
        doc->RollBack();
    doc->MarkLastUndo();
    raiseToFront();
    update();
}

// --- Edit items (mirror OnEditUsersections / OnEditContext / OnEditThrowlist) ---
void ClassDiagramCanvas::editUserSections()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Class* pClass = pSingle ? dynamic_cast<Class*>(pSingle->GetGti()) : nullptr;
    if (!pClass || !_pCD)
        return;
    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
    Qt_ShowUserSectionsDialog(pClass, ownerHwnd);   // dialog owns its own undo
    update();
}

void ClassDiagramCanvas::editContext()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    if (!pGti || !_pCD)
        return;
    pGti->OnEditContext();   // dispatches to the Qt context dialog (Class/Member/...)
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::editExceptionSpecification()
{
    ClassDiagramShape* pSingle = singleSelectedShape();
    Gti* pGti = pSingle ? pSingle->GetGti() : nullptr;
    if (!pGti || !_pCD)
        return;
    pGti->OnEditExceptionSpecification();   // dispatches to the Qt throw-list dialog
    if (_pCD->GetDataModelDoc())
        _pCD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void ClassDiagramCanvas::contextMenuEvent(QContextMenuEvent* e)
{
    if (!_pCD || !_pViewModel)
        return;

    // A right-click while arming an Add Class/Note placement just cancels it.
    if (placementActive())
    {
        cancelPlacement();
        e->accept();
        return;
    }

    // Right-click on an unselected shape selects it first (matches MFC); a
    // right-click that hits an already-selected shape leaves the set alone.
    const QPointF m = widgetToModel(QPointF(e->pos()));
    if (ClassDiagramShape* pHit = hitTest(m))
    {
        if (!findSelection(pHit))
        {
            _pViewModel->DeleteAllSelected();
            (void)new ClassDiagramViewModelSelection(_pViewModel, pHit);
            update();
        }
    }

    ClassDiagramShape* pSingle = singleSelectedShape();
    const bool anySel      = (_pViewModel->GetSelectedCount() > 0);
    const bool openEnabled = pSingle && pSingle->OnOpen(true);
    const bool editEnabled = pSingle && pSingle->OnEditAttributes(true);

    bool anyPenColor = false, anyTextColor = false;
    {
        ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            ClassDiagramShape* p = iSel->GetClassDiagramShape();
            if (!p) continue;
            if (p->UsesPenColor())  anyPenColor  = true;
            if (p->UsesTextColor()) anyTextColor = true;
        }
    }

    QMenu menu(this);
    QAction* aOpen = menu.addAction(tr("&Open"));
    aOpen->setEnabled(openEnabled);
    QAction* aEdit = menu.addAction(tr("&Edit Attributes"));
    aEdit->setEnabled(editEnabled);
    QAction* aDelete = menu.addAction(tr("&Delete"));
    aDelete->setEnabled(anySel);

    // Gates for the model-level Add items + edit items -- mirror the MFC OnUpdate*
    // (each checks the would-be operation via the OnXxx(true) "checkOnly" path).
    Gti* pAddGti = pSingle ? pSingle->GetGti() : nullptr;
    RelationShape* pAddRel = pSingle ? pSingle->GetRelationShape() : nullptr;
    const bool addMemberEnable = pAddGti && pAddGti->OnAddMember(true);
    const bool addMethodEnable =
        (pAddGti && pAddGti->OnAddMethod(true)) ||
        (pAddRel && pAddRel->GetRelation()->GetFromRelation()->OnAddMethod(true));
    const bool addCtorEnable    = pAddGti && pAddGti->OnAddConstructor(true);
    const bool addArgEnable     = pAddGti && pAddGti->OnAddArgument(true);
    const bool addVirtualEnable = pAddGti && pAddGti->OnAddVirtuals(true);
    const bool addIsClassEnable = pAddGti && pAddGti->OnAddIsClassMethods(true);
    // Bare Relation / Inheritance: created from 1-2 selected classes.
    ClassShape* pBareFrom = nullptr;
    ClassShape* pBareTo   = nullptr;
    const bool bareConnEnable = selectedFromToClassShapes(pBareFrom, pBareTo);
    const bool addInheritEnable = pAddGti && pAddGti->OnAddInherit(true);
    const bool addRelationEnable = pAddGti && pAddGti->OnAddRelation(true);

    // "Add" submenu, ordered as: diagram elements (Class / Note) first, then the
    // four connections grouped together (Inheritance / Relation / Relation-Diagram-
    // Only / Dependency -- the least-used diagram-only pair sits below the model
    // ones, not at the top), then the per-class members/methods. Class & Note arm
    // interactive placement; the diagram-only connections are create-on-OK + always
    // enabled; the model-level items mirror the MFC "Add" submenu enable gates.
    QMenu* addMenu = menu.addMenu(tr("&Add"));
    QAction* aAddClass = addMenu->addAction(tr("&Class"));
    QAction* aAddNote  = addMenu->addAction(tr("N&ote"));
    addMenu->addSeparator();
    QAction* aAddInherit = addMenu->addAction(tr("&Inheritance"));
    aAddInherit->setEnabled(addInheritEnable || bareConnEnable);
    QAction* aAddRelation = addMenu->addAction(tr("&Relation"));
    aAddRelation->setEnabled(addRelationEnable || bareConnEnable);
    QAction* aAddRelDiag = addMenu->addAction(tr("Relation (&Diagram Only)"));
    QAction* aAddDepend  = addMenu->addAction(tr("De&pendency"));
    addMenu->addSeparator();
    QAction* aAddMember = addMenu->addAction(tr("&Member"));
    aAddMember->setEnabled(addMemberEnable);
    QAction* aAddMethod = addMenu->addAction(tr("Met&hod"));
    aAddMethod->setEnabled(addMethodEnable);
    QAction* aAddCtor = addMenu->addAction(tr("Co&nstructor"));
    aAddCtor->setEnabled(addCtorEnable);
    QAction* aAddArg = addMenu->addAction(tr("&Argument"));
    aAddArg->setEnabled(addArgEnable);
    addMenu->addSeparator();
    QAction* aAddVirtual = addMenu->addAction(tr("&Virtual Methods"));
    aAddVirtual->setEnabled(addVirtualEnable);
    QAction* aAddIsClass = addMenu->addAction(tr("I&sClass Methods"));
    aAddIsClass->setEnabled(addIsClassEnable);

    // Accelerator hints, right-aligned in the menu (same Add* scheme as the tree;
    // R/I/D/O match the drag-gesture letters). These transient menu actions only
    // register their shortcut while the menu is open; the keyPressEvent Ctrl+Shift
    // block below does the firing when the menu is closed (canvas-scoped).
    aAddClass   ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));
    aAddNote    ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+N")));
    aAddInherit ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+I")));
    aAddRelation->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+R")));
    aAddRelDiag ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+O")));
    aAddDepend  ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+D")));
    aAddMember  ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+B")));
    aAddMethod->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+M")));
    aAddCtor    ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+U")));
    aAddArg     ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+A")));
    aAddVirtual ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+V")));
    aAddIsClass ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+S")));

    // Edit items (single selection) -- User Sections (Class), Exception
    // Specification (Method), Edit/Assign Context (any Gti).
    Gti* pEditGti = pSingle ? pSingle->GetGti() : nullptr;
    QAction* aUserSections = menu.addAction(tr("Edit &User Sections..."));
    aUserSections->setEnabled(dynamic_cast<Class*>(pEditGti) != nullptr);
    QAction* aExcSpec = menu.addAction(tr("Edit Exception Specifica&tion..."));
    aExcSpec->setEnabled(pEditGti && pEditGti->OnEditExceptionSpecification(true));
    QAction* aContext = menu.addAction(tr("Edit/Assign Conte&xt..."));
    aContext->setEnabled(pEditGti && pEditGti->OnEditContext(true));

    menu.addSeparator();
    QAction* aLineColor = menu.addAction(tr("Change &Line Color..."));
    aLineColor->setEnabled(anyPenColor);
    QAction* aTextColor = menu.addAction(tr("Change &Text Color..."));
    aTextColor->setEnabled(anyTextColor);

    // Colour Templates submenu -- document-level defaults, always available.
    QMenu* templatesMenu = menu.addMenu(tr("Color Templates"));
    QVector<QAction*> templateActions;
    for (int i = 0; i < kTemplateColorCount; ++i)
    {
        if (kTemplateColors[i].separatorBefore)
            templatesMenu->addSeparator();
        templateActions.append(templatesMenu->addAction(tr(kTemplateColors[i].title)));
    }

    // View toggles -- mirror the MFC CD menu (OnClassdiagramAutowidth /
    // Showmethodarguments / Showrelationnames / Hide / Showallhidden). The first
    // three act on the single selected ClassShape/RelationShape (checkable); Hide
    // enables when a connection is selected; Show Hidden when the diagram has
    // hidden connections reachable from the selection (or any, with no selection).
    ClassShape*    pTogClass = pSingle ? pSingle->GetClassShape()    : nullptr;
    RelationShape* pTogRel   = pSingle ? pSingle->GetRelationShape() : nullptr;

    bool hideEnable = false;
    {
        ClassDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (!hideEnable && ++iSel)
        {
            ClassDiagramShape* p = iSel->GetClassDiagramShape();
            if (p && p->GetConnectionShape())
                hideEnable = true;
        }
    }
    // Enabled whenever the diagram has anything hidden -- "Show Hidden" is useless
    // otherwise, and (unlike the old MFC gate) it must NOT depend on the selection.
    // Selection only *focuses* the reveal inside showHiddenConnections().
    const bool showHiddenEnable = (_pCD->GetHiddenCount() > 0);

    menu.addSeparator();
    QAction* aAutoWidth = menu.addAction(tr("Auto &Width"));
    aAutoWidth->setEnabled(pTogClass != nullptr);
    aAutoWidth->setCheckable(true);
    aAutoWidth->setChecked(pTogClass && pTogClass->GetAutoWidth());
    QAction* aShowArgs = menu.addAction(tr("Show &Method Arguments"));
    aShowArgs->setEnabled(pTogClass != nullptr);
    aShowArgs->setCheckable(true);
    aShowArgs->setChecked(pTogClass && pTogClass->GetVerbosity());
    QAction* aShowRelNames = menu.addAction(tr("Show Relation &Names"));
    aShowRelNames->setEnabled(pTogRel != nullptr);
    aShowRelNames->setCheckable(true);
    aShowRelNames->setChecked(pTogRel && pTogRel->GetVerbosity());
    menu.addSeparator();
    QAction* aHide = menu.addAction(tr("&Hide"));
    aHide->setEnabled(hideEnable);
    QAction* aShowHidden = menu.addAction(tr("Show Hidde&n"));
    aShowHidden->setEnabled(showHiddenEnable);

    // Select Classes (always available -- picks which classes the diagram shows) /
    // Select Members&Methods (a single class shape's, or diagram-wide when nothing
    // -- or a non-class -- is selected). Mirrors OnUpdateClassSelectmembersmethods.
    menu.addSeparator();
    QAction* aSelectClasses = menu.addAction(tr("Select &Classes..."));
    QAction* aSelectMembers = menu.addAction(tr("Select Me&mbers && Methods..."));
    aSelectMembers->setEnabled((pSingle && pSingle->GetClassShape()) ||
                               _pViewModel->GetSelectedCount() == 0);

    // --- Layout / navigation (Optimize Placement / Align / Reorder / Inherits) ---
    // Gates mirror the MFC OnUpdate* handlers.
    int classCnt = 0;
    {
        ClassDiagram::ClassDiagramShapeIterator
            iClassShape(_pCD, &ClassDiagramShape::IsClassShape);
        while (++iClassShape && classCnt < 2)
            classCnt++;
    }
    const bool optimizeEnable = (classCnt >= 2);
    const bool alignEnable    = canAlign();

    ClassShape* pReorderClass = pSingle ? pSingle->GetClassShape() : nullptr;
    const bool reorderEnable = pReorderClass &&
        (pReorderClass->GetMemberShapeCount() > 1 ||
         pReorderClass->GetMethodShapeCount() > 1);

    Gti* pSingleGti = pSingle ? pSingle->GetGti() : nullptr;
    ExternClass* pInhFromCls = dynamic_cast<ExternClass*>(pSingleGti);
    BaseClass*   pInhByCls   = dynamic_cast<BaseClass*>(pSingleGti);
    const bool inheritsFromEnable = pInhFromCls && pInhFromCls->GetInheritCount() > 0;
    const bool inheritedByEnable  = pInhByCls && pInhByCls->GetInheritCount()   > 0;

    menu.addSeparator();
    QAction* aOptimize = menu.addAction(tr("&Optimize Placement"));
    aOptimize->setEnabled(optimizeEnable);

    QMenu* alignMenu = menu.addMenu(tr("Ali&gn"));
    alignMenu->setEnabled(alignEnable);
    QAction* aAlignLeft   = alignMenu->addAction(tr("&Left"));
    QAction* aAlignCenter = alignMenu->addAction(tr("&Center"));
    QAction* aAlignRight  = alignMenu->addAction(tr("&Right"));
    alignMenu->addSeparator();
    QAction* aAlignTop    = alignMenu->addAction(tr("&Top"));
    QAction* aAlignMiddle = alignMenu->addAction(tr("&Middle"));
    QAction* aAlignBottom = alignMenu->addAction(tr("&Bottom"));

    QAction* aReorder = menu.addAction(tr("&Reorder Members && Methods..."));
    aReorder->setEnabled(reorderEnable);

    menu.addSeparator();
    QAction* aInheritsFrom = menu.addAction(tr("&Inherits From..."));
    aInheritsFrom->setEnabled(inheritsFromEnable);
    QAction* aInheritedBy = menu.addAction(tr("Inherited &By..."));
    aInheritedBy->setEnabled(inheritedByEnable);

    QAction* chosen = menu.exec(e->globalPos());
    e->accept();
    if (!chosen || !_pCD || !_pViewModel)
        return;

    if      (chosen == aOpen)      openSelected(/*ctrlHeld=*/false);
    else if (chosen == aEdit)      openSelected(/*ctrlHeld=*/true);
    else if (chosen == aDelete)    deleteSelected();
    else if (chosen == aAddRelDiag)
        addConnectionFromSelection(AddConnectionKind::DiagramOnlyRelation);
    else if (chosen == aAddDepend)
        addConnectionFromSelection(AddConnectionKind::Dependency);
    else if (chosen == aLineColor) changeLineColor();
    else if (chosen == aTextColor) changeTextColor();
    else if (chosen == aAutoWidth)    toggleAutoWidth();
    else if (chosen == aShowArgs)     toggleShowMethodArguments();
    else if (chosen == aShowRelNames) toggleShowRelationNames();
    else if (chosen == aHide)         hideSelectedConnections();
    else if (chosen == aShowHidden)   showHiddenConnections();
    else if (chosen == aSelectClasses) selectClasses();
    else if (chosen == aSelectMembers) selectMembersAndMethods();
    else if (chosen == aOptimize)      optimizePlacement();
    else if (chosen == aAlignLeft)     alignSelected(AlignKind::Left);
    else if (chosen == aAlignCenter)   alignSelected(AlignKind::Center);
    else if (chosen == aAlignRight)    alignSelected(AlignKind::Right);
    else if (chosen == aAlignTop)      alignSelected(AlignKind::Top);
    else if (chosen == aAlignMiddle)   alignSelected(AlignKind::Middle);
    else if (chosen == aAlignBottom)   alignSelected(AlignKind::Bottom);
    else if (chosen == aReorder)       reorderMembersMethods();
    else if (chosen == aInheritsFrom)  inheritsFrom();
    else if (chosen == aInheritedBy)   inheritedBy();
    else if (chosen == aAddClass)      beginPlacement(PlacementKind::Class);
    else if (chosen == aAddNote)       beginPlacement(PlacementKind::Note);
    else if (chosen == aAddInherit)    addBareInheritance();
    else if (chosen == aAddRelation)   addBareRelation();
    else if (chosen == aAddMember)     addMember();
    else if (chosen == aAddMethod)   addMethod();
    else if (chosen == aAddCtor)       addConstructor();
    else if (chosen == aAddArg)        addArgument();
    else if (chosen == aAddVirtual)    addVirtualMethods();
    else if (chosen == aAddIsClass)    addIsClassMethods();
    else if (chosen == aUserSections)  editUserSections();
    else if (chosen == aExcSpec)       editExceptionSpecification();
    else if (chosen == aContext)       editContext();
    else if (templateActions.contains(chosen))
        changeTemplateColor(templateActions.indexOf(chosen));
}

// One undo-stack entry is safe to apply unless it's the creation (UndoNew) of
// a diagram that still has an open view -- undoing it deletes the diagram out
// from under that view. Mirrors CClassBuilderDoc::OnUpdateEditUndo, plus an
// explicit guard for THIS view's own diagram (_pCD). Mirrors the SD canvas.
// Undo/Redo enablement is ONE rule for every view -- "is there a step on the
// stack?" -- delegated to the doc (CClassBuilderDoc::CanUndo/CanRedo) so the
// tree, CD and SD buttons behave identically. No per-entry walk/deref here.
bool ClassDiagramCanvas::canUndo() const
{
    DataModelDoc* doc = _pCD ? _pCD->GetDataModelDoc() : nullptr;
    return doc && doc->CanUndo();
}

bool ClassDiagramCanvas::canRedo() const
{
    DataModelDoc* doc = _pCD ? _pCD->GetDataModelDoc() : nullptr;
    return doc && doc->CanRedo();
}

void ClassDiagramCanvas::undo()
{
    if (!canUndo()) return;
    // DataModelDoc::Undo refreshes every view of the model by construction
    // (the old per-canvas UpdateClassDiagramViews()+update() here compensated
    // for entry points that didn't -- gone with that gap).
    _pCD->GetDataModelDoc()->Undo();
}

void ClassDiagramCanvas::redo()
{
    if (!canRedo()) return;
    _pCD->GetDataModelDoc()->Redo();   // see undo()
}

// ---------------------------------------------------------------------------
// ClassDiagramQtView (hosting window)
// ---------------------------------------------------------------------------
ClassDiagramQtView::ClassDiagramQtView(ClassDiagram* pClassDiagram,
                                       QWidget* parent)
    : QDialog(parent)
    , _canvas(new ClassDiagramCanvas(pClassDiagram, this))
{
    setWindowTitle(QString("CD: %1")
                       .arg(pClassDiagram ? toQ(pClassDiagram->GetName())
                                          : QString()));
    resize(900, 700);

    auto* lay = new QGridLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Per-window toolbar: the full "Add" set (same actions + enables as the
    // canvas context menu), then zoom. Icon-only with model icons (the tree's
    // glyphs) + tooltips; the Add buttons grey out exactly as the menu would,
    // refreshed live on selection change.
    auto* tb = new QToolBar(this);
    tb->setMovable(false);
    tb->setIconSize(QSize(CB_TOOLBAR_ICON_PX, CB_TOOLBAR_ICON_PX));   // icon-only; tooltips name each
    tb->setStyleSheet("QToolBar{border:0;spacing:1px;}"
                      "QToolButton{padding:1px;}");
    if (tb->layout())
        tb->layout()->setContentsMargins(0, 0, 0, 0);

    // The exact MFC main-toolbar glyphs (res/Toolbar.bmp via Qt_ToolBarIcon),
    // so Add Note / Inheritance / Dependency / ... look as they always did.
    struct AddDef { ClassDiagramCanvas::AddItem item; int glyph; const char* tip; };
    static const AddDef adds[] = {
        { ClassDiagramCanvas::AddClass,               TG_ADD_CLASS,                "Add Class (click to place)" },
        { ClassDiagramCanvas::AddNote,                TG_ADD_NOTE,                 "Add Note (click to place)" },
        { ClassDiagramCanvas::AddInheritance,         TG_ADD_INHERIT,              "Add Inheritance" },
        { ClassDiagramCanvas::AddRelation,            TG_ADD_RELATION,             "Add Relation" },
        { ClassDiagramCanvas::AddRelationDiagramOnly, TG_ADD_RELATION_DIAGRAMONLY, "Add Relation (Diagram Only)" },
        { ClassDiagramCanvas::AddDependency,          TG_ADD_DEPENDENCY,           "Add Dependency" },
        { ClassDiagramCanvas::AddMember,              TG_ADD_MEMBER,               "Add Member" },
        { ClassDiagramCanvas::AddMethod,            TG_ADD_FUNCTION,             "Add Method" },
        { ClassDiagramCanvas::AddConstructor,         TG_ADD_CONSTRUCTOR,          "Add Constructor" },
        { ClassDiagramCanvas::AddArgument,            TG_ADD_ARGUMENT,             "Add Argument" },
        { ClassDiagramCanvas::AddVirtualMethods,      TG_ADD_VIRTUALS,             "Add Virtual Methods" },
        { ClassDiagramCanvas::AddIsClassMethods,      TG_ADD_ISCLASS,              "Add IsClass Methods" },
    };
    for (const AddDef& d : adds)
    {
        const int item = d.item;
        QAction* a = tb->addAction(Qt_ToolBarIcon(d.glyph), d.tip, this,
                                   [this, item] { _canvas->runAddItem(item); });
        a->setToolTip(d.tip);
        _addActions.append(qMakePair(item, a));
    }

    // Align: ONE toolbar button (icon + dropdown arrow) that folds out the six
    // align options downward, instead of six buttons eating the bar. Same actions
    // as the context-menu "Align" submenu; the last-selected Class/Note is the
    // reference (the darker-select anchor frame on the canvas). Enable-gated like
    // Delete -- needs >=2 alignable shapes (refreshAddActions, on selectionChanged).
    tb->addSeparator();
    {
        QMenu* alignMenu = new QMenu(this);
        struct AlignDef { ClassDiagramCanvas::AlignKind kind; const char* icon; const char* text; };
        static const AlignDef aligns[] = {
            { ClassDiagramCanvas::AlignKind::Left,   ":/icons/align_horizontal_left.svg",   "&Left"   },
            { ClassDiagramCanvas::AlignKind::Center, ":/icons/align_horizontal_center.svg", "&Center" },
            { ClassDiagramCanvas::AlignKind::Right,  ":/icons/align_horizontal_right.svg",  "&Right"  },
            { ClassDiagramCanvas::AlignKind::Top,    ":/icons/align_vertical_top.svg",      "&Top"    },
            { ClassDiagramCanvas::AlignKind::Middle, ":/icons/align_vertical_center.svg",   "&Middle" },
            { ClassDiagramCanvas::AlignKind::Bottom, ":/icons/align_vertical_bottom.svg",   "&Bottom" },
        };
        int i = 0;
        for (const AlignDef& d : aligns)
        {
            if (i++ == 3) alignMenu->addSeparator();   // horizontal group | vertical group
            const ClassDiagramCanvas::AlignKind kind = d.kind;
            alignMenu->addAction(QIcon(QString::fromLatin1(d.icon)), tr(d.text),
                                 this, [this, kind] { _canvas->alignSelected(kind); });
        }
        _alignButton = new QToolButton(tb);
        _alignButton->setMenu(alignMenu);
        _alignButton->setPopupMode(QToolButton::InstantPopup);
        _alignButton->setAutoRaise(true);
        _alignButton->setIconSize(QSize(CB_TOOLBAR_ICON_PX, CB_TOOLBAR_ICON_PX));
        _alignButton->setIcon(QIcon(QStringLiteral(":/icons/align_horizontal_left.svg")));
        _alignButton->setToolTip(tr("Align selected shapes to the last-selected (reference)"));
        tb->addWidget(_alignButton);
    }

    // Edit: Delete (selected) + Undo/Redo, using the exact MFC toolbar glyphs at
    // the shared CB_TOOLBAR_ICON_PX -- icon-only at the same size as the Add
    // buttons, so the bar height is unchanged. Undo/Redo stay always-enabled
    // (the canvas's undo()/redo() no-op when the stack is empty); Delete greys
    // out when nothing is selected (refreshAddActions, on selectionChanged).
    tb->addSeparator();
    _deleteAction = tb->addAction(Qt_ToolBarIcon(TG_EDIT_DELETE), "Delete",
                                  this, [this] { _canvas->editDeleteSelected(); });
    _deleteAction->setToolTip("Delete selected (Del)");
    _undoAction = tb->addAction(Qt_ToolBarIcon(TG_EDIT_UNDO), "Undo",
                                this, [this] { _canvas->editUndo(); });
    _undoAction->setToolTip("Undo (Ctrl+Z)");
    _redoAction = tb->addAction(Qt_ToolBarIcon(TG_EDIT_REDO), "Redo",
                                this, [this] { _canvas->editRedo(); });
    _redoAction->setToolTip("Redo (Ctrl+Y)");

    tb->addSeparator();
    QAction* zi = tb->addAction(QIcon::fromTheme(QStringLiteral("zoom-in")),
                                "Zoom In", this, [this] { _canvas->applyToolbarZoom(+1); });
    zi->setToolTip("Zoom in (Ctrl+wheel / Ctrl+=)");
    QAction* zo = tb->addAction(QIcon::fromTheme(QStringLiteral("zoom-out")),
                                "Zoom Out", this, [this] { _canvas->applyToolbarZoom(-1); });
    zo->setToolTip("Zoom out (Ctrl+wheel / Ctrl+-)");
    QAction* zf = tb->addAction(QIcon::fromTheme(QStringLiteral("zoom-fit-best")),
                                "Fit", this, [this] { _canvas->applyToolbarZoom(0); });
    zf->setToolTip("Reset to fit-to-window (double-click)");
    tb->addSeparator();
    QAction* sx = tb->addAction(QIcon::fromTheme(QStringLiteral("document-send")),
                                "Export SVG", this, [this] { exportSvg(); });
    sx->setToolTip("Export this diagram as a vector .svg file");

    // Grid: toolbar across the top, canvas with scrollbars below (same layout as
    // the SD view, so the CD now has scrollbars too -- they sync to pan/zoom).
    QScrollBar* hbar = new QScrollBar(Qt::Horizontal, this);
    QScrollBar* vbar = new QScrollBar(Qt::Vertical, this);
    lay->addWidget(tb,      0, 0, 1, 2);
    lay->addWidget(_canvas, 1, 0);
    lay->addWidget(vbar,    1, 1);
    lay->addWidget(hbar,    2, 0);
    lay->setRowStretch(1, 1);
    lay->setColumnStretch(0, 1);
    _canvas->bindScrollBars(hbar, vbar);

    connect(_canvas, &ClassDiagramCanvas::selectionChanged,
            this, &ClassDiagramQtView::refreshAddActions);
    refreshAddActions();
}

void ClassDiagramQtView::exportSvg()
{
    QString def = _canvas->diagramName();
    if (def.isEmpty())
        def = "classdiagram";
    // macOS: force Qt's own file dialog (the native NSSavePanel does not appear
    // in this app -- same gap as the File Open panel in QtShellWindow).
#ifdef __APPLE__
    const QFileDialog::Options svgOpts = QFileDialog::DontUseNativeDialog;
#else
    const QFileDialog::Options svgOpts = QFileDialog::Options();
#endif
    QString path = QFileDialog::getSaveFileName(
        this, "Export Diagram as SVG", def + ".svg", "SVG files (*.svg)",
        nullptr, svgOpts);
    if (path.isEmpty())
        return;
    if (!path.endsWith(".svg", Qt::CaseInsensitive))
        path += ".svg";
    if (!_canvas->exportSvg(path))
        QMessageBox::warning(this, "Export SVG",
            "SVG export failed -- the Qt Svg module may be unavailable in this build.");
}

void ClassDiagramQtView::refreshAddActions()
{
    for (const auto& pair : _addActions)
        pair.second->setEnabled(_canvas->addItemEnabled(pair.first));
    if (_deleteAction)
        _deleteAction->setEnabled(_canvas->hasSelection());
    if (_alignButton)
        _alignButton->setEnabled(_canvas->canAlign());
    refreshUndoRedoEnables();
}

// Enable-only refresh of this view's Undo/Redo buttons -- invoked by name from
// the shell on every open view when any view broadcasts a state change, so all
// views of a model agree on undo/redo availability.
void ClassDiagramQtView::refreshUndoRedoEnables()
{
    if (_undoAction)
        _undoAction->setEnabled(_canvas->canEditUndo());
    if (_redoAction)
        _redoAction->setEnabled(_canvas->canEditRedo());
}

ClassDiagramQtView::~ClassDiagramQtView()
{
}

bool ClassDiagramQtView::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate)
        Qt_SetActiveDiagramZoom(_canvas, [](QWidget* w, int op) {
            static_cast<ClassDiagramCanvas*>(w)->applyToolbarZoom(op);
        });
    return QDialog::event(e);
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowClassDiagramView(ClassDiagram* pClassDiagram, void* ownerHwnd)
{
    Qt_EnsureApplication();

    auto* w = new ClassDiagramQtView(pClassDiagram);
    // Prefer a dockable shell dock (floating by default, tab/dock like a tree).
    // The dock takes WA_DeleteOnClose + ownership; fall back to a standalone
    // top-level window only if there's no shell.
    if (!Qt_HostDiagramDock(w))
    {
        w->setAttribute(Qt::WA_DeleteOnClose, true);
        Qt_ShowModeless(*w, ownerHwnd);
    }
}
