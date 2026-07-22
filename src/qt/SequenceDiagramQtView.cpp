// qt/SequenceDiagramQtView.cpp -- see header.
//
// M2.0: the modal scaffold from M1 is promoted to a non-modal top-level
// window. The canvas constructs a SequenceDiagramViewModel that wires the
// model's update path back to a Qt repaint via a static C-style callback.
// Interaction lands in M2.1+.

#include "SequenceDiagramQtView.h"

#include "QtSequenceDiagramView.h"   // bridge declaration
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ShowModeless
#include "QtMenuStyle.h"
#include "QtDesktopTheme.h"   // Cb_FileDialogOptions             // Qt_ApplyCompactMenuStyle (consistent menus)
#include "QtModelText.h"             // toQ
#include "CbPainter_QPainter.h"
#include "QtHandleMetrics.h"         // QtHandle::grabToleranceModel
#include "QtToolBarIcons.h"          // Qt_ToolBarIcon (real MFC toolbar glyphs)
#include "CbPainter_QFontMetrics.h"  // headless text measurement for hit-test

#include <QApplication>              // allWidgets (pipe SVG-export canvas lookup)
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QNativeGestureEvent>
#include <QResizeEvent>
#include <QGridLayout>
#include <QToolBar>
#include <QDockWidget>
#include <QRubberBand>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QtMath>
#ifdef CB_HAVE_SVG
#include <QSvgGenerator>
#endif
#include <QMenu>
#include <QContextMenuEvent>
#include <QColorDialog>
#include <QPolygon>                  // QPolygonF (message-drag arrowhead)
#include <QtGlobal>

#include <cmath>                     // std::hypot (message-drag arrowhead)


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

#include "QtSignalDialog.h"          // Qt_ShowSignalDialog (Add Message)
#include "QtLifeLineDialog.h"        // Qt_ShowLifeLineDialogNew (Add Lifeline)

// ---------------------------------------------------------------------------
// SequenceDiagramCanvas
// ---------------------------------------------------------------------------
SequenceDiagramCanvas::SequenceDiagramCanvas(SequenceDiagram* pSD,
                                             QWidget* parent)
    : QWidget(parent)
    , _pSD(pSD)
    , _pViewModel(nullptr)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    // Keyboard focus -- needed for Ctrl+0 / Ctrl++ / Ctrl+-.
    setFocusPolicy(Qt::StrongFocus);

    // Mouse-move events without a button held -- needed for hover-driven
    // cursor change in mouseMoveEvent.
    setMouseTracking(true);

    // Feed the app accent into CbPainter as the selection colours -- a solid
    // tone for pens/outlines/message lines, a light tint for box interiors --
    // so the shapes' Draw(vm, selected) can recolour selected shapes without
    // knowing the Qt palette. Mirrors the ClassDiagram canvas.
    const QColor sc = palette().color(QPalette::Active, QPalette::Highlight);
    CbPainter::SetSelectColor(Cb_RGB(sc.red(), sc.green(), sc.blue()));
    CbPainter::SetSelectFillColor(Cb_RGB(
        int(sc.red()   * 0.30 + 255 * 0.70),
        int(sc.green() * 0.30 + 255 * 0.70),
        int(sc.blue()  * 0.30 + 255 * 0.70)));

    // Construct the ViewModel that represents "this open Qt view" in the
    // model. Lifetime is tied to this canvas EITHER way:
    //  - canvas going down first -> dtor sets _destructing then deletes
    //    the ViewModel; the close callback fires but no-ops on the flag.
    //  - SequenceDiagram going down first -> CB cascade deletes the
    //    ViewModel; its dtor invokes CloseCanvas which nulls our model
    //    pointers and schedules a Qt close on this widget.
    if (_pSD)
    {
        _pViewModel = new SequenceDiagramViewModel(
            _pSD,
            &SequenceDiagramCanvas::RefreshCanvas,
            &SequenceDiagramCanvas::CloseCanvas,
            this);
    }
}

SequenceDiagramCanvas::~SequenceDiagramCanvas()
{
    // The ViewModel is owned via CB's intrusive list on the SequenceDiagram;
    // deleting it unlinks from that list AND fires our close callback. Set
    // _destructing first so CloseCanvas sees we're already going down and
    // no-ops (we don't want to re-enter ourselves via close()).
    _destructing = true;
    if (_pViewModel)
    {
        delete _pViewModel;
        _pViewModel = nullptr;
    }
}

// Posted as a Qt repaint -- not invoked directly: the model side may call
// from any code path, including ones that hold transient state (e.g. mid-
// SaveState). update() schedules; doesn't paint synchronously.
void SequenceDiagramCanvas::RefreshCanvas(void* ctx)
{
    if (auto* self = static_cast<SequenceDiagramCanvas*>(ctx))
    {
        self->update();
        // Once-per-coalesced-op model-change callback: let the owning view
        // re-evaluate its toolbar Edit enables (Undo/Redo track the stack).
        emit self->editActionsChanged();
        // Broadcast so the OTHER open views of this model refresh their undo/redo
        // enables too -- this canvas's repaint never reaches them. Routed through
        // DataModelDoc so only IT knows the framework doc.
        if (self->_pSD)
            self->_pSD->GetDataModelDoc()->NotifyStateChanged();
    }
}

bool SequenceDiagramCanvas::hasSelection() const
{
    return _pViewModel && _pViewModel->GetSelectedCount() > 0;
}

// Fired from ~SequenceDiagramViewModel when the model side is tearing it
// down (e.g. SequenceDiagram cascade, document close). The canvas-going-
// down-first path sets _destructing first; on that path this is a no-op.
void SequenceDiagramCanvas::CloseCanvas(void* ctx)
{
    if (auto* self = static_cast<SequenceDiagramCanvas*>(ctx))
        self->notifyModelGone();
}

void SequenceDiagramCanvas::notifyModelGone()
{
    if (_destructing)
        return;
    // The ViewModel object that's invoking us is mid-destruction -- forget
    // the pointer immediately so nothing in this canvas tries to dereference
    // it. _pSD is also moot from here: if we're being closed because the SD
    // is going away, the SD is mid-destruction too; if it's a document
    // close, our pointer is about to dangle. Either way, drop it.
    _pViewModel = nullptr;
    _pSD        = nullptr;
    // Schedule the Qt window to close -- bubbles up via QWidget::close()
    // through this widget's top-level window (the SequenceDiagramQtView
    // dialog), which was constructed with WA_DeleteOnClose so it
    // self-destructs from there.
    //
    // When hosted in a QDockWidget (docked OR floated as a dock), close the DOCK
    // -- window() would resolve to the SHELL when docked, which must not close.
    QWidget* w = this;
    while (w && !qobject_cast<QDockWidget*>(w))
        w = w->parentWidget();
    if (w)
        w->close();                       // the dock: WA_DeleteOnClose deletes us
    else if (QWidget* top = window())
        top->close();
}

// Single source of truth for the page-size + fit-to-window numbers. Both
// paintEvent and updateScrollBars use this so the rendered transform and
// the scrollbar geometry stay in sync.
SequenceDiagramCanvas::FitInfo SequenceDiagramCanvas::computeFit() const
{
    FitInfo f;
    const int pageW = _pSD ? _pSD->GetWidth()  : 0;
    const int pageH = _pSD ? _pSD->GetHeight() : 0;
    f.pageW = pageW > 0 ? pageW : 2100;     // SD default A4 (mm*10)
    f.pageH = pageH > 0 ? pageH : 2970;

    const qreal margin  = 8.0;
    const qreal usableW = qMax<qreal>(1, width()  - 2 * margin);
    const qreal usableH = qMax<qreal>(1, height() - 2 * margin);
    f.fitScale = qMin(usableW / f.pageW, usableH / f.pageH);

    const qreal drawnW = f.pageW * f.fitScale;
    const qreal drawnH = f.pageH * f.fitScale;
    f.originX = (width()  - drawnW) / 2.0;
    f.originY = (height() - drawnH) / 2.0;
    f.scaledW = f.pageW * f.fitScale * _zoom;
    f.scaledH = f.pageH * f.fitScale * _zoom;
    return f;
}

// Paint: fit the SequenceDiagram page (0,0 .. pageW, pageH in model coords,
// with model Y up) into the widget. Grey outside the page, white inside --
// matches the MFC printable-canvas affordance. The same shape Draw methods
// the MFC view calls; only the painter backend changes.
void SequenceDiagramCanvas::paintEvent(QPaintEvent* /*event*/)
{
    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing,     true);
    qp.setRenderHint(QPainter::TextAntialiasing, true);

    // Grey margin -- visible wherever the scaled page doesn't cover the
    // widget. Read from the palette so dark/light themes stay consistent.
    qp.fillRect(rect(), palette().color(QPalette::Window).darker(118));

    if (!_pSD)
        return;

    // Selection changed (no model edit) -> refresh the toolbar Edit enables
    // (Delete tracks selection). RefreshCanvas covers model/stack changes; this
    // covers selection-only changes, which don't fire RefreshCanvas.
    if (_pViewModel)
    {
        quintptr sig = quintptr(_pViewModel->GetSelectedCount()) * 1099511628211ULL
                     ^ reinterpret_cast<quintptr>(singleSelectedShape());
        if (sig != _editSelSig)
        {
            _editSelSig = sig;
            emit editActionsChanged();
        }
    }

    const FitInfo f = computeFit();

    // User zoom + pan, composed BEFORE the fit-to-window transform so
    // they read in widget coords. With the defaults (_zoom=1, _pan=0)
    // these are no-ops and the rendering reduces to plain fit-to-window.
    qp.translate(_pan);
    qp.scale(_zoom, _zoom);

    // Model coords -> device coords (the MFC view uses MM_ISOTROPIC with a
    // *negative* vertical viewport extent, so model Y points up and screen Y
    // points down; the scale(1, -1) here is the same flip).
    qp.translate(f.originX, f.originY);
    qp.scale(f.fitScale, -f.fitScale);

    // The printable canvas: white fill + thin 1px border (cosmetic pen so
    // the line stays one device pixel at any zoom). The SD model uses Y-up
    // with the page going from (0, -pageH) to (pageW, 0) -- top-left at the
    // model origin -- so the rect's model-Y range is [-pageH, 0], not
    // [0, pageH]. Drawing it at (0, 0, pageW, pageH) puts it on the wrong
    // side of the X axis and the shapes (which sit at modelY in [-pageH, 0])
    // end up "below" the white rect in the widget.
    const QRectF pageRect(0, -f.pageH, f.pageW, f.pageH);
    qp.fillRect(pageRect, Qt::white);

    // Multi-page print preview -- pale blue dotted lines marking where the
    // printer's page breaks would fall, so the user can avoid placing labels
    // / shapes across them. Mirrors SequenceDiagramView::OnDraw: _multiPage
    // is a power-of-2 exponent (GetNumberOfPages = 1 << _multiPage), and the
    // page grid is laid out by alternately doubling the smaller-aspect axis
    // first -- portrait doubles rows first, landscape doubles columns first.
    if (_pSD->GetMultiPage() > 0)
    {
        int nx = 1, ny = 1;
        const int total = _pSD->GetNumberOfPages();
        if (_pSD->GetWidth() < _pSD->GetHeight())   // portrait
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
        qp.setPen(QPen(QColor(170, 143, 191), 0, Qt::DotLine));  // cosmetic
        const qreal ox = f.pageW / qreal(nx);
        const qreal oy = f.pageH / qreal(ny);
        for (int ix = 1; ix < nx; ++ix)
            qp.drawLine(QPointF(ox * ix, 0), QPointF(ox * ix, -f.pageH));
        for (int iy = 1; iy < ny; ++iy)
            qp.drawLine(QPointF(0, -oy * iy), QPointF(f.pageW, -oy * iy));
        qp.restore();
    }

    CbPainter_QPainter painter(&qp);
    // Selection is drawn IN the shapes' Draw(vm, selected) now (per-view, via
    // the ViewModel's ClassDiagram-style selection list) -- no separate overlay
    // pass. Each shape recolours/fills itself when selected in THIS view.
    _pSD->Draw(painter, _pViewModel);

    paintDragGhost(qp);
    paintPlacementGhost(qp);
    paintDropGhost(qp);         // footprint hint while Ctrl-dragging a tree class/actor in
    paintMessageDragLine(qp);
    paintSignalMoveGhost(qp);
    paintTextDragGhost(qp);
    paintNoteDragGhost(qp);
    paintClassLifeLineResizeGhost(qp);

    qp.save();
    qp.setPen(QPen(QColor(0x80, 0x80, 0x80), 0));   // cosmetic 1px grey
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(pageRect);
    qp.restore();
}

void SequenceDiagramCanvas::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    updateScrollBars();
}

// Inverse of the paintEvent transform stack -- widget coords back to model.
// Forward is:
//   widgetPt = _pan + _zoom * (origin + fitScale * (modelX, -modelY))
// Inverse:
//   m  = (widget - _pan) / _zoom
//   m' = (m - origin) / fitScale       (model X, then flip the Y back)
QPointF SequenceDiagramCanvas::widgetToModel(QPointF widgetPt) const
{
    const FitInfo f = computeFit();
    const qreal x = ((widgetPt.x() - _pan.x()) / _zoom - f.originX) / f.fitScale;
    const qreal y = -((widgetPt.y() - _pan.y()) / _zoom - f.originY) / f.fitScale;
    return QPointF(x, y);
}

// All canvas hit-tests funnel through here. We hand SignalShape a headless
// CbPainter_QFontMetrics for the duration of the GetHitShape call so its
// PointInShape can measure the name/label/return TEXT rects off-view -- so a
// click on a message's method-name text selects it, not just its arrow. The
// static hook is set/cleared synchronously around the single-threaded hit-test.
SequenceDiagramShape* SequenceDiagramCanvas::hitTest(const CbPoint& modelPt) const
{
    if (!_pSD)
        return nullptr;
    CbPainter_QFontMetrics measure;
    SignalShape::SetMeasurePainter(&measure);  
    SequenceDiagramShape* pHit = _pSD->GetHitShape(_pViewModel, modelPt, true);   // Qt ViewModel hit-test path
    SignalShape::SetMeasurePainter(nullptr);
    return pHit;
}

// ---------------------------------------------------------------------------
// Selection helpers (used by keyboard navigation + Del + Esc)
// ---------------------------------------------------------------------------
SequenceDiagramShape* SequenceDiagramCanvas::singleSelectedShape() const
{
    if (!_pViewModel)
        return nullptr;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    if (!++iSel)
        return nullptr;
    SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
    if (++iSel)                                  // more than one -> not single
        return nullptr;
    return pShape;
}

// Replace the current selection set with just `pNew` (clears everything, then
// inserts a new junction). The repaint is the caller's job.
void SequenceDiagramCanvas::replaceSelection(SequenceDiagramShape* pNew)
{
    if (!_pViewModel)
        return;
    _pViewModel->DeleteAllSelected();
    if (pNew)
        (void)new SequenceDiagramViewModelSelection(_pViewModel, pNew);
}

// --- Cross-view drop (tree Ctrl-drag onto this SD canvas) ------------------
bool SequenceDiagramCanvas::dropFromTree(Gti* pGti, QPoint globalPos)
{
    if (!_pSD || !_pViewModel || !pGti)
        return false;
    if (!pGti->DropTarget(true, _pSD))   // same gate the MFC tree drag uses
        return false;

    DataModelDoc* doc = _pSD->GetDataModelDoc();
    const QPointF mm = widgetToModel(mapFromGlobal(globalPos));
    CbPoint point(qRound(mm.x()), 0);   // cursor X; the lifeline sits on the header row
    Shape::Round(point);

    SequenceDiagramShape* pShape = nullptr;
    if (BaseClass* pBaseClass = dynamic_cast<BaseClass*>(pGti))
        pShape = new ClassLifeLineShape(_pSD, pBaseClass, point);
    else if (Actor* pActor = dynamic_cast<Actor*>(pGti))
        pShape = new ActorLifeLineShape(_pSD, pActor, point);
    if (!pShape)
        return false;

    _pViewModel->DeleteAllSelected();
    (void)new SequenceDiagramViewModelSelection(_pViewModel, pShape);
    doc->MarkLastUndo();
    return true;
}

bool SequenceDiagramCanvas::showDropGhost(Gti* pGti, QPoint globalPos)
{
    if (!_pSD || !pGti || !pGti->DropTarget(true, _pSD))
    {
        clearDropGhost();
        return false;
    }
    _dropGhostModelPt = widgetToModel(mapFromGlobal(globalPos));
    _dropGhostActive  = true;
    update();
    return true;
}

void SequenceDiagramCanvas::clearDropGhost()
{
    if (_dropGhostActive)
    {
        _dropGhostActive = false;
        update();
    }
}

void SequenceDiagramCanvas::paintDropGhost(QPainter& qp)
{
    if (!_dropGhostActive)
        return;
    // 250 x lifeline-height footprint on the lifeline row at the cursor X,
    // matching paintPlacementGhost's LifeLine branch + where the lifeline lands.
    const qreal w  = 250;
    const qreal h  = SequenceDiagram::GetClassLifeLineHeight();
    const qreal ax = _dropGhostModelPt.x();
    const qreal ay = -SequenceDiagram::GetClassLifeLineOffset()
                     + SequenceDiagram::GetClassLifeLineHeight();
    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(QRectF(ax, ay - h, w, h));
    qp.restore();
}

void SequenceDiagramCanvas::clearSelection()
{
    if (!_pViewModel || _pViewModel->GetSelectedCount() == 0)
        return;
    _pViewModel->DeleteAllSelected();
    update();
}

// Del -- mirror SequenceDiagramView::OnDelete: ask each selected shape via
// OnDelete(checkOnly=true), and if it consents, delete it. The model's
// own UpdateSequenceDiagramViews path (and our tail on UnLockAndUpdateAllViews)
// refresh the canvas; mark one undo entry at the end.
void SequenceDiagramCanvas::deleteSelected()
{
    if (!_pViewModel || _pViewModel->GetSelectedCount() == 0)
        return;
    // Walk the live selection relation directly -- its iterator is mutation-safe, so
    // deleting during the walk is fine. Deleting a shape cascades to its children (a
    // class header takes its activations; a signal takes its receiver activation) and
    // removes THEIR selection junctions from this same list, so they're never visited a
    // second time. The earlier QVector snapshot was the bug: it held stale pointers to
    // cascade-deleted shapes and deleted them again, corrupting the undo stack.
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
        if (pShape && pShape->OnDelete(true))
            pShape->OnDelete(false);
    }
    // Surviving shapes reposition (activations below a deleted one shift up); recompute
    // and carry the attached notes, same as every edit. The no-shape RecalculateAfterEdit
    // IS the delete path -- it recomputes and follows, owning the move-list internally.
    // Before MarkLastUndo so the note moves land in this undo entry.
    if (_pSD)
        _pSD->RecalculateAfterEdit();
    if (_pSD && _pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// ---------------------------------------------------------------------------
// Navigation (plain arrows) -- mirrors MFC OnKeyLeft/Right/Up/Down.
// Operates only when exactly one shape is selected.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::navigateLeft()
{
    SequenceDiagramShape* p = singleSelectedShape();
    if (!p) return;
    if (LifeLineShape* pLL = p->GetLifeLine())
    {
        if (LifeLineShape* pNew = _pSD->GetPrevLifeLineShape(pLL))
        { replaceSelection(pNew); update(); }
        return;
    }
    if (SignalShape* pSig = p->GetSignal())
    {
        if (ChildActivationShape* pNew = pSig->GetSender())
        { replaceSelection(pNew); update(); }
        return;
    }
    if (ChildActivationShape* pCA = p->GetChildActivation())
    {
        if (SignalShape* pNew = pCA->GetSender())
        { replaceSelection(pNew); update(); }
        return;
    }
}

void SequenceDiagramCanvas::navigateRight()
{
    SequenceDiagramShape* p = singleSelectedShape();
    if (!p) return;
    if (LifeLineShape* pLL = p->GetLifeLine())
    {
        if (LifeLineShape* pNew = _pSD->GetNextLifeLineShape(pLL))
        { replaceSelection(pNew); update(); }
        return;
    }
    if (SignalShape* pSig = p->GetSignal())
    {
        if (ChildActivationShape* pNew = pSig->GetReceiver())
        { replaceSelection(pNew); update(); }
        return;
    }
    if (ChildActivationShape* pCA = p->GetChildActivation())
    {
        if (SignalShape* pNew = pCA->GetFirstReceiver())
        { replaceSelection(pNew); update(); }
        return;
    }
}

void SequenceDiagramCanvas::navigateUp()
{
    SequenceDiagramShape* p = singleSelectedShape();
    if (!p) return;
    if (SignalShape* pSig = p->GetSignal())
    {
        if (ChildActivationShape* pSender = pSig->GetSender())
            if (SignalShape* pNew = pSender->GetPrevReceiver(pSig))
            { replaceSelection(pNew); update(); }
        return;
    }
    if (ChildActivationShape* pCA = p->GetChildActivation())
    {
        if (ParentActivationShape* pParent = pCA->GetParentActivationShape())
            if (ChildActivationShape* pNew = pParent->GetPrevChildActivationShape(pCA))
            { replaceSelection(pNew); update(); }
        return;
    }
}

void SequenceDiagramCanvas::navigateDown()
{
    SequenceDiagramShape* p = singleSelectedShape();
    if (!p) return;
    if (SignalShape* pSig = p->GetSignal())
    {
        if (ChildActivationShape* pSender = pSig->GetSender())
            if (SignalShape* pNew = pSender->GetNextReceiver(pSig))
            { replaceSelection(pNew); update(); }
        return;
    }
    if (ChildActivationShape* pCA = p->GetChildActivation())
    {
        if (ParentActivationShape* pParent = pCA->GetParentActivationShape())
            if (ChildActivationShape* pNew = pParent->GetNextChildActivationShape(pCA))
            { replaceSelection(pNew); update(); }
        return;
    }
}

// ---------------------------------------------------------------------------
// Ctrl+arrows -- swap with neighbour. Mirrors OnSequencediagramMoveleft /
// Moveright / Moveup / Movedown.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::swapLifeLineLeft()
{
    LifeLineShape* pLL = singleSelectedShape() ?
        singleSelectedShape()->GetLifeLine() : nullptr;
    if (!pLL) return;
    LifeLineShape* pPrev = _pSD->GetPrevLifeLineShape(pLL);
    if (!pPrev) return;
    // Swap the two lifelines' x; report each move to the model (its SetRect sort
    // re-establishes the order). Math + follow live in RecalculateAfterEdit.
    const CbRect rect     = pLL->GetRect();
    const CbRect prevRect = pPrev->GetRect();
    _pSD->RecalculateAfterEdit(pLL,   CbSize(prevRect.left  - rect.left,  0));
    _pSD->RecalculateAfterEdit(pPrev, CbSize(rect.right - prevRect.right, 0));
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
}

void SequenceDiagramCanvas::swapLifeLineRight()
{
    LifeLineShape* pLL = singleSelectedShape() ?
        singleSelectedShape()->GetLifeLine() : nullptr;
    if (!pLL) return;
    LifeLineShape* pNext = _pSD->GetNextLifeLineShape(pLL);
    if (!pNext) return;
    // Swap the two lifelines' x; report each move to the model (its SetRect sort
    // re-establishes the order). Math + follow live in RecalculateAfterEdit.
    const CbRect rect     = pLL->GetRect();
    const CbRect nextRect = pNext->GetRect();
    _pSD->RecalculateAfterEdit(pNext, CbSize(rect.left  - nextRect.left,  0));
    _pSD->RecalculateAfterEdit(pLL,   CbSize(nextRect.right - rect.right, 0));
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
}

void SequenceDiagramCanvas::swapChildActivationUp()
{
    ChildActivationShape* pCA = singleSelectedShape() ?
        singleSelectedShape()->GetChildActivation() : nullptr;
    if (!pCA) return;
    ParentActivationShape* pParent = pCA->GetParentActivationShape();
    if (!pParent) return;
    ChildActivationShape* pPrev = pParent->GetPrevChildActivationShape(pCA);
    if (!pPrev) return;
    pCA->SaveState(1);
    pParent->MoveChildActivationShapeBefore(pCA, pPrev);
    if (ChildActivationShape* pParentCA = pParent->GetChildActivation())
    {
        pCA->GetSender()->SaveState(1);
        pParentCA->MoveReceiverBefore(pCA->GetSender(), pPrev->GetSender());
    }
    // Reorder is structural; the recompute records the resulting activation moves
    // and carries the attached note points (zero offset -- nothing to apply).
    _pSD->RecalculateAfterEdit(pCA);
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
}

void SequenceDiagramCanvas::swapChildActivationDown()
{
    ChildActivationShape* pCA = singleSelectedShape() ?
        singleSelectedShape()->GetChildActivation() : nullptr;
    if (!pCA) return;
    ParentActivationShape* pParent = pCA->GetParentActivationShape();
    if (!pParent) return;
    ChildActivationShape* pNext = pParent->GetNextChildActivationShape(pCA);
    if (!pNext) return;
    pCA->SaveState(1);
    pParent->MoveChildActivationShapeAfter(pCA, pNext);
    if (ChildActivationShape* pParentCA = pParent->GetChildActivation())
    {
        pCA->GetSender()->SaveState(1);
        pParentCA->MoveReceiverAfter(pCA->GetSender(), pNext->GetSender());
    }
    // Reorder is structural; the recompute records the moves + follows (see Up).
    _pSD->RecalculateAfterEdit(pCA);
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
}

// ---------------------------------------------------------------------------
// Checkable view toggles -- mirror OnSequenceDiagramAutowidth /
// OnShowmethodarguments / OnShowmethodscope. Enable/check predicates are the
// menu side (see contextMenuEvent); these just flip and refresh.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::toggleAutoWidth()
{
    ClassLifeLineShape* pCLL = singleSelectedShape() ?
        singleSelectedShape()->GetClassLifeLine() : nullptr;
    if (!pCLL) return;
    pCLL->SaveState(1);
    pCLL->SetAutoWidth(pCLL->GetAutoWidth() ? false : true);
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    // MFC only re-lays-out when turning auto-width ON (turning it off leaves
    // the box at its current width); mirror that, but always repaint our view.
    update();
}

void SequenceDiagramCanvas::toggleShowMethodArguments()
{
    ChildActivationShape* pCA = singleSelectedShape() ?
        singleSelectedShape()->GetChildActivation() : nullptr;
    if (!pCA) return;
    SignalShape* pSender = pCA->GetSender();
    pSender->SaveState(1);   // snapshot before mutation, else nothing to undo
    pSender->SetArguments(pSender->GetArguments() ? 0 : 1);
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void SequenceDiagramCanvas::toggleShowMethodScope()
{
    ChildActivationShape* pCA = singleSelectedShape() ?
        singleSelectedShape()->GetChildActivation() : nullptr;
    if (!pCA) return;
    SignalShape* pSender = pCA->GetSender();
    pSender->SaveState(1);   // snapshot before mutation, else nothing to undo
    pSender->SetScope(pSender->GetScope() ? 0 : 1);
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// ---------------------------------------------------------------------------
// Collapse Activations to Note -- faithful port of OnSDCollapseToNote. Requires
// 2+ selected child-activations all on the same lifeline (enforced by the menu
// enable predicate; re-checked here defensively). Reads each activation's
// signal/method name BEFORE deleting (the SignalShape dies with its receiver),
// parks an SDNote just right of the lifeline, then deletes the activations.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::collapseToNote()
{
    if (!_pSD || !_pViewModel)
        return;

    LifeLineShape* pCommonLL = nullptr;
    int topY = 0, bottomX = 0;
    bool haveTop = false;
    QVector<ChildActivationShape*> selected;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
        ChildActivationShape* pAct = pShape ? pShape->GetChildActivation() : nullptr;
        if (!pAct) return;                                  // mixed selection -> abort
        if (!pCommonLL) pCommonLL = pAct->GetLifeLineShape();
        else if (pCommonLL != pAct->GetLifeLineShape()) return;
        CbRect r = pAct->GetRect();
        if (!haveTop || r.top > topY) { topY = r.top; haveTop = true; }
        if (r.right > bottomX) bottomX = r.right;
        selected.append(pAct);
    }
    if (selected.size() < 2 || !pCommonLL)
        return;

    CbString text;
    for (ChildActivationShape* pAct : selected)
    {
        CbString item;
        if (SignalShape* pSig = pAct->GetSender()) item = pSig->GetName();
        else if (Method* pM = pAct->GetMethod())   item = pM->GetName();
        else                                       item = "(anonymous activation)";
        if (!text.IsEmpty()) text += NL;
        text += item;
    }

    _pSD->GetDataModelDoc()->MarkLastUndo();

    CbPoint notePoint(bottomX + 40, topY);
    Shape::Round(notePoint);
    SDNoteShape* pNote = new SDNoteShape(_pSD, notePoint);
    pNote->SetNote(text);

    for (ChildActivationShape* pAct : selected)
        pAct->Delete();     // cascades to the incoming SignalShapes

    _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// ---------------------------------------------------------------------------
// Change Line / Text colour -- mirror OnSequenceDiagramChangecolor /
// Changetextcolor. Seed the picker with the selection's common colour (black
// if they disagree), then apply the pick (with SaveState) to every selected
// shape that uses that colour. CbColorRef <-> QColor via the RGB/GetRValue macros.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::changeLineColor()
{
    if (!_pSD || !_pViewModel)
        return;

    CbColorRef seed = Cb_RGB(0, 0, 0);
    bool first = true;
    {
        SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            SequenceDiagramShape* p = iSel->GetSequenceDiagramShape();
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

    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* p = iSel->GetSequenceDiagramShape();
        if (p && p->UsesPenColor())
        {
            p->SaveState(1);
            p->SetPenColor(chosen);
        }
    }
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void SequenceDiagramCanvas::changeTextColor()
{
    if (!_pSD || !_pViewModel)
        return;

    CbColorRef seed = Cb_RGB(0, 0, 0);
    bool first = true;
    {
        SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            SequenceDiagramShape* p = iSel->GetSequenceDiagramShape();
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

    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* p = iSel->GetSequenceDiagramShape();
        if (p && p->UsesTextColor())
        {
            p->SaveState(1);
            p->SetTextColor(chosen);
        }
    }
    if (_pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

// ---------------------------------------------------------------------------
// Box-select (M2.5) -- mirrors MFC BoxSelectTrack: a shape is selected iff
// both its top-left AND bottom-right corners are inside the band (fully
// enclosed). The band is rendered via a QRubberBand widget child so we
// don't have to mix widget-coord overlays into the model-transform paint.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::beginBoxSelectIfReady(QPointF widgetPos)
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

void SequenceDiagramCanvas::finishBoxSelect(bool additive)
{
    if (!_pSD || !_pViewModel)
    {
        cancelBoxSelect();
        return;
    }
    // The widget-coord band -> model-coord rect via the inverse transform.
    // Take BOTH corners from the (already normalized) rubber-band geometry.
    // Using _boxSelectPress + bottomRight() only spans the rectangle when the
    // drag went top-left -> bottom-right; for the other three drag directions
    // the press is not the top-left corner, so a and b shared a coordinate (or
    // coincided) and the resulting rect was degenerate -- the band drew fine
    // but nothing was ever enclosed.
    const QRect band = _rubberBand ? _rubberBand->geometry()
                                   : QRect(_boxSelectPress.toPoint(),
                                           _boxSelectPress.toPoint());
    QPointF a = widgetToModel(QPointF(band.topLeft()));
    QPointF b = widgetToModel(QPointF(band.bottomRight()));
    const qreal mx1 = qMin(a.x(), b.x());
    const qreal mx2 = qMax(a.x(), b.x());
    const qreal my1 = qMin(a.y(), b.y());
    const qreal my2 = qMax(a.y(), b.y());

    if (!additive)
        _pViewModel->DeleteAllSelected();   // re-clear (press also cleared) for additive=false

    SequenceDiagram::SequenceDiagramShapeIterator iShape(_pSD);
    while (++iShape)
    {
        SequenceDiagramShape* pShape = iShape.Get();
        if (!pShape)
            continue;
        const CbRect r = pShape->GetRect();
        const qreal sx1 = qMin<qreal>(r.left, r.right);
        const qreal sx2 = qMax<qreal>(r.left, r.right);
        const qreal sy1 = qMin<qreal>(r.top,  r.bottom);
        const qreal sy2 = qMax<qreal>(r.top,  r.bottom);
        // Fully enclosed: both extents inside the band.
        if (sx1 >= mx1 && sx2 <= mx2 && sy1 >= my1 && sy2 <= my2)
        {
            if (!findSelection(pShape))
                (void)new SequenceDiagramViewModelSelection(_pViewModel, pShape);
        }
    }

    cancelBoxSelect();      // hide band, reset state
    update();
}

void SequenceDiagramCanvas::cancelBoxSelect()
{
    if (_rubberBand)
        _rubberBand->hide();
    _boxSelectPotential = false;
    _boxSelectActive    = false;
}

// ---------------------------------------------------------------------------
// Open-shape dispatch (M3) -- double-click and Enter both route here. The
// shape's virtual OnOpen() / OnEditAttributes() already wires to the right
// Qt_Show*Dialog with AfxGetMainWnd() as owner, so this canvas just hit-
// tests and calls.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::openShapeAt(QPointF widgetPt, bool ctrlHeld)
{
    if (!_pSD || !_pViewModel)
        return;
    const QPointF m = widgetToModel(widgetPt);
    const CbPoint modelPt(qRound(m.x()), qRound(m.y()));
    SequenceDiagramShape* pHit = hitTest(modelPt);
    if (!pHit)
        return;
    // Match MFC: force the double-clicked shape into the (sole) selection.
    if (!findSelection(pHit))
    {
        _pViewModel->DeleteAllSelected();
        (void)new SequenceDiagramViewModelSelection(_pViewModel, pHit);
        update();
    }
    if (ctrlHeld)
        pHit->OnEditAttributes(false);
    else
        pHit->OnOpen(false);
    if (_pSD && _pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    raiseToFront();
}

void SequenceDiagramCanvas::openSelected(bool ctrlHeld)
{
    SequenceDiagramShape* p = singleSelectedShape();
    if (!p)
        return;
    if (ctrlHeld)
        p->OnEditAttributes(false);
    else
        p->OnOpen(false);
    if (_pSD && _pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    raiseToFront();
}

// The dialogs opened above are owned by the MFC main window (their OnOpen
// uses AfxGetMainWnd), so when one closes activation returns to MFC and
// this Qt view -- also just an owned popup of MFC -- drops behind it.
// Pull our top-level window back to the front.
void SequenceDiagramCanvas::raiseToFront()
{
    if (QWidget* top = window())
    {
        top->raise();
        top->activateWindow();
    }
}

// Color Templates submenu table -- document-level default colour per shape
// type. Each entry mirrors one OnSequencediagramChange*Color handler: seed a
// picker with the DataModelDoc's current value, store the pick back. Driven by
// member-function pointers so all 11 share one helper (changeTemplateColor).
namespace {
struct TemplateColorEntry {
    const char* title;
    CbColorRef (DataModelDoc::*get)() const;
    void     (DataModelDoc::*set)(CbColorRef);
    bool separatorBefore;   // group divider (lifeline / activation / signal / note)
};
const TemplateColorEntry kTemplateColors[] = {
    { "Change LifeLine Line Color",           &DataModelDoc::GetLifeLinePenColor,           &DataModelDoc::SetLifeLinePenColor,           false },
    { "Change LifeLine Text Color",           &DataModelDoc::GetLifeLineTextColor,          &DataModelDoc::SetLifeLineTextColor,          false },
    { "Change Activation Line Color",         &DataModelDoc::GetActivationPenColor,         &DataModelDoc::SetActivationPenColor,         true  },
    { "Change No Code Activation Line Color", &DataModelDoc::GetActivationInitialPenColor,  &DataModelDoc::SetActivationInitialPenColor,  false },
    { "Change Method in User3 Activation Line Color",   &DataModelDoc::GetActivationUser3PenColor,    &DataModelDoc::SetActivationUser3PenColor,    false },
    { "Change No Method Activation Line Color", &DataModelDoc::GetActivationNoMethodPenColor, &DataModelDoc::SetActivationNoMethodPenColor, false },
    { "Change Message Line Color",            &DataModelDoc::GetSignalPenColor,             &DataModelDoc::SetSignalPenColor,             true  },
    { "Change No Method Message Line Color",  &DataModelDoc::GetSignalNoMethodPenColor,     &DataModelDoc::SetSignalNoMethodPenColor,     false },
    { "Change Message Text Color",            &DataModelDoc::GetSignalTextColor,            &DataModelDoc::SetSignalTextColor,            false },
    { "Change Note Line Color",               &DataModelDoc::GetSDNoteShapePenColor,        &DataModelDoc::SetSDNoteShapePenColor,        true  },
    { "Change Note Text Color",               &DataModelDoc::GetSDNoteShapeTextColor,       &DataModelDoc::SetSDNoteShapeTextColor,       false },
};
const int kTemplateColorCount = (int)(sizeof(kTemplateColors) / sizeof(kTemplateColors[0]));
} // namespace

void SequenceDiagramCanvas::changeTemplateColor(int entryIndex)
{
    if (!_pSD || entryIndex < 0 || entryIndex >= kTemplateColorCount)
        return;
    DataModelDoc* doc = _pSD->GetDataModelDoc();
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

// Common tail after an Add-submenu model command (BaseClass::OnAddX /
// Method::OnAddArgument). Those show their own (Qt) dialog; once it returns we
// mark one undo step, refresh the views, and pull this window back in front
// (the dialog is owned by the MFC main window, so we drop behind it on close).
void SequenceDiagramCanvas::afterModelAdd()
{
    if (_pSD && _pSD->GetDataModelDoc())
        _pSD->GetDataModelDoc()->MarkLastUndo();
    raiseToFront();
    update();
}

// Resolve the first and last selected shapes (insertion order) -- only when
// exactly two are selected. Mirrors the MFC GetFirstSelected/GetLastSelected
// pair used by Add Message's two-selection case.
bool SequenceDiagramCanvas::twoSelected(SequenceDiagramShape*& pFirst,
                                        SequenceDiagramShape*& pLast) const
{
    pFirst = nullptr;
    pLast  = nullptr;
    if (!_pViewModel || _pViewModel->GetSelectedCount() != 2)
        return false;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    if (++iSel) pFirst = iSel->GetSequenceDiagramShape();
    if (++iSel) pLast  = iSel->GetSequenceDiagramShape();
    return pFirst && pLast;
}

// Mirror OnUpdateAddMessage: a single lifeline or activation, or two shapes
// where the first is an activation and the second is either a lifeline or an
// activation that can legally receive a call from the first (no cycle).
bool SequenceDiagramCanvas::canAddMessage() const
{
    SequenceDiagramShape* pSingle = singleSelectedShape();
    if (pSingle)
        return pSingle->GetLifeLine() || pSingle->GetChildActivation();

    SequenceDiagramShape* pFirst = nullptr;
    SequenceDiagramShape* pLast  = nullptr;
    if (!twoSelected(pFirst, pLast))
        return false;
    ChildActivationShape* pCA1 = pFirst->GetChildActivation();
    if (!pCA1)
        return false;
    if (pLast->GetLifeLine())
        return true;
    ChildActivationShape* pCA2 = pLast->GetChildActivation();
    return pCA2
 && pCA2->GetParentActivationShape() != pCA1
 && !pCA1->IsDirectOrIndirectChild(pCA2);
}

// Port of SequenceDiagramView::OnAddMessage. Selection-based, no mouse track.
// The SignalDialog is owned by this Qt window (winId); on cancel we RollBack to
// the undo marker taken at the top of the matching branch.
void SequenceDiagramCanvas::addMessage()
{
    if (!_pSD || !_pViewModel)
        return;
    DataModelDoc* doc = _pSD->GetDataModelDoc();
    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());

    SequenceDiagramShape* pSingle = singleSelectedShape();
    if (pSingle)
    {
        UndoBase* pLastUndo = doc->MarkLastUndo();
        LifeLineShape* pLifeLineShape1 = pSingle->GetLifeLine();
        ChildActivationShape* pCA1 = pSingle->GetChildActivation();
        if (pLifeLineShape1)
        {
            // Root call on the lifeline -- no dialog (matches MFC).
            ChildActivationShape* pNew = new ChildActivationShape(pLifeLineShape1);
            _pViewModel->DeleteAllSelected();
            (void)new SequenceDiagramViewModelSelection(_pViewModel, pNew);
        }
        else if (pCA1)
        {
            // Nested call from the selected activation; spec it in the dialog.
            ChildActivationShape* pNew =
                new ChildActivationShape(pCA1->GetLifeLineShape(), pCA1);
            if (Qt_ShowSignalDialog(pNew->GetSender(), ownerHwnd))
            {
                _pViewModel->DeleteAllSelected();
                (void)new SequenceDiagramViewModelSelection(_pViewModel,
                                                            pNew->GetSender());
            }
            else
            {
                doc->RollBack(pLastUndo);
            }
        }
    }
    else if (_pViewModel->GetSelectedCount() == 2)
    {
        UndoBase* pLastUndo = doc->MarkLastUndo();
        SequenceDiagramShape* pFirst = nullptr;
        SequenceDiagramShape* pLast  = nullptr;
        twoSelected(pFirst, pLast);
        ChildActivationShape* pCA1 = pFirst ? pFirst->GetChildActivation() : nullptr;
        ChildActivationShape* pCA2 = pLast  ? pLast->GetChildActivation()  : nullptr;
        LifeLineShape*        pLL2 = pLast  ? pLast->GetLifeLine()         : nullptr;
        if (pCA1)
        {
            if (pLL2)
            {
                // Call from pCA1 to a new activation on the target lifeline.
                ChildActivationShape* pNew = new ChildActivationShape(pLL2, pCA1);
                if (Qt_ShowSignalDialog(pNew->GetSender(), ownerHwnd))
                {
                    _pViewModel->DeleteAllSelected();
                    (void)new SequenceDiagramViewModelSelection(_pViewModel,
                                                                pNew->GetSender());
                }
                else
                {
                    doc->RollBack(pLastUndo);
                }
            }
            else if (pCA2
 && pCA2->GetParentActivationShape() != pCA1
 && !pCA1->IsDirectOrIndirectChild(pCA2))
            {
                // Reparent pCA2 under pCA1 (a call from pCA1 that triggers the
                // existing activation pCA2), carrying or creating its signal.
                pCA2->SaveState(1);
                pCA1->MoveChildActivationShapeLast(pCA2);
                SignalShape* pSignalShape = pCA2->GetSender();
                if (pSignalShape)
                {
                    pSignalShape->SaveState(1);
                    pCA1->MoveReceiverLast(pSignalShape);
                    _pViewModel->DeleteAllSelected();
                    (void)new SequenceDiagramViewModelSelection(_pViewModel,
                                                                pSignalShape);
                }
                else
                {
                    pSignalShape = new SignalShape(pCA2, pCA1);
                    if (Qt_ShowSignalDialog(pSignalShape, ownerHwnd))
                    {
                        _pViewModel->DeleteAllSelected();
                        (void)new SequenceDiagramViewModelSelection(_pViewModel,
                                                                    pSignalShape);
                    }
                    else
                    {
                        doc->RollBack(pLastUndo);
                    }
                }
            }
        }
    }

    doc->MarkLastUndo();
    raiseToFront();
    update();
}

// Port of SequenceDiagramView::OnEditAddclass. GetParent()->OnAddClass shows
// the add-class dialog (and adds a Class to the model on OK); if a new class
// actually appeared, auto-place a ClassLifeLineShape just right of the last
// lifeline (X = lastRight + 20, or 80 if the diagram is empty), select it, and
// open its spec -- no mouse tracking, unlike Note/Lifeline.
void SequenceDiagramCanvas::addClass()
{
    if (!_pSD || !_pViewModel)
        return;
    DataModelDoc* doc        = _pSD->GetDataModelDoc();
    DataModel*    pDataModel  = doc->GetDataModel();
    Class*        pLastClass  = pDataModel->GetLastClass();

    if (_pSD->GetParent()->OnAddClass(false))
    {
        Class* pClass = pDataModel->GetLastClass();
        if (pClass != pLastClass)   // a class was actually added (not cancelled)
        {
            int lastRight = 80;
            if (LifeLineShape* pLast = _pSD->GetLastLifeLineShape())
                lastRight = pLast->GetRect().right;
            CbPoint point(lastRight + 20, 0);
            Shape::Round(point);
            ClassLifeLineShape* pCLL =
                new ClassLifeLineShape(_pSD, pClass, point);
            _pViewModel->DeleteAllSelected();
            (void)new SequenceDiagramViewModelSelection(_pViewModel, pCLL);
            pCLL->OnOpen();
        }
    }

    doc->MarkLastUndo();
    raiseToFront();
    update();
}

// ---------------------------------------------------------------------------
// Add Note / Lifeline -- interactive placement (port of OnAddNote /
// OnAddLifeline). MFC's SDNoteShape::TrackAdd / LifeLineShape::TrackAdd run a
// modal ::GetMessage loop that XOR-draws a rubber-band and places the shape on
// the next click. That modal-capture style fights the Qt event loop, so the
// port models it as a non-modal "placement mode": the menu arms _placementKind,
// the next left-click drops the shape, mouse-move paints a ghost rect, and Esc
// / right-click cancels. Behaviour matches MFC: Note places then opens its spec
// dialog (cancel -> rollback); Lifeline shows the actor/class picker on the
// click, then places at the click point (X only -- Y is pinned to the
// class-lifeline band, mirroring TrackAdd's forced point.y).
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::beginPlacement(PlacementKind kind)
{
    _placementKind   = kind;
    _placementHasPos = false;
    setCursor(Qt::CrossCursor);
    update();
}

// Public toolbar wrappers (PlacementKind is class-internal; addClass private).
void SequenceDiagramCanvas::armAddLifeLinePlacement() { beginPlacement(PlacementKind::LifeLine); }
void SequenceDiagramCanvas::armAddNotePlacement()     { beginPlacement(PlacementKind::Note); }
void SequenceDiagramCanvas::addClassFromToolBar()     { addClass(); }

QString SequenceDiagramCanvas::diagramName() const
{
    return _pSD ? toQ(_pSD->GetName()) : QString();
}

bool SequenceDiagramCanvas::exportSvg(const QString& path, bool tight, int margin)
{
#ifdef CB_HAVE_SVG
    if (!_pSD)
        return false;

    // Page mode: the fixed printable page, same as computeFit()'s A4 default.
    // Tight mode: the actual shape extents + a margin, so a small diagram
    // doesn't export as mostly whitespace. GetBoundingRect() calls
    // RecalculateDiagram() internally, so lifeline/activation layout is
    // current even if nothing has painted yet.
    CbRect view;
    if (tight)
    {
        view = _pSD->GetBoundingRect();
        view.InflateRect(margin, margin);
    }
    else
    {
        const FitInfo f = computeFit();
        view = CbRect(0, -f.pageH, f.pageW, 0);
    }

    QSvgGenerator gen;
    gen.setFileName(path);
    gen.setSize(QSize(qCeil((qreal)view.Width()), qCeil((qreal)view.Height())));
    gen.setViewBox(QRectF(0, 0, view.Width(), view.Height()));
    gen.setTitle(diagramName());
    gen.setDescription(QStringLiteral("ClassBuilder sequence diagram"));

    QPainter qp(&gen);
    qp.setRenderHint(QPainter::Antialiasing, true);
    qp.setRenderHint(QPainter::TextAntialiasing, true);
    // Model Y-up; the scale(1,-1) flip maps model-space onto the SVG viewBox
    // -- the same flip paintEvent applies (minus fit-to-window scale + user
    // zoom/pan). The translate re-anchors view's top-left onto the viewBox
    // origin; it's a no-op in page mode (view.left/bottom == 0).
    qp.scale(1.0, -1.0);
    qp.translate(-view.left, -view.bottom);
    qp.fillRect(QRectF(view.left, view.top, view.Width(), view.Height()), Qt::white);

    CbPainter_QPainter painter(&qp);
    painter.SetScreen(false);   // suppress selection highlights, like print/EMF
    _pSD->Draw(painter, _pViewModel);
    return qp.end();
#else
    Q_UNUSED(path);
    Q_UNUSED(tight);
    Q_UNUSED(margin);
    return false;
#endif
}

void SequenceDiagramCanvas::cancelPlacement()
{
    if (_placementKind == PlacementKind::None)
        return;
    _placementKind   = PlacementKind::None;
    _placementHasPos = false;
    unsetCursor();
    update();
}

void SequenceDiagramCanvas::finishPlacementAt(QPointF widgetPt)
{
    const PlacementKind kind = _placementKind;
    // Leave placement mode BEFORE the dialogs spin their own event loop, so a
    // stray click can't re-enter placement underneath them.
    _placementKind   = PlacementKind::None;
    _placementHasPos = false;
    unsetCursor();

    if (!_pSD || !_pViewModel || kind == PlacementKind::None)
    {
        update();
        return;
    }
    DataModelDoc* doc = _pSD->GetDataModelDoc();
    const QPointF m   = widgetToModel(widgetPt);

    if (kind == PlacementKind::Note)
    {
        UndoBase* pLastUndo = doc->MarkLastUndo();
        CbPoint point(qRound(m.x()), qRound(m.y()));
        Shape::Round(point);
        SDNoteShape* pNote = new SDNoteShape(_pSD, point);
        // The ctor builds a 100-tall rect; Draw only shrinks it to the no-text
        // height (2*(GetFontHeight()+2) = 60) on the first paint, showing as a
        // 100->60 jump behind the dialog. Settle it NOW so it matches the ghost
        // (keep bottom = the click anchor + width, raise top).
        {
            CbRect r = pNote->GetRect();
            r.top = r.bottom - 2 * (pNote->GetFontHeight() + 2);
            pNote->SetRect(r);
        }
        if (pNote->OnOpen())
        {
            replaceSelection(pNote);
        }
        else
        {
            doc->RollBack(pLastUndo);
        }
    }
    else // PlacementKind::LifeLine
    {
        void* ownerHwnd = reinterpret_cast<void*>(window()->winId());
        Actor*     pDlgActor          = 0;
        BaseClass* pDlgBaseClass      = 0;
        CbString   dlgTemplate;
        bool       dlgShowActivations = true;
        CbString   dlgName;
        CbString   dlgNote;
        UndoBase* pLastUndo = doc->MarkLastUndo();
        if (Qt_ShowLifeLineDialogNew(doc, ownerHwnd, pDlgActor, pDlgBaseClass,
                                     dlgTemplate, dlgShowActivations,
                                     dlgName, dlgNote))
        {
            CbPoint point(qRound(m.x()), qRound(m.y()));
            // Lifelines anchor to the class-lifeline band -- only X follows the
            // cursor (mirrors LifeLineShape::TrackAdd's forced point.y).
            point.y = -SequenceDiagram::GetClassLifeLineOffset()
                      + SequenceDiagram::GetClassLifeLineHeight();
            Shape::Round(point);

            LifeLineShape* pLL = 0;
            if (pDlgActor)
                pLL = new ActorLifeLineShape(_pSD, pDlgActor, point);
            if (pDlgBaseClass)
            {
                pLL = new ClassLifeLineShape(_pSD, pDlgBaseClass, point);
                ((ClassLifeLineShape*)pLL)->SetTemplate(dlgTemplate);
            }
            if (pLL)
            {
                pLL->SetShowActivations(dlgShowActivations);
                if (!dlgName.IsEmpty()) pLL->SetName(dlgName);
                if (!dlgNote.IsEmpty()) pLL->SetNote(dlgNote);
                replaceSelection(pLL);
            }
            else
            {
                doc->RollBack(pLastUndo);
            }
        }
        else
        {
            doc->RollBack(pLastUndo);
        }
    }

    doc->MarkLastUndo();
    raiseToFront();
    update();
}

// Placement ghost -- a cosmetic dashed rect of the shape's base footprint
// tracking the cursor, painted inside the model transform (like
// paintDragGhost). Base sizes mirror the MFC TrackAdd baseRects (model units,
// Y-up: the rect extends DOWNWARD from the anchor).
void SequenceDiagramCanvas::paintPlacementGhost(QPainter& qp)
{
    if (_placementKind == PlacementKind::None || !_placementHasPos)
        return;

    // Note: width is the constructor's 400 (Draw never changes it), but the
    // height shown is the note's *settled* height, not the 100-tall
    // construction rect. SDNoteShape::Draw auto-fits to content with a minimum
    // of size*2 where size = GetFontHeight()+2 (default font 28 -> 30), so an
    // empty / one-line note lands at 60. (MFC's rubber-band shows the 100 here
    // and is misleadingly tall -- this ghost matches what actually gets placed.)
    qreal   w = 400, h = 2 * (28 + 2);
    QPointF anchor = _placementModelPt;
    if (_placementKind == PlacementKind::LifeLine)
    {
        w = 250;
        h = SequenceDiagram::GetClassLifeLineHeight();
        anchor.setY(-SequenceDiagram::GetClassLifeLineOffset()
                    + SequenceDiagram::GetClassLifeLineHeight());
    }

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(QRectF(anchor.x(), anchor.y() - h, w, h));
    qp.restore();
}

// ---------------------------------------------------------------------------
// Undo / Redo (M7) -- Ctrl+Z / Ctrl+Y. Routes to DataModelDoc::Undo/Redo (the
// same calls as CClassBuilderDoc::OnEditUndo/OnEditRedo). canUndo/canRedo mirror
// OnUpdateEditUndo/Redo's guard -- never undo a diagram-creation (UndoNew) /
// redo a diagram-deletion (RedoDelete) while that diagram has open views -- and
// additionally never undo THIS view's own SD creation, so _pSD stays valid
// across the call. Undo() internally Lock/UnLock+UpdateAllViews; we also call
// UpdateSequenceDiagramViews so the Qt canvas repaints.
// ---------------------------------------------------------------------------
// Undo/Redo enablement is ONE rule for every view -- "is there a step on the
// stack?" -- delegated to the doc (CClassBuilderDoc::CanUndo/CanRedo) so the
// tree, CD and SD buttons behave identically. No per-entry walk/deref here.
bool SequenceDiagramCanvas::canUndo() const
{
    DataModelDoc* doc = _pSD ? _pSD->GetDataModelDoc() : nullptr;
    return doc && doc->CanUndo();
}

bool SequenceDiagramCanvas::canRedo() const
{
    DataModelDoc* doc = _pSD ? _pSD->GetDataModelDoc() : nullptr;
    return doc && doc->CanRedo();
}

void SequenceDiagramCanvas::undo()
{
    if (!canUndo()) return;
    // DataModelDoc::Undo refreshes every view of the model by construction
    // (the old per-canvas refresh here compensated for entry points that
    // didn't -- gone with that gap).
    _pSD->GetDataModelDoc()->Undo();
}

void SequenceDiagramCanvas::redo()
{
    if (!canRedo()) return;
    _pSD->GetDataModelDoc()->Redo();   // see undo()
}

// ---------------------------------------------------------------------------
// Context menu (M4) -- right-click. Shape-scoped actions (Open / Delete) plus
// the diagram-wide layout commands, each reusing the model calls the MFC view
// uses (SequenceDiagramView::OnOpen / OnDelete / OnOptimizeplacement / ...).
// Right-clicking a shape that isn't already selected makes it the sole
// selection first, matching the MFC view's behaviour.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::contextMenuEvent(QContextMenuEvent* e)
{
    if (!_pSD || !_pViewModel)
        return;

    // A right-click while arming an Add Note / Lifeline cancels the placement
    // (mirrors the MFC TrackAdd WM_RBUTTONDOWN bail) instead of popping a menu.
    if (placementActive())
    {
        cancelPlacement();
        e->accept();
        return;
    }

    const QPointF m = widgetToModel(QPointF(e->pos()));
    const CbPoint modelPt(qRound(m.x()), qRound(m.y()));
    if (SequenceDiagramShape* pHit = hitTest(modelPt))
    {
        if (!findSelection(pHit))
        {
            replaceSelection(pHit);
            update();
        }
    }

    SequenceDiagramShape* pSingle = singleSelectedShape();
    const bool anySel = (_pViewModel->GetSelectedCount() > 0);

    // Mirror SequenceDiagramView::OnUpdateOpen: the entry names what it opens,
    // and is offered only when the shape consents (OnOpen(true)).
    QString openText = tr("&Open");
    bool openEnabled = false;
    if (pSingle && pSingle->OnOpen(true))
    {
        openEnabled = true;
        if      (pSingle->GetSignal())          openText = tr("&Open Message specification");
        else if (pSingle->GetChildActivation()) openText = tr("&Open Code Editor");
        else if (pSingle->GetLifeLine())        openText = tr("&Open LifeLine Specification");
        else if (pSingle->GetNoteShape())       openText = tr("&Open Note Specification");
    }

    // Mirror SequenceDiagramView::OnUpdateEditAttributes: dynamic label, only
    // when the shape consents (OnEditAttributes(true)).
    QString editText = tr("&Edit Attributes");
    bool editEnabled = false;
    if (pSingle && pSingle->OnEditAttributes(true))
    {
        editEnabled = true;
        if      (pSingle->GetChildActivation()) editText = tr("&Edit Method Attributes");
        else if (pSingle->GetActorLifeLine())   editText = tr("&Edit Actor Attributes");
        else if (pSingle->GetClassLifeLine())   editText = tr("&Edit Class Attributes");
    }

    // Mirror SequenceDiagramView::OnUpdateSequencediagramMove{up,down,left,right}:
    // Up/Down reorder a child-activation among its siblings; Left/Right reorder
    // a lifeline among its neighbours. Each is offered only when a neighbour
    // exists in that direction.
    bool canUp = false, canDown = false, canLeft = false, canRight = false;
    if (pSingle)
    {
        if (ChildActivationShape* pCA = pSingle->GetChildActivation())
        {
            if (ParentActivationShape* pParent = pCA->GetParentActivationShape())
            {
                canUp   = pParent->GetPrevChildActivationShape(pCA) != NULL;
                canDown = pParent->GetNextChildActivationShape(pCA) != NULL;
            }
        }
        if (LifeLineShape* pLL = pSingle->GetLifeLine())
        {
            canLeft  = _pSD->GetPrevLifeLineShape(pLL) != NULL;
            canRight = _pSD->GetNextLifeLineShape(pLL) != NULL;
        }
    }

    // View toggles -- mirror OnUpdateSequenceDiagramAutowidth /
    // OnUpdateShowmethodarguments / OnUpdateShowmethodscope (enable + check).
    bool awEnable = false, awCheck = false;
    bool argEnable = false, argCheck = false;
    bool scpEnable = false, scpCheck = false;
    if (pSingle)
    {
        if (ClassLifeLineShape* pCLL = pSingle->GetClassLifeLine())
        {
            awEnable = true;
            awCheck  = pCLL->GetAutoWidth() != false;
        }
        if (ChildActivationShape* pCA = pSingle->GetChildActivation())
        {
            if (pCA->GetMethod())   // only methods carry args/scope flags
            {
                argEnable = scpEnable = true;
                argCheck = pCA->GetSender()->GetArguments() != false;
                scpCheck = pCA->GetSender()->GetScope() != false;
            }
        }
    }

    // Collapse-to-Note enable -- mirror OnUpdateSDCollapseToNote: 2+ selected
    // child-activations all on the same lifeline.
    bool canCollapse = false;
    {
        int count = 0;
        LifeLineShape* pLL = nullptr;
        bool bad = false;
        SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
            ChildActivationShape* pAct = pShape ? pShape->GetChildActivation() : nullptr;
            if (!pAct) { bad = true; break; }
            if (!pLL) pLL = pAct->GetLifeLineShape();
            else if (pLL != pAct->GetLifeLineShape()) { bad = true; break; }
            ++count;
        }
        canCollapse = (!bad && count >= 2);
    }

    // Add-submenu scope: the single selected class lifeline's class (for the
    // class-scoped adds) and the single selected activation's method (for Add
    // Argument). Mirrors the OnUpdateEditAdd* handlers' shape accessors.
    BaseClass* pAddBase = nullptr;
    Method*    pAddArgMethod = nullptr;
    if (pSingle)
    {
        if (ClassLifeLineShape* pCLL = pSingle->GetClassLifeLine())
            pAddBase = pCLL->GetBaseClass();
        if (ChildActivationShape* pCA = pSingle->GetChildActivation())
            pAddArgMethod = pCA->GetMethod();
    }

    // Colour commands enable -- any selected shape that uses pen / text colour
    // (mirror OnUpdateSequenceDiagramChangecolor / Changetextcolor).
    bool anyPenColor = false, anyTextColor = false;
    {
        SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
        while (++iSel)
        {
            SequenceDiagramShape* p = iSel->GetSequenceDiagramShape();
            if (!p) continue;
            if (p->UsesPenColor())  anyPenColor  = true;
            if (p->UsesTextColor()) anyTextColor = true;
        }
    }

    QMenu menu(this);
    Qt_ApplyCompactMenuStyle(&menu);   // consistent with the tree/editor menus
    QAction* aOpen   = menu.addAction(openText);
    aOpen->setEnabled(openEnabled);
    QAction* aEdit   = menu.addAction(editText);
    aEdit->setEnabled(editEnabled);
    QAction* aDelete = menu.addAction(tr("&Delete"));
    aDelete->setEnabled(anySel);
    menu.addSeparator();
    // Add submenu -- the dialog-based, class/method-scoped items (each routes
    // to the model's OnAddX), Message, the interactive-placement items Lifeline
    // / Note (arm placement mode -- see finishPlacementAt), and Class
    // (add-class dialog + auto-placed lifeline -- see addClass). Complete port
    // of the MFC SD Add menu.
    QMenu* addMenu = menu.addMenu(tr("&Add"));
    QAction* aAddMessage = addMenu->addAction(tr("Me&ssage"));
    aAddMessage->setEnabled(canAddMessage());
    // Lifeline / Note arm interactive placement (always available -- they only
    // need a diagram). MFC OnAddLifeline / OnAddNote have no update handler.
    QAction* aAddLifeline = addMenu->addAction(tr("&Lifeline"));
    QAction* aAddNote     = addMenu->addAction(tr("&Note"));
    QAction* aAddClass    = addMenu->addAction(tr("&Class"));
    aAddClass->setEnabled(_pSD->GetParent()->OnAddClass(true) != 0);
    addMenu->addSeparator();
    QAction* aAddMember = addMenu->addAction(tr("&Member"));
    aAddMember->setEnabled(pAddBase && pAddBase->OnAddMember(true));
    QAction* aAddMethod = addMenu->addAction(tr("Met&hod"));
    aAddMethod->setEnabled(pAddBase && pAddBase->OnAddMethod(true));
    QAction* aAddConstructor = addMenu->addAction(tr("Co&nstructor"));
    aAddConstructor->setEnabled(pAddBase && pAddBase->OnAddConstructor(true));
    QAction* aAddArgument = addMenu->addAction(tr("&Argument"));
    aAddArgument->setEnabled(pAddArgMethod && pAddArgMethod->OnAddArgument(true));
    addMenu->addSeparator();
    QAction* aAddVirtuals = addMenu->addAction(tr("&Virtual Methods"));
    aAddVirtuals->setEnabled(pAddBase && pAddBase->OnAddVirtuals(true));
    QAction* aAddIsClass = addMenu->addAction(tr("I&sClass Methods"));
    aAddIsClass->setEnabled(pAddBase && pAddBase->OnAddIsClassMethods(true));

    // Accelerator hints, right-aligned in the menu -- the SAME keys as the tree/CD for
    // the shared class-model adds (consistency), plus the SD-specific Lifeline / Message.
    // Firing is the keyPressEvent Ctrl+Shift block (canvas-scoped); these transient menu
    // actions only register while the menu is open. Message = K (no mnemonic free --
    // M/S/A are tree-reserved; not W, which sits next to the OS-grabbed Ctrl+Shift+X).
    aAddMessage    ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+K")));
    aAddClass      ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));
    aAddLifeline   ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+L")));
    aAddNote       ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+N")));
    aAddMember     ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+B")));
    aAddMethod   ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+M")));
    aAddConstructor->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+U")));
    aAddArgument   ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+A")));
    aAddVirtuals   ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+V")));
    aAddIsClass    ->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+S")));
    menu.addSeparator();
    // Move group -- routes to the same swap* helpers as the Ctrl+Arrow keys.
    QAction* aMoveUp    = menu.addAction(tr("Move &Up"));
    aMoveUp->setEnabled(canUp);
    QAction* aMoveDown  = menu.addAction(tr("Move &Down"));
    aMoveDown->setEnabled(canDown);
    QAction* aMoveLeft  = menu.addAction(tr("Move &Left"));
    aMoveLeft->setEnabled(canLeft);
    QAction* aMoveRight = menu.addAction(tr("Move &Right"));
    aMoveRight->setEnabled(canRight);
    menu.addSeparator();
    // Checkable view toggles.
    QAction* aAutoWidth = menu.addAction(tr("Auto &Width"));
    aAutoWidth->setEnabled(awEnable);
    aAutoWidth->setCheckable(true);
    aAutoWidth->setChecked(awCheck);
    QAction* aShowArgs = menu.addAction(tr("Show Method &Arguments"));
    aShowArgs->setEnabled(argEnable);
    aShowArgs->setCheckable(true);
    aShowArgs->setChecked(argCheck);
    QAction* aShowScope = menu.addAction(tr("Show Method Sc&ope"));
    aShowScope->setEnabled(scpEnable);
    aShowScope->setCheckable(true);
    aShowScope->setChecked(scpCheck);
    menu.addSeparator();
    QAction* aOptimize = menu.addAction(tr("Optimize &Placement"));
    QAction* aSpace    = menu.addAction(tr("&Space Lifelines"));
    QAction* aReset    = menu.addAction(tr("&Reset Activation Offsets"));
    menu.addSeparator();
    QAction* aCollapse = menu.addAction(tr("Collapse Activations to &Note"));
    aCollapse->setEnabled(canCollapse);
    menu.addSeparator();
    QAction* aLineColor = menu.addAction(tr("Change &Line Color..."));
    aLineColor->setEnabled(anyPenColor);
    QAction* aTextColor = menu.addAction(tr("Change &Text Color..."));
    aTextColor->setEnabled(anyTextColor);

    // Color Templates submenu -- document-level defaults, always available.
    QMenu* templatesMenu = menu.addMenu(tr("Color Templates"));
    QVector<QAction*> templateActions;
    for (int i = 0; i < kTemplateColorCount; ++i)
    {
        if (kTemplateColors[i].separatorBefore)
            templatesMenu->addSeparator();
        templateActions.append(templatesMenu->addAction(tr(kTemplateColors[i].title)));
    }

    QAction* chosen = menu.exec(e->globalPos());
    e->accept();
    if (!chosen || !_pSD || !_pViewModel)
        return;

    if (chosen == aOpen)
    {
        openSelected(/*ctrlHeld=*/false);
    }
    else if (chosen == aEdit)
    {
        openSelected(/*ctrlHeld=*/true);   // ctrl path routes to OnEditAttributes
    }
    else if (chosen == aDelete)
    {
        deleteSelected();
    }
    else if (chosen == aMoveUp)    { swapChildActivationUp();   update(); }
    else if (chosen == aMoveDown)  { swapChildActivationDown(); update(); }
    else if (chosen == aMoveLeft)  { swapLifeLineLeft();        update(); }
    else if (chosen == aMoveRight) { swapLifeLineRight();       update(); }
    else if (chosen == aAddMessage)     { addMessage(); }
    else if (chosen == aAddLifeline)    { beginPlacement(PlacementKind::LifeLine); }
    else if (chosen == aAddNote)        { beginPlacement(PlacementKind::Note); }
    else if (chosen == aAddClass)       { addClass(); }
    else if (chosen == aAddMember)      { pAddBase->OnAddMember(false);        afterModelAdd(); }
    else if (chosen == aAddMethod)    { pAddBase->OnAddMethod(false);        afterModelAdd(); }
    else if (chosen == aAddConstructor) { pAddBase->OnAddConstructor(false);   afterModelAdd(); }
    else if (chosen == aAddArgument)    { pAddArgMethod->OnAddArgument(false); afterModelAdd(); }
    else if (chosen == aAddVirtuals)    { pAddBase->OnAddVirtuals(false);      afterModelAdd(); }
    else if (chosen == aAddIsClass)     { pAddBase->OnAddIsClassMethods(false);afterModelAdd(); }
    else if (chosen == aAutoWidth) { toggleAutoWidth(); }
    else if (chosen == aShowArgs)  { toggleShowMethodArguments(); }
    else if (chosen == aShowScope) { toggleShowMethodScope(); }
    else if (chosen == aCollapse)  { collapseToNote(); }
    else if (chosen == aLineColor) { changeLineColor(); }
    else if (chosen == aTextColor) { changeTextColor(); }
    else if (templateActions.contains(chosen)) { changeTemplateColor(templateActions.indexOf(chosen)); }
    else    // one of the diagram-wide layout commands
    {
        if      (chosen == aOptimize) _pSD->OptimizePlacement();
        else if (chosen == aSpace)    _pSD->SpaceLifeLines();
        else if (chosen == aReset)    _pSD->ResetActivationOffsets();
        if (_pSD->GetDataModelDoc())
            _pSD->GetDataModelDoc()->MarkLastUndo();
        update();
    }
}

// What kinds can be drag-moved in the M2.3 first cut. Lifelines (plain,
// actor, class) cover ~80% per the M2 plan. Signal sub-parts, activations,
// and SDNote points / resize land in M2.9 once the headless text-measure
// path is in.
bool SequenceDiagramCanvas::isDraggableKind(SequenceDiagramShape* p)
{
    return p && p->IsLifeLineShape();
}

// Transition: potential -> active. Called from mouseMoveEvent once the
// pointer has moved past a small widget-pixel threshold so a click + tiny
// jitter doesn't count as a drag.
void SequenceDiagramCanvas::beginDragIfReady(QPointF widgetPos)
{
    if (!_dragPotential || _dragActive)
        return;
    const QPointF d = widgetPos - _pressWidgetPos;
    if (d.manhattanLength() < 4)
        return;
    _dragActive = true;
    // Match the hover cursor for the dragged kind: a class lifeline (resizable)
    // moves with SizeAll to distinguish move-from-resize; actor / plain lifelines
    // keep SizeHor. (All draggable kinds are lifelines for now; revisit when the
    // M2.9 batch brings signals / notes and free-/vertical-move kinds.)
    bool draggingClassLL = false;
    for (const DragItem& it : _dragItems)
        if (it.shape && it.shape->GetClassLifeLine()) { draggingClassLL = true; break; }
    setCursor(draggingClassLL ? Qt::SizeAllCursor : Qt::SizeHorCursor);
}

// Commit: the model side gets exactly one SetRect per dragged shape, which
// triggers SaveState + UpdateSequenceDiagramViews + SortLifeLineShape inside
// LifeLineShape::SetRect. ESC cancellation goes through cancelDrag() and
// never reaches here.
void SequenceDiagramCanvas::commitDrag()
{
    if (!_dragActive || _dragItems.isEmpty())
        return;
    const QPointF rawDelta = _dragCurrentModelPt - _pressModelPt;
    // This drag is always lifeline-led (it only starts on a lifeline press), so
    // the whole gesture is horizontal: if any dragged shape is x-only (a
    // lifeline), a selected note riding along is x-constrained too -- it stays
    // put relative to the lifelines instead of drifting in Y on a downward move.
    bool groupHorizOnly = false;
    for (const DragItem& it : _dragItems)
        if (it.shape && it.shape->IsLifeLineShape()) { groupHorizOnly = true; break; }
    bool moved = false;
    for (const DragItem& it : _dragItems)
    {
        const int dx = qRound(rawDelta.x());
        const int dy = groupHorizOnly ? 0 : qRound(rawDelta.y());
        if ((dx == 0 && dy == 0) || !it.shape || !_pSD)
            continue;
        // Qt reports only the gesture; the model applies + recomputes + follows.
        _pSD->RecalculateAfterEdit(it.shape, CbSize(dx, dy));
        moved = true;
    }
    if (moved && _pSD && _pSD->GetDataModelDoc())
    {
        _pSD->GetDataModelDoc()->MarkLastUndo();
    }
    _dragItems.clear();
    _dragActive    = false;
    _dragPotential = false;
}

void SequenceDiagramCanvas::cancelDrag()
{
    if (!_dragPotential && !_dragActive)
        return;
    _dragItems.clear();
    _dragActive    = false;
    _dragPotential = false;
    unsetCursor();
    update();
}

// ---------------------------------------------------------------------------
// Message-drag -- the interactive form of Add Message (drag from an activation
// onto a lifeline / activation). The grab is on an activation, so it never
// overlaps the lifeline-move drag (lifelines) or box-select (empty space).
// On release the drop point is hit-tested; if {source,target} is a legal pair
// the two are selected (source first, target last -- the order addMessage()
// reads) and addMessage() does the rest (dialog, rollback, undo, refresh).
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::beginMessageDragIfReady(QPointF widgetPos)
{
    if (!_msgDragPotential || _msgDragActive)
        return;
    if ((widgetPos - _pressWidgetPos).manhattanLength() < 4)
        return;
    _msgDragActive = true;
    // Cursor (valid-target hand / no-drop) is set by the caller's active block
    // on this same move, so no need to seed one here.
}

// Mirror canAddMessage()'s two-selection rule: source must resolve to an
// activation; target is either a lifeline (call to a new activation there) or
// a distinct activation that the source can legally call (not its own parent,
// not one of its descendants -- no cycle).
bool SequenceDiagramCanvas::canMakeMessage(SequenceDiagramShape* pSource,
                                           SequenceDiagramShape* pTarget) const
{
    if (!pSource || !pTarget || pSource == pTarget)
        return false;
    ChildActivationShape* pCA1 = pSource->GetChildActivation();
    if (!pCA1)
        return false;
    if (pTarget->GetLifeLine())
        return true;
    ChildActivationShape* pCA2 = pTarget->GetChildActivation();
    return pCA2
 && pCA2 != pCA1
 && pCA2->GetParentActivationShape() != pCA1
 && !pCA1->IsDirectOrIndirectChild(pCA2);
}

void SequenceDiagramCanvas::commitMessageDrag(QPointF widgetPos)
{
    SequenceDiagramShape* pSource = _msgDragSource;
    _msgDragActive    = false;
    _msgDragPotential = false;
    _msgDragSource    = nullptr;
    unsetCursor();

    if (!_pSD || !_pViewModel || !pSource)
    {
        update();
        return;
    }

    const QPointF m = widgetToModel(widgetPos);
    const CbPoint modelPt(qRound(m.x()), qRound(m.y()));
    SequenceDiagramShape* pTarget = hitTest(modelPt);

    if (canMakeMessage(pSource, pTarget))
        createMessageFromDrag(pSource, pTarget);   // selects ONLY the new message
    else
        update();   // no valid target -- leave the source selected, just repaint
}

// Drag form of OnAddMessage. Source + target come straight from the drag (not
// the selection), so -- unlike addMessage() -- the lifeline drop target is never
// selected and its big selection box never flashes up (during the signal dialog
// or after). On success ONLY the resulting message is selected; the source
// activation selection from the press is cleared too. The two-shape cases mirror
// addMessage()'s two-selected branch exactly.
void SequenceDiagramCanvas::createMessageFromDrag(SequenceDiagramShape* pSource,
                                                  SequenceDiagramShape* pTarget)
{
    if (!_pSD || !_pViewModel || !pSource || !pTarget)
        return;
    DataModelDoc* doc = _pSD->GetDataModelDoc();
    void* ownerHwnd = reinterpret_cast<void*>(window()->winId());

    ChildActivationShape* pCA1 = pSource->GetChildActivation();
    ChildActivationShape* pCA2 = pTarget->GetChildActivation();
    LifeLineShape*        pLL2 = pTarget->GetLifeLine();
    if (!pCA1)
        return;

    UndoBase* pLastUndo = doc->MarkLastUndo();
    SequenceDiagramShape* pToSelect = nullptr;   // only the result, set on success

    if (pLL2)
    {
        // Call from pCA1 to a new activation on the target lifeline.
        ChildActivationShape* pNew = new ChildActivationShape(pLL2, pCA1);
        if (Qt_ShowSignalDialog(pNew->GetSender(), ownerHwnd))
            pToSelect = pNew->GetSender();
        else
            doc->RollBack(pLastUndo);
    }
    else if (pCA2
 && pCA2->GetParentActivationShape() != pCA1
 && !pCA1->IsDirectOrIndirectChild(pCA2))
    {
        // Reparent pCA2 under pCA1, carrying or creating its signal.
        pCA2->SaveState(1);
        pCA1->MoveChildActivationShapeLast(pCA2);
        SignalShape* pSignalShape = pCA2->GetSender();
        if (pSignalShape)
        {
            pSignalShape->SaveState(1);
            pCA1->MoveReceiverLast(pSignalShape);
            pToSelect = pSignalShape;
        }
        else
        {
            pSignalShape = new SignalShape(pCA2, pCA1);
            if (Qt_ShowSignalDialog(pSignalShape, ownerHwnd))
                pToSelect = pSignalShape;
            else
                doc->RollBack(pLastUndo);
        }
    }

    // Replace whatever was selected (the source activation from the press) with
    // ONLY the new message -- never the lifeline drop target.
    _pViewModel->DeleteAllSelected();
    if (pToSelect)
        (void)new SequenceDiagramViewModelSelection(_pViewModel, pToSelect);
    doc->MarkLastUndo();
    raiseToFront();
    update();
}

void SequenceDiagramCanvas::cancelMessageDrag()
{
    if (!_msgDragPotential && !_msgDragActive)
        return;
    _msgDragActive    = false;
    _msgDragPotential = false;
    _msgDragSource    = nullptr;
    unsetCursor();
    update();
}

// Connector preview -- a dashed line from the grab point to the cursor, capped
// with a solid FILLED-triangle arrowhead (style of SignalShape::DrawArrow). The
// line ALWAYS runs full length to the cursor: over a valid target the arrow then
// touches the lifeline (where the message lands on release); over an invalid
// area only the arrowHEAD is pulled back along the line so the centred no-drop
// cursor doesn't blanket it, while the line still fills the space behind it (so
// no empty gap shows). Painted inside the model transform (like the drag ghost)
// so it tracks geometry at any zoom; the isotropic + Y-flip keeps it oriented.
void SequenceDiagramCanvas::paintMessageDragLine(QPainter& qp)
{
    if (!_msgDragActive)
        return;
    const QPointF a      = _msgDragStartModelPt;    // grab point
    const QPointF cursor = _msgDragCurrentModelPt;  // actual drop / cursor point

    const QColor accent = palette().color(QPalette::Active, QPalette::Highlight);

    qp.save();
    QPen pen(accent);
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawLine(a, cursor);   // always full length -> no empty gap, reaches the target

    // Arrowhead tip: at the cursor over a valid target (hand cursor leaves it
    // visible and it should touch the lifeline); pulled back ~16 device-px over
    // an invalid area so the centred no-drop sign doesn't cover it.
    QPointF tip = cursor;
    if (!_msgDragOverTarget)
    {
        const qreal ddx = cursor.x() - a.x();
        const qreal ddy = cursor.y() - a.y();
        const qreal dlen = std::hypot(ddx, ddy);
        const FitInfo f = computeFit();
        const qreal scale = f.fitScale * _zoom;
        if (dlen > 1e-3 && scale > 1e-6)
        {
            // Just enough to clear the no-drop sign and sit right next to it --
            // not a trailing gap.
            const qreal pull = qMin(10.0 / scale, dlen * 0.5);   // don't cross the grab pt
            tip = QPointF(cursor.x() - ddx / dlen * pull,
                          cursor.y() - ddy / dlen * pull);
        }
    }

    const qreal dx = a.x() - tip.x();           // tip -> grab direction
    const qreal dy = a.y() - tip.y();
    const qreal len = std::hypot(dx, dy);
    if (len > 1e-3)
    {
        const qreal ux = dx / len, uy = dy / len;
        const qreal size = 16.0;   // == SignalShape's sync arrow (solid fill reads fine)
        const QPointF av(ux * size, uy * size);           // back along the line
        const QPointF bv(-uy * (size / 2), ux * (size / 2));  // perpendicular
        QPolygonF head;
        head << tip << (tip + av - bv) << (tip + av + bv);
        QPen edge(accent);
        edge.setCosmetic(true);
        edge.setWidth(1);
        edge.setStyle(Qt::SolidLine);
        qp.setPen(edge);
        qp.setBrush(accent);                              // filled, like a real message
        qp.drawPolygon(head);
    }
    qp.restore();
}

// ---------------------------------------------------------------------------
// Signal vertical move -- drag a message arrow up/down to reposition its
// RECEIVING activation. Grabbed on the signal (not the activation), so it's
// orthogonal to the message-create drag. Vertical only; the model is mutated
// once on release (overlay-style), mirroring ChildActivationShape::Track's
// commit: SetRect + SetOffset, clamped upward by GetMaxUpOffset.
// ---------------------------------------------------------------------------
// Which activation the drag moves: the receiving one (signal's child), or the
// SENDING one (its parent) when Alt is held. Alt just picks the end; the cascade
// does the rest. Sender that's a pinned RootActivationShape -> fall back to the
// receiver (a root has no offset to move).
ChildActivationShape* SequenceDiagramCanvas::signalMoveTarget() const
{
    if (!_sigMoveSignal)
        return nullptr;
    ChildActivationShape* pRecv = _sigMoveSignal->GetChildActivation();
    if (!pRecv)
        return nullptr;
    if (_sigMoveBothEnds)
    {
        ParentActivationShape* pParent = pRecv->GetParentActivationShape();
        if (pParent && pParent->IsChildActivationShape())
            return static_cast<ChildActivationShape*>(pParent);
    }
    return pRecv;
}

// Clamped, grid-snapped vertical model delta for the in-progress drag (shared
// by the ghost and the commit so they always agree).
int SequenceDiagramCanvas::signalMoveOffsetY() const
{
    ChildActivationShape* pCA = signalMoveTarget();
    if (!pCA)
        return 0;
    CbSize offset(0, qRound(_sigMoveCurrentModelPt.y() - _sigMoveStartModelPt.y()));
    Shape::Round(offset);                       // snap to grid (matches MFC)
    if (offset.cy > pCA->GetMaxUpOffset())      // clamp upward travel only
        offset.cy = pCA->GetMaxUpOffset();
    return offset.cy;
}

void SequenceDiagramCanvas::beginSignalMoveIfReady(QPointF widgetPos)
{
    if (!_sigMovePotential || _sigMoveActive)
        return;
    if ((widgetPos - _pressWidgetPos).manhattanLength() < 4)
        return;
    _sigMoveActive = true;
    setCursor(Qt::SizeVerCursor);   // vertical-only move affordance
}

void SequenceDiagramCanvas::commitSignalMove()
{
    // Resolve target + delta BEFORE clearing the drag state they read from.
    ChildActivationShape* pCA = signalMoveTarget();
    const int dy = signalMoveOffsetY();
    _sigMoveActive    = false;
    _sigMovePotential = false;
    _sigMoveSignal    = nullptr;
    _sigMoveBothEnds  = false;
    unsetCursor();

    // Qt reports only the gesture (which activation, vertical offset); the model
    // applies the move, recomputes, and carries attached note points.
    if (_pSD && pCA && dy != 0)
    {
        _pSD->RecalculateAfterEdit(pCA, CbSize(0, dy));
        _pSD->GetDataModelDoc()->MarkLastUndo();
    }
    update();
}

void SequenceDiagramCanvas::cancelSignalMove()
{
    if (!_sigMovePotential && !_sigMoveActive)
        return;
    _sigMoveActive    = false;
    _sigMovePotential = false;
    _sigMoveSignal    = nullptr;
    _sigMoveBothEnds  = false;
    unsetCursor();
    update();
}

// Ghost -- outline of the moved activation (receiver, or sender under Alt) at its
// would-be position (which rect ghosts also signals which end Alt picked), plus the
// signal arrow itself -- where the cursor is -- and its return arrow if enabled, all
// shifted by the same dy so they ride up/down together.
void SequenceDiagramCanvas::paintSignalMoveGhost(QPainter& qp)
{
    if (!_sigMoveActive || !_sigMoveSignal)
        return;
    ChildActivationShape* pCA = signalMoveTarget();
    if (!pCA)
        return;
    const int dy = signalMoveOffsetY();
    const CbRect& r = pCA->GetRect();
    const qreal x1 = qMin<qreal>(r.left, r.right);
    const qreal x2 = qMax<qreal>(r.left, r.right);
    const qreal y1 = qMin<qreal>(r.top,  r.bottom) + dy;
    const qreal y2 = qMax<qreal>(r.top,  r.bottom) + dy;

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));

    // The signal arrow is the part you grab, so it must always show -- ghost it at the
    // same dy. A recursive (self) signal is a 3-segment loop (right / up / back), not a
    // straight line, so mirror SignalShape::Draw's path; otherwise it collapses to a
    // short diagonal. A normal arrow is just start->end. Return arrow too when enabled.
    const CbPoint s = _sigMoveSignal->GetStartPoint();
    const CbPoint e = _sigMoveSignal->GetEndPoint();
    if (_sigMoveSignal->IsRecursiveActivation())
    {
        const CbPoint p1 = s + CbSize(SequenceDiagram::GetSignalLengthRecursive(), 0)
                             + CbSize(0, -_sigMoveSignal->GetDuration() / 2);
        const CbPoint p2 = p1 + CbSize(0, -SequenceDiagram::GetActivationSpaceRecursive());
        qp.drawLine(QPointF(s.x,  s.y  + dy), QPointF(p1.x, p1.y + dy));
        qp.drawLine(QPointF(p1.x, p1.y + dy), QPointF(p2.x, p2.y + dy));
        qp.drawLine(QPointF(p2.x, p2.y + dy), QPointF(e.x,  e.y  + dy));
    }
    else
    {
        qp.drawLine(QPointF(s.x, s.y + dy), QPointF(e.x, e.y + dy));
    }
    if (_sigMoveSignal->GetEnableReturn())
    {
        const CbPoint rs = _sigMoveSignal->GetReturnStartPoint();
        const CbPoint re = _sigMoveSignal->GetReturnEndPoint();
        qp.drawLine(QPointF(rs.x, rs.y + dy), QPointF(re.x, re.y + dy));
    }

    // Create message: the arrow lands on the class-header box, which rides down with the
    // creation (its top tracks the creation activation's bottom, fixed height -> rigid
    // +dy). The activation ghost above doesn't include it, so ghost the header box too.
    ChildActivationShape* pRecv = _sigMoveSignal->GetReceiver();
    if (pRecv && pRecv->GetCreation())
        if (LifeLineShape* pLL = pRecv->GetLifeLineShape())
        {
            const CbRect& hr = pLL->GetRect();
            const qreal hx1 = qMin<qreal>(hr.left, hr.right);
            const qreal hx2 = qMax<qreal>(hr.left, hr.right);
            const qreal hy1 = qMin<qreal>(hr.top,  hr.bottom) + dy;
            const qreal hy2 = qMax<qreal>(hr.top,  hr.bottom) + dy;
            qp.drawRect(QRectF(hx1, hy1, hx2 - hx1, hy2 - hy1));
        }
    qp.restore();
}

// ---------------------------------------------------------------------------
// Signal text sub-part drag -- move a name/label/return text block. Routed here
// (not to the activation move) when the press lands on a text rect; text wins
// over the arrow where they overlap (the name rect intersects the arrow active
// area). Free move (X+Y); the model is touched once on release via
// SetNamePoint/SetLabelPoint/SetReturnPoint (each stores the sub-part offset +
// SaveState). Mirrors MFC TrackName/TrackLabel/TrackReturn.
// ---------------------------------------------------------------------------
SequenceDiagramCanvas::SignalPart
SequenceDiagramCanvas::signalPartAt(SignalShape* pSig, const CbPoint& modelPt) const
{
    if (!pSig)
        return SignalPart::None;
    CbPainter_QFontMetrics m;
    // Text first -- it takes priority over the overlapping arrow active area.
    if (pSig->GetNameRect(m).PtInRect(modelPt))
        return SignalPart::Name;
    if (!pSig->GetLabel().IsEmpty() && pSig->GetLabelRect(m).PtInRect(modelPt))
        return SignalPart::Label;
    if (pSig->GetEnableReturn() && !pSig->GetReturn().IsEmpty() &&
        pSig->GetReturnRect(m).PtInRect(modelPt))
        return SignalPart::Return;
    if (pSig->GetActiveAreaRect().PtInRect(modelPt))
        return SignalPart::Arrow;
    if (pSig->GetEnableReturn() &&
        pSig->GetReturnActiveAreaRect().PtInRect(modelPt))
        return SignalPart::Arrow;
    return SignalPart::None;
}

CbRect SequenceDiagramCanvas::textDragRect() const
{
    if (!_textDragSig)
        return CbRect();
    CbPainter_QFontMetrics m;
    switch (_textDragPart)
    {
    case SignalPart::Name:   return _textDragSig->GetNameRect(m);
    case SignalPart::Label:  return _textDragSig->GetLabelRect(m);
    case SignalPart::Return: return _textDragSig->GetReturnRect(m);
    default:                 return CbRect();
    }
}

void SequenceDiagramCanvas::beginTextDragIfReady(QPointF widgetPos)
{
    if (!_textDragPotential || _textDragActive)
        return;
    if ((widgetPos - _pressWidgetPos).manhattanLength() < 4)
        return;
    _textDragActive = true;
    setCursor(Qt::SizeAllCursor);   // free move
}

void SequenceDiagramCanvas::commitTextDrag()
{
    SignalShape*     pSig = _textDragSig;
    const SignalPart part = _textDragPart;
    CbSize off(qRound(_textDragCurrentModelPt.x() - _textDragStartModelPt.x()),
               qRound(_textDragCurrentModelPt.y() - _textDragStartModelPt.y()));
    Shape::Round(off);
    _textDragActive    = false;
    _textDragPotential = false;
    _textDragSig       = nullptr;
    _textDragPart      = SignalPart::None;
    unsetCursor();

    if (!_pSD || !pSig || (off.cx == 0 && off.cy == 0))
    {
        update();
        return;
    }
    DataModelDoc* doc = _pSD->GetDataModelDoc();
    // Set*Point store the offset (point - referencePoint) and SaveState.
    if      (part == SignalPart::Name)   pSig->SetNamePoint(pSig->GetNamePoint() + off);
    else if (part == SignalPart::Label)  pSig->SetLabelPoint(pSig->GetLabelPoint() + off);
    else if (part == SignalPart::Return) pSig->SetReturnPoint(pSig->GetReturnPoint() + off);
    doc->MarkLastUndo();
    update();
}

void SequenceDiagramCanvas::cancelTextDrag()
{
    if (!_textDragPotential && !_textDragActive)
        return;
    _textDragActive    = false;
    _textDragPotential = false;
    _textDragSig       = nullptr;
    _textDragPart      = SignalPart::None;
    unsetCursor();
    update();
}

void SequenceDiagramCanvas::paintTextDragGhost(QPainter& qp)
{
    if (!_textDragActive || !_textDragSig)
        return;
    const CbRect r = textDragRect();
    CbSize off(qRound(_textDragCurrentModelPt.x() - _textDragStartModelPt.x()),
               qRound(_textDragCurrentModelPt.y() - _textDragStartModelPt.y()));
    Shape::Round(off);
    const qreal x1 = qMin<qreal>(r.left, r.right)  + off.cx;
    const qreal x2 = qMax<qreal>(r.left, r.right)  + off.cx;
    const qreal y1 = qMin<qreal>(r.top,  r.bottom) + off.cy;
    const qreal y2 = qMax<qreal>(r.top,  r.bottom) + off.cy;

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    qp.restore();
}

// ---------------------------------------------------------------------------
// Note sub-part drag -- move the whole note (body) or resize via the left /
// right edge handles (min width 100). Overlay-style: the model is touched once
// on release via SetRect. Mirrors MFC SDNoteShape::Track routing
// (TrackMove / TrackResizeLeft / TrackResizeRight).
// ---------------------------------------------------------------------------
SequenceDiagramCanvas::NotePart
SequenceDiagramCanvas::notePartAt(SDNoteShape* pNote, const CbPoint& modelPt) const
{
    if (!pNote)
        return NotePart::None;
    // Handle hit tolerance ~8 device px (MFC uses a 6px box); convert to model.
    const FitInfo f = computeFit();
    const qreal scale = f.fitScale * _zoom;
    // Grab half-width ~6 device px -- a bit bigger than the 5px-drawn handle
    // (drawn half = 2.5px), so it's easy to grab while staying visually tidy.
    const int tol = QtHandle::grabToleranceModel(scale);
    const CbPoint lp = pNote->GetLeftSelectedPoint();
    const CbPoint rp = pNote->GetRightSelectedPoint();
    if (qAbs(modelPt.x - lp.x) <= tol && qAbs(modelPt.y - lp.y) <= tol)
        return NotePart::ResizeLeft;
    if (qAbs(modelPt.x - rp.x) <= tol && qAbs(modelPt.y - rp.y) <= tol)
        return NotePart::ResizeRight;
    return NotePart::Body;   // anywhere else on the note moves it
}

SDNoteShape*
SequenceDiagramCanvas::selectedNoteResizeAt(const CbPoint& modelPt, NotePart& part) const
{
    part = NotePart::None;
    if (!_pViewModel)
        return nullptr;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
        SDNoteShape* pNote = pShape ? pShape->GetNoteShape() : nullptr;
        if (!pNote)
            continue;
        const NotePart p = notePartAt(pNote, modelPt);
        if (p == NotePart::ResizeLeft || p == NotePart::ResizeRight)
        {
            part = p;
            return pNote;
        }
    }
    return nullptr;
}

SDNoteShapePoint*
SequenceDiagramCanvas::notePointAt(SDNoteShape* pNote, const CbPoint& modelPt) const
{
    if (!pNote)
        return nullptr;
    const FitInfo f = computeFit();
    const qreal scale = f.fitScale * _zoom;
    const int tol = QtHandle::grabToleranceModel(scale);
    // The relation iterator is NESTED in SDNoteShape, so qualify it from here
    // (the model's own code uses the unqualified name from in-class scope). The
    // iterator is the safe traversal -- its Check() nulls _ref if a point is
    // replaced/deleted mid-walk.
    SDNoteShape::SDNoteShapePointIterator iPt(pNote);
    while (++iPt)
    {
        const CbPoint pt = iPt->GetPoint();
        if (qAbs(modelPt.x - pt.x) <= tol && qAbs(modelPt.y - pt.y) <= tol)
            return iPt.Get();
    }
    return nullptr;
}

// A selected note's attach-line END-POINT handle. A point beats a line and a
// selected shape takes priority -- so this is checked ahead of hitTest in both
// the hover-cursor and the press; otherwise a point sitting ON a signal arrow
// is stolen by the signal's up/down move (the line wins the plain hitTest).
SDNoteShapePoint*
SequenceDiagramCanvas::selectedNotePointAt(const CbPoint& modelPt,
                                           SDNoteShape*& pNoteOut) const
{
    pNoteOut = nullptr;
    if (!_pViewModel)
        return nullptr;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
        SDNoteShape* pNote = pShape ? pShape->GetNoteShape() : nullptr;
        if (!pNote)
            continue;
        if (SDNoteShapePoint* pPt = notePointAt(pNote, modelPt))
        {
            pNoteOut = pNote;
            return pPt;
        }
    }
    return nullptr;
}

CbPoint SequenceDiagramCanvas::snapNotePointToSignal(CbPoint p, int tol) const
{
    if (!_pSD)
        return p;
    int bestY = p.y, bestDist = tol + 1;
    SequenceDiagram::SequenceDiagramShapeIterator iShape(_pSD);
    while (++iShape)
    {
        SignalShape* pSig = iShape->GetSignal();
        if (!pSig)
            continue;
        const CbPoint sp = pSig->GetStartPoint();
        const CbPoint ep = pSig->GetEndPoint();
        const int lineY = sp.y;                       // horizontal arrow line
        if (p.x < qMin(sp.x, ep.x) - tol || p.x > qMax(sp.x, ep.x) + tol)
            continue;                                 // not over this arrow's span
        const int d = qAbs(p.y - lineY);
        if (d < bestDist)
        {
            bestDist = d;
            bestY    = lineY;
        }
    }
    if (bestDist <= tol)
        p.y = bestY;
    return p;
}

// Would-be note rect for the current drag (shared by ghost + commit).
CbRect SequenceDiagramCanvas::noteDragRect() const
{
    CbRect rect = _noteDragPressRect;
    if (_noteDragPart == NotePart::Body)
    {
        CbSize off(qRound(_noteDragCurrentModelPt.x() - _noteDragStartModelPt.x()),
                   qRound(_noteDragCurrentModelPt.y() - _noteDragStartModelPt.y()));
        Shape::Round(off);
        rect.OffsetRect(off.cx, off.cy);
    }
    else if (_noteDragPart == NotePart::ResizeLeft)
    {
        CbPoint p(qRound(_noteDragCurrentModelPt.x()), rect.top);
        Shape::Round(p);
        rect.left = qMin(p.x, rect.right - 100);   // keep min width
    }
    else if (_noteDragPart == NotePart::ResizeRight)
    {
        CbPoint p(qRound(_noteDragCurrentModelPt.x()), rect.top);
        Shape::Round(p);
        rect.right = qMax(p.x, rect.left + 100);
    }
    return rect;
}

void SequenceDiagramCanvas::beginNoteDragIfReady(QPointF widgetPos)
{
    if (!_noteDragPotential || _noteDragActive)
        return;
    if ((widgetPos - _pressWidgetPos).manhattanLength() < 4)
        return;
    _noteDragActive = true;
    const bool resize = (_noteDragPart == NotePart::ResizeLeft ||
                         _noteDragPart == NotePart::ResizeRight);
    setCursor(resize ? Qt::SizeHorCursor : Qt::SizeAllCursor);  // body/point free-move
}

void SequenceDiagramCanvas::commitNoteDrag()
{
    SDNoteShape*      pNote  = _noteDragNote;
    const NotePart    part   = _noteDragPart;
    SDNoteShapePoint* pPoint = _noteDragPoint;
    const CbRect newRect = noteDragRect();
    CbPoint newPoint(qRound(_noteDragCurrentModelPt.x()),
                     qRound(_noteDragCurrentModelPt.y()));
    Shape::Round(newPoint);
    newPoint = snapNotePointToSignal(newPoint, 8);   // land exactly on a near line

    _noteDragActive    = false;
    _noteDragPotential = false;
    _noteDragNote      = nullptr;
    _noteDragPart      = NotePart::None;
    _noteDragPoint     = nullptr;
    unsetCursor();

    if (!_pSD || !pNote)
    {
        update();
        return;
    }
    DataModelDoc* doc = _pSD->GetDataModelDoc();
    // Qt reports only the gesture; the model applies (SetPoint spawns/deletes/moves
    // a corner, or SetRect for body/resize), recomputes, and follows.
    if (part == NotePart::Point && pPoint)
    {
        const CbPoint oldPoint = pPoint->GetPoint();
        _pSD->RecalculateAfterEdit(pNote,
            CbSize(newPoint.x - oldPoint.x, newPoint.y - oldPoint.y), oldPoint);
    }
    else if (newRect != _noteDragPressRect)   // body move or resize -> new rect
    {
        _pSD->RecalculateAfterEdit(pNote, CbSize(0, 0),
                                 CbPoint(INT_MIN, INT_MIN), newRect);
        CbPainter_QFontMetrics m;
        pNote->RecalcHeight(m);   // width may have changed -> re-derive height off-view
                                  // (RecalculateAfterEdit has no painter to measure with)
    }
    else
    {
        update();
        return;
    }
    doc->MarkLastUndo();
    update();
}

void SequenceDiagramCanvas::cancelNoteDrag()
{
    if (!_noteDragPotential && !_noteDragActive)
        return;
    _noteDragActive    = false;
    _noteDragPotential = false;
    _noteDragNote      = nullptr;
    _noteDragPart      = NotePart::None;
    unsetCursor();
    update();
}

void SequenceDiagramCanvas::paintNoteDragGhost(QPainter& qp)
{
    if (!_noteDragActive || !_noteDragNote)
        return;

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);

    if (_noteDragPart == NotePart::Point)
    {
        // Connector preview: a dashed line from the note edge (where it re-
        // anchors) to the dragged point, plus a small marker square.
        CbPoint cur(qRound(_noteDragCurrentModelPt.x()),
                    qRound(_noteDragCurrentModelPt.y()));
        Shape::Round(cur);
        cur = snapNotePointToSignal(cur, 8);   // preview the snapped landing
        const CbPoint cross = Shape::CrossPoint(_noteDragNote->GetRect(), cur);
        qp.drawLine(QPointF(cross.x, cross.y), QPointF(cur.x, cur.y));
        const FitInfo f = computeFit();
        const qreal scale = f.fitScale * _zoom;
        const qreal hs = scale > 1e-6 ? 5.0 / scale : 5.0;
        qp.drawRect(QRectF(cur.x - hs / 2, cur.y - hs / 2, hs, hs));
        qp.restore();
        return;
    }

    const CbRect r = noteDragRect();
    const qreal x1 = qMin<qreal>(r.left, r.right);
    const qreal x2 = qMax<qreal>(r.left, r.right);
    const qreal y1 = qMin<qreal>(r.top,  r.bottom);
    const qreal y2 = qMax<qreal>(r.top,  r.bottom);
    qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    qp.restore();
}

// ---------------------------------------------------------------------------
// Class-lifeline resize -- drag a SELECTED class lifeline's L/R edge handle to
// give it a manual width. Same shape as the note resize (selected-handle
// priority, overlay-style ghost, model touched once on release). Commit turns
// AutoWidth off and carries attached note-connector points with the edge, as
// MFC's TrackResizeLeft/Right do. The lifeline body stays move-draggable.
// ---------------------------------------------------------------------------
bool SequenceDiagramCanvas::classLifeLineHandleAt(ClassLifeLineShape* pLL,
                                                  const CbPoint& modelPt,
                                                  bool& right) const
{
    if (!pLL)
        return false;
    const FitInfo f = computeFit();
    const qreal scale = f.fitScale * _zoom;
    const int tol = QtHandle::grabToleranceModel(scale);
    const CbPoint lp = pLL->GetLeftSelectedPoint();
    const CbPoint rp = pLL->GetRightSelectedPoint();
    if (qAbs(modelPt.x - lp.x) <= tol && qAbs(modelPt.y - lp.y) <= tol)
    {
        right = false;
        return true;
    }
    if (qAbs(modelPt.x - rp.x) <= tol && qAbs(modelPt.y - rp.y) <= tol)
    {
        right = true;
        return true;
    }
    return false;
}

ClassLifeLineShape*
SequenceDiagramCanvas::selectedClassLifeLineResizeAt(const CbPoint& modelPt,
                                                     bool& right) const
{
    if (!_pViewModel)
        return nullptr;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
        if (!pShape || !pShape->IsClassLifeLineShape())
            continue;
        ClassLifeLineShape* pLL = static_cast<ClassLifeLineShape*>(pShape);
        if (classLifeLineHandleAt(pLL, modelPt, right))
            return pLL;
    }
    return nullptr;
}

// Would-be rect for the current resize (shared by ghost + commit): the dragged
// edge follows the grid-snapped cursor X, clamped to min width 100; the other
// edge stays put. (Matches the note resize; MFC's xMax/xMin = right-100/left+100.)
CbRect SequenceDiagramCanvas::classLifeLineResizeRect() const
{
    CbRect rect = _llResizePressRect;
    CbPoint p(qRound(_llResizeCurrentModelPt.x()), rect.top);
    Shape::Round(p);
    if (_llResizeRight)
        rect.right = qMax(p.x, rect.left + 100);
    else
        rect.left = qMin(p.x, rect.right - 100);
    return rect;
}

void SequenceDiagramCanvas::beginClassLifeLineResizeIfReady(QPointF widgetPos)
{
    if (!_llResizePotential || _llResizeActive)
        return;
    if ((widgetPos - _pressWidgetPos).manhattanLength() < 4)
        return;
    _llResizeActive = true;
    setCursor(Qt::SizeHorCursor);
}

void SequenceDiagramCanvas::commitClassLifeLineResize()
{
    ClassLifeLineShape* pLL    = _llResizeShape;
    const CbRect        newRect = classLifeLineResizeRect();

    _llResizeActive    = false;
    _llResizePotential = false;
    _llResizeShape     = nullptr;
    unsetCursor();

    if (!_pSD || !pLL || newRect == _llResizePressRect)
    {
        update();
        return;
    }
    // Qt reports the gesture (the lifeline + its new width rect); the model applies
    // the resize (SetAutoWidth + SetRect) and follows the notes.
    _pSD->RecalculateAfterEdit(pLL, CbSize(0, 0), CbPoint(INT_MIN, INT_MIN), newRect);
    _pSD->GetDataModelDoc()->MarkLastUndo();
    update();
}

void SequenceDiagramCanvas::cancelClassLifeLineResize()
{
    if (!_llResizePotential && !_llResizeActive)
        return;
    _llResizeActive    = false;
    _llResizePotential = false;
    _llResizeShape     = nullptr;
    unsetCursor();
    update();
}

void SequenceDiagramCanvas::paintClassLifeLineResizeGhost(QPainter& qp)
{
    if (!_llResizeActive || !_llResizeShape)
        return;
    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);
    const CbRect r = classLifeLineResizeRect();
    const qreal x1 = qMin<qreal>(r.left, r.right);
    const qreal x2 = qMax<qreal>(r.left, r.right);
    const qreal y1 = qMin<qreal>(r.top,  r.bottom);
    const qreal y2 = qMax<qreal>(r.top,  r.bottom);
    qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    qp.restore();
}

// Ghost overlay -- one cosmetic-outlined rect per dragged shape at the
// would-be position, painted INSIDE the model transform so it sits in
// the right place at any zoom. The original positions stay visible as
// the unchanged model paint; the ghost shows where the shape will land
// if the user releases now.
void SequenceDiagramCanvas::paintDragGhost(QPainter& qp)
{
    if (!_dragActive || _dragItems.isEmpty())
        return;
    const QPointF rawDelta = _dragCurrentModelPt - _pressModelPt;
    // Mirror commitDrag's group constraint so the ghost matches the commit:
    // a lifeline-led drag is horizontal for the whole group (notes ride x-only).
    bool groupHorizOnly = false;
    for (const DragItem& it : _dragItems)
        if (it.shape && it.shape->IsLifeLineShape()) { groupHorizOnly = true; break; }

    qp.save();
    QPen pen(palette().color(QPalette::Active, QPalette::Highlight));
    pen.setCosmetic(true);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    qp.setPen(pen);
    qp.setBrush(Qt::NoBrush);

    for (const DragItem& it : _dragItems)
    {
        const qreal dx = rawDelta.x();
        const qreal dy = groupHorizOnly ? 0 : rawDelta.y();
        const CbRect& r = it.originalRect;
        const qreal x1 = qMin<qreal>(r.left, r.right)  + dx;
        const qreal x2 = qMax<qreal>(r.left, r.right)  + dx;
        const qreal y1 = qMin<qreal>(r.top,  r.bottom) + dy;
        const qreal y2 = qMax<qreal>(r.top,  r.bottom) + dy;
        qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    }
    qp.restore();
}

// Find the existing selection junction for `pShape` on this canvas's
// ViewModel. Linear scan -- selection counts are tiny in practice (rarely
// more than a handful of shapes).
SequenceDiagramViewModelSelection*
SequenceDiagramCanvas::findSelection(SequenceDiagramShape* pShape) const
{
    if (!_pViewModel || !pShape)
        return nullptr;
    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        if (iSel->GetSequenceDiagramShape() == pShape)
            return iSel.Get();
    }
    return nullptr;
}

// Second-pass selection highlight, painted within the model transform.
// Each selected shape gets a translucent fill + 2-device-pixel cosmetic
// outline (the /_zoom keeps the visual outline width constant across
// zoom levels -- pure 0-width cosmetic is hard to see on light fills).
void SequenceDiagramCanvas::paintSelectionOverlay(QPainter& qp)
{
    if (!_pViewModel)
        return;

    const QColor accent = palette().color(QPalette::Active, QPalette::Highlight);
    QColor fill = accent;  fill.setAlpha(56);
    QColor edge = accent;  edge.setAlpha(220);

    qp.save();
    QPen pen(edge);
    pen.setCosmetic(true);
    pen.setWidth(2);
    qp.setPen(pen);
    qp.setBrush(fill);

    // Local helper: draw one rect outlined, normalising MFC's "top can be
    // either above or below bottom numerically" CbRect convention (model
    // Y is up).
    auto drawRectOutline = [&](const CbRect& r) {
        const qreal x1 = qMin<qreal>(r.left, r.right);
        const qreal x2 = qMax<qreal>(r.left, r.right);
        const qreal y1 = qMin<qreal>(r.top,  r.bottom);
        const qreal y2 = qMax<qreal>(r.top,  r.bottom);
        qp.drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
    };

    SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
    while (++iSel)
    {
        SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
        if (!pShape)
            continue;

        if (pShape->IsSignalShape())
        {
            // Signals have independently-movable sub-parts; outline each so it
            // reads as part of the selected message AND signals that each is
            // separately draggable (arrow body -> activation move; the text
            // blocks -> their own offset). The text rects are measured off-view
            // via a headless painter (M2.9 text-measurement path).
            SignalShape* pSig = pShape->GetSignal();
            drawRectOutline(pSig->GetActiveAreaRect());
            if (pSig->GetEnableReturn())
                drawRectOutline(pSig->GetReturnActiveAreaRect());

            CbPainter_QFontMetrics m;
            drawRectOutline(pSig->GetNameRect(m));
            if (!pSig->GetLabel().IsEmpty())
                drawRectOutline(pSig->GetLabelRect(m));
            if (pSig->GetEnableReturn() && !pSig->GetReturn().IsEmpty())
                drawRectOutline(pSig->GetReturnRect(m));
        }
        else if (pShape->GetNoteShape())
        {
            // Note: outline the box + draw the two resize handles (small filled
            // squares at the L/R edge centres, constant ~7 device px so they
            // stay grabbable at any zoom).
            SDNoteShape* pNote = pShape->GetNoteShape();
            drawRectOutline(pNote->GetBoundingRect());

            const FitInfo f = computeFit();
            const qreal scale = f.fitScale * _zoom;
            const qreal hs = scale > 1e-6 ? 5.0 / scale : 5.0;   // ~5 device px
            qp.save();
            qp.setBrush(edge);
            const CbPoint handles[2] = { pNote->GetLeftSelectedPoint(),
                                         pNote->GetRightSelectedPoint() };
            for (const CbPoint& p : handles)
                qp.drawRect(QRectF(p.x - hs / 2, p.y - hs / 2, hs, hs));
            // Attach-line points (incl. the corner) -- grabbable squares; drag
            // the corner out to grow the connector.
            SDNoteShape::SDNoteShapePointIterator iPt(pNote);
            while (++iPt)
            {
                const CbPoint pt = iPt->GetPoint();
                qp.drawRect(QRectF(pt.x - hs / 2, pt.y - hs / 2, hs, hs));
            }
            qp.restore();
        }
        else if (pShape->IsClassLifeLineShape())
        {
            // Class lifeline: outline + two L/R resize handles at the box edge
            // centres (drag one to set a manual width). Same handle style/size
            // as the note resize.
            ClassLifeLineShape* pLL = static_cast<ClassLifeLineShape*>(pShape);
            drawRectOutline(pLL->GetBoundingRect());

            const FitInfo f = computeFit();
            const qreal scale = f.fitScale * _zoom;
            const qreal hs = scale > 1e-6 ? 5.0 / scale : 5.0;
            qp.save();
            qp.setBrush(edge);
            const CbPoint handles[2] = { pLL->GetLeftSelectedPoint(),
                                         pLL->GetRightSelectedPoint() };
            for (const CbPoint& p : handles)
                qp.drawRect(QRectF(p.x - hs / 2, p.y - hs / 2, hs, hs));
            qp.restore();
        }
        else
        {
            drawRectOutline(pShape->GetBoundingRect());
        }
    }

    qp.restore();
}

// Hover-driven cursor. Per the M2 plan: hit-test from the canvas (no MFC
// view dependency), then pick a cursor based on what the shape ACTUALLY
// allows -- the cursor should TELL the user what a click-drag would do,
// not over-promise.
//
// Cursor table (per-axis: the cursor signals which way the shape can move):
//   - Class LifeLineShape (resizable)          -> body Qt::SizeAllCursor (move),
//       edge handle Qt::SizeHorCursor (resize). The class box is the only
//       lifeline that resizes, so move and resize are both horizontal and would
//       share SizeHor; SizeAll on the body distinguishes "move" from "resize".
//       The drag itself shows the move is still horizontal-only.
//   - Actor / plain LifeLineShape (no resize)  -> Qt::SizeHorCursor
//       horizontal-only slide (no resize to disambiguate from)
//   - SignalShape with a receiving activation  -> Qt::SizeVerCursor
//       drag the message up/down to move its activation (vertical-only)
//   - Other hits (activations, notes, ...)     -> default arrow
//       selectable; activations are the message-create drag source but have
//       no good native "link" cursor, so no special hover affordance
//   - No hit                                  -> default arrow
//
// All chosen cursors are NATIVE Win32 (IDC_SIZEWE / IDC_SIZENS / IDC_SIZEALL /
// IDC_ARROW),
// so they scale per-monitor DPI. Qt's OpenHand/ClosedHand are bitmap-only and
// render huge on a higher-DPI second monitor -- see
// [[feedback_qt_native_cursors_for_multimonitor]].
void SequenceDiagramCanvas::updateHoverCursor(QPointF widgetPt)
{
    if (!_pSD || _panning || _dragActive || _msgDragActive || _sigMoveActive ||
        _textDragActive || _noteDragActive || _llResizeActive)
        return;

    const QPointF m = widgetToModel(widgetPt);
    const CbPoint modelPt(qRound(m.x()), qRound(m.y()));

    // A selected note's resize handle (outer half is outside the rect, so check
    // before the inside-only GetHitShape path).
    NotePart noteResizePart = NotePart::None;
    if (selectedNoteResizeAt(modelPt, noteResizePart))
    {
        setCursor(Qt::SizeHorCursor);
        return;
    }

    // Likewise a selected class lifeline's edge handle (same outer-half-outside
    // reason). Body move is already SizeHor, so the handle matches -- both are
    // horizontal; the overlay squares show where the resize grab is.
    bool llResizeRight = false;
    if (selectedClassLifeLineResizeAt(modelPt, llResizeRight))
    {
        setCursor(Qt::SizeHorCursor);
        return;
    }

    // A selected note's end-point handle wins over whatever line sits under it
    // (commonly a signal arrow the point is anchored to): point beats line, and
    // a selected shape has priority -- so check it before the plain hitTest,
    // which would otherwise hand the signal its up/down move cursor.
    {
        SDNoteShape* pPtNote = nullptr;
        if (selectedNotePointAt(modelPt, pPtNote))
        {
            setCursor(Qt::SizeAllCursor);   // free-move / drag-out the point
            return;
        }
    }

    SequenceDiagramShape* pHit = hitTest(modelPt);

    if (pHit && pHit->IsLifeLineShape())
        // Class lifeline body moves with SizeAll (distinguishes from its SizeHor
        // resize handles, checked above); actor / plain lifelines aren't
        // resizable, so SizeHor keeps the horizontal-axis hint.
        setCursor(pHit->GetClassLifeLine() ? Qt::SizeAllCursor : Qt::SizeHorCursor);
    else if (pHit && pHit->GetNoteShape())
    {
        // Note: left/right edge handle resizes (SizeHor); body moves (SizeAll).
        const NotePart part = notePartAt(pHit->GetNoteShape(), modelPt);
        setCursor(part == NotePart::Body ? Qt::SizeAllCursor : Qt::SizeHorCursor);
    }
    else if (pHit && pHit->IsSignalShape())
    {
        // Per sub-part: a text block free-moves (SizeAll); the bare arrow line
        // moves the receiving activation up/down (SizeVer).
        const SignalPart part = signalPartAt(pHit->GetSignal(), modelPt);
        if (part == SignalPart::Name || part == SignalPart::Label ||
            part == SignalPart::Return)
            setCursor(Qt::SizeAllCursor);
        else if (pHit->GetChildActivation())
            setCursor(Qt::SizeVerCursor);
        else
            unsetCursor();
    }
    else
        unsetCursor();
}

// ---------------------------------------------------------------------------
// Zoom + pan
// ---------------------------------------------------------------------------
// Anchored zoom: keep the model point currently under `anchor` (widget coords)
// stationary while scaling. The user transform applied in paintEvent is
//   widgetPt = _pan + _zoom * fitPt(modelPt)
// so for `anchor` to stay invariant under a zoom factor `f`:
//   newPan = anchor + f * (_pan - anchor)
// Clamp the absolute zoom so the user can't lose the page entirely.
void SequenceDiagramCanvas::zoomAt(qreal factor, QPointF anchor)
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

void SequenceDiagramCanvas::resetView()
{
    _zoom = 1.0;
    _pan  = QPointF();
    updateScrollBars();
    update();
}

void SequenceDiagramCanvas::applyToolbarZoom(int op)
{
    const QPointF center(width() / 2.0, height() / 2.0);
    if (op > 0)
        zoomAt(1.15, center);
    else if (op < 0)
        zoomAt(1.0 / 1.15, center);
    else
        resetView();
}

// ---------------------------------------------------------------------------
// Scrollbar binding -- bidirectional, with a guard against echoing our own
// programmatic value writes back as user pan.
// ---------------------------------------------------------------------------
void SequenceDiagramCanvas::bindScrollBars(QScrollBar* h, QScrollBar* v)
{
    _hbar = h;
    _vbar = v;
    if (_hbar)
        connect(_hbar, &QScrollBar::valueChanged,
                this,  &SequenceDiagramCanvas::onScrollH);
    if (_vbar)
        connect(_vbar, &QScrollBar::valueChanged,
                this,  &SequenceDiagramCanvas::onScrollV);
    updateScrollBars();
}

// Scrollbar geometry from the current fit / zoom / pan. The scrollbar value
// represents "how far the page has scrolled left/up under the viewport":
//
//   horizontal: value=0     -> page-left  flush with viewport-left
//               value=max   -> page-right flush with viewport-right
//               max = scaledW - viewportW   (>0 only when zoomed past fit)
//
// Translating between scrollbar value and _pan -- the widget X of the page's
// left edge is (_pan.x + _zoom*originX), so value = -(_pan.x + _zoom*originX).
// Vertical is analogous; the model Y-flip means value=0 maps the model TOP
// (modelY=pageH) to viewport-top.
void SequenceDiagramCanvas::updateScrollBars()
{
    if (!_hbar && !_vbar)
        return;

    const FitInfo f = computeFit();
    const int rangeH = qMax<int>(0, qRound(f.scaledW - width()));
    const int rangeV = qMax<int>(0, qRound(f.scaledH - height()));

    _ignoreScrollSignals = true;

    if (_hbar)
    {
        _hbar->setRange(0, rangeH);
        _hbar->setPageStep(width());
        _hbar->setSingleStep(qMax(1, width() / 20));
        const int v = qRound(-(_pan.x() + _zoom * f.originX));
        _hbar->setValue(qBound(0, v, rangeH));
        _hbar->setEnabled(rangeH > 0);
    }
    if (_vbar)
    {
        _vbar->setRange(0, rangeV);
        _vbar->setPageStep(height());
        _vbar->setSingleStep(qMax(1, height() / 20));
        const int v = qRound(-(_pan.y() + _zoom * f.originY));
        _vbar->setValue(qBound(0, v, rangeV));
        _vbar->setEnabled(rangeV > 0);
    }

    _ignoreScrollSignals = false;
}

void SequenceDiagramCanvas::onScrollH(int v)
{
    if (_ignoreScrollSignals)
        return;
    const FitInfo f = computeFit();
    _pan.setX(-v - _zoom * f.originX);
    update();
}

void SequenceDiagramCanvas::onScrollV(int v)
{
    if (_ignoreScrollSignals)
        return;
    const FitInfo f = computeFit();
    _pan.setY(-v - _zoom * f.originY);
    update();
}

// Ctrl+wheel = zoom around cursor; bare wheel scrolls aren't yet meaningful
// (no scrollbars) and are ignored.
bool SequenceDiagramCanvas::event(QEvent* e)
{
    // macOS trackpad pinch arrives as a native zoom gesture (not a wheel event).
    // value() is the incremental scale delta per step; map it to a multiplicative
    // zoom factor about the gesture point.
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

void SequenceDiagramCanvas::wheelEvent(QWheelEvent* e)
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
    // wheel. Same _pan/repaint path as the middle-drag pan.
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

// Middle-mouse drag pans. Cursor switches to a closed hand for the drag.
// Left-button handles selection (M2.2): plain click selects a single shape
// (or clears all if empty); Ctrl-click toggles; Shift-click adds (no toggle).
void SequenceDiagramCanvas::mousePressEvent(QMouseEvent* e)
{
    if (placementActive())
    {
        // Left-click drops the shape; any other button is swallowed here so it
        // doesn't start a pan/select. Right-click cancellation is handled in
        // contextMenuEvent (so the normal popup doesn't also appear).
        if (e->button() == Qt::LeftButton)
            finishPlacementAt(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::MiddleButton)
    {
        _panning    = true;
        _lastPanPos = e->position();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && _pSD && _pViewModel)
    {
        const QPointF m = widgetToModel(e->position());
        const CbPoint modelPt(qRound(m.x()), qRound(m.y()));

        // Grabbing a selected note's resize handle takes priority -- the handle
        // straddles the note edge, so its outer half is OUTSIDE the rect where
        // GetHitShape would miss it. Keep the selection, start the resize.
        NotePart noteResizePart = NotePart::None;
        if (SDNoteShape* pNote = selectedNoteResizeAt(modelPt, noteResizePart))
        {
            _noteDragNote           = pNote;
            _noteDragPart           = noteResizePart;
            _noteDragPressRect      = pNote->GetRect();
            _noteDragStartModelPt   = m;
            _noteDragCurrentModelPt = m;
            _pressWidgetPos         = e->position();
            _noteDragPotential      = true;
            _noteDragActive         = false;
            e->accept();
            return;
        }

        // Same priority for a selected class lifeline's L/R resize handle (the
        // handle straddles the box edge, outer half outside the rect). Grabbing
        // it resizes instead of starting the body move-drag.
        bool llResizeRight = false;
        if (ClassLifeLineShape* pLL =
                selectedClassLifeLineResizeAt(modelPt, llResizeRight))
        {
            _llResizeShape          = pLL;
            _llResizeRight          = llResizeRight;
            _llResizePressRect      = pLL->GetRect();
            _llResizeStartModelPt   = m;
            _llResizeCurrentModelPt = m;
            _pressWidgetPos         = e->position();
            _llResizePotential      = true;
            _llResizeActive         = false;
            e->accept();
            return;
        }

        // A selected note's end-point handle takes priority over a line under
        // it (the signal it's anchored to): grab the point, not the signal.
        // Point-beats-line + selected-shape priority, checked before hitTest.
        {
            SDNoteShape* pPtNote = nullptr;
            if (SDNoteShapePoint* pPt = selectedNotePointAt(modelPt, pPtNote))
            {
                _noteDragNote           = pPtNote;
                _noteDragPart           = NotePart::Point;
                _noteDragPoint          = pPt;
                _noteDragPressRect      = pPtNote->GetRect();
                _noteDragStartModelPt   = m;
                _noteDragCurrentModelPt = m;
                _pressWidgetPos         = e->position();
                _noteDragPotential      = true;
                _noteDragActive         = false;
                e->accept();
                return;
            }
        }

        SequenceDiagramShape* pHit =
            hitTest(modelPt);

        const bool ctrl  = (e->modifiers() & Qt::ControlModifier) != 0;
        const bool shift = (e->modifiers() & Qt::ShiftModifier) != 0;

        if (ctrl || shift)
        {
            // Additive / toggle modifier. Empty-space click is a no-op
            // (matches MFC: Shift/Ctrl on empty doesn't clear selection).
            if (pHit)
            {
                if (SequenceDiagramViewModelSelection* pSel = findSelection(pHit))
                {
                    // Already selected. Ctrl removes (toggle); Shift keeps.
                    if (ctrl)
                        delete pSel;
                }
                else
                {
                    (void)new SequenceDiagramViewModelSelection(
                        _pViewModel, pHit);
                }
                update();
            }
        }
        else
        {
            // Plain click. If the hit shape is already selected, leave the
            // selection alone (preparing for a multi-shape drag). Otherwise
            // clear previous selection and select just this shape; empty
            // click clears all.
            const bool hitAlreadySelected =
                pHit && findSelection(pHit) != nullptr;
            if (!hitAlreadySelected)
            {
                _pViewModel->DeleteAllSelected();
                if (pHit)
                    (void)new SequenceDiagramViewModelSelection(_pViewModel, pHit);
                update();
            }
        }

        // After selection settled: if the hit shape (or any currently
        // selected shape) is draggable, set up drag-potential. The actual
        // drag activates on first mouseMove past the threshold.
        if (pHit && (isDraggableKind(pHit) ||
                     (pHit->IsSDNoteShape() && _pViewModel &&
                      _pViewModel->GetSelectedCount() > 1)))
        {
            _dragItems.clear();
            SequenceDiagramViewModel::SelectedIterator iSel(_pViewModel);
            while (++iSel)
            {
                SequenceDiagramShape* pShape = iSel->GetSequenceDiagramShape();
                // Group move: lifelines + any selected notes. The group goes x-only
                // when a lifeline is present (commitDrag's groupHorizOnly), so a note
                // rides along horizontally. Reached when the press is on a lifeline OR
                // on a note in a multi-select; a note pressed alone (or its resize/point
                // handles) takes the free-move note-drag path below.
                if (isDraggableKind(pShape) || (pShape && pShape->IsSDNoteShape()))
                    _dragItems.append({pShape, pShape->GetRect()});
            }
            if (!_dragItems.isEmpty())
            {
                _pressWidgetPos     = e->position();
                _pressModelPt       = m;
                _dragCurrentModelPt = m;
                _dragPotential      = true;
                _dragActive         = false;
            }
        }
        // Press on an activation (not move-draggable) arms a message-drag:
        // dragging out to a lifeline / other activation creates the call.
        else if (pHit && pHit->IsChildActivationShape())
        {
            _msgDragSource         = pHit;
            _msgDragStartModelPt   = m;
            _msgDragCurrentModelPt = m;
            _pressWidgetPos        = e->position();
            _msgDragPotential      = true;
            _msgDragActive         = false;
        }
        // Press on a signal: route by sub-part. A text block (name/label/return)
        // drags that text; the bare arrow line drags the receiving activation
        // up/down. Text wins where the name rect overlaps the arrow area.
        else if (pHit && pHit->IsSignalShape())
        {
            SignalShape*     pSig = pHit->GetSignal();
            const SignalPart part = signalPartAt(pSig, modelPt);
            if (part == SignalPart::Name || part == SignalPart::Label ||
                part == SignalPart::Return)
            {
                _textDragSig            = pSig;
                _textDragPart           = part;
                _textDragStartModelPt   = m;
                _textDragCurrentModelPt = m;
                _pressWidgetPos         = e->position();
                _textDragPotential      = true;
                _textDragActive         = false;
            }
            else if (pHit->GetChildActivation())
            {
                _sigMoveSignal         = pSig;
                _sigMoveStartModelPt   = m;
                _sigMoveCurrentModelPt = m;
                _pressWidgetPos        = e->position();
                _sigMovePotential      = true;
                _sigMoveActive         = false;
            }
        }
        // Press on a note: drag an attach-line point, or move (body) / resize.
        else if (pHit && pHit->GetNoteShape())
        {
            SDNoteShape* pNote      = pHit->GetNoteShape();
            _noteDragNote           = pNote;
            _noteDragPressRect      = pNote->GetRect();
            _noteDragStartModelPt   = m;
            _noteDragCurrentModelPt = m;
            _pressWidgetPos         = e->position();
            if (SDNoteShapePoint* pPt = notePointAt(pNote, modelPt))
            {
                _noteDragPart  = NotePart::Point;
                _noteDragPoint = pPt;
            }
            else
            {
                // Resize handles were already consumed by selectedNoteResizeAt
                // above for a selected note; for a first click this catches the
                // inner-half handle, else Body.
                _noteDragPart  = notePartAt(pNote, modelPt);
                _noteDragPoint = nullptr;
            }
            _noteDragPotential = true;
            _noteDragActive    = false;
        }
        // No hit and no drag setup -> a drag from empty space starts a
        // box-select (the actual rubber-band shows up once the user has
        // moved past the threshold; a pure click without movement just
        // clears, which already happened above for the no-modifier case).
        else if (!pHit)
        {
            _boxSelectPress     = e->position();
            _boxSelectAdditive  = ctrl || shift;
            _boxSelectPotential = true;
            _boxSelectActive    = false;
        }
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void SequenceDiagramCanvas::mouseMoveEvent(QMouseEvent* e)
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
        setCursor(Qt::CrossCursor);
        update();
        e->accept();
        return;
    }
    if (_dragPotential)
    {
        beginDragIfReady(e->position());
        if (_dragActive)
        {
            _dragCurrentModelPt = widgetToModel(e->position());
            update();
            e->accept();
            return;
        }
    }
    if (_msgDragPotential)
    {
        beginMessageDragIfReady(e->position());
        if (_msgDragActive)
        {
            _msgDragCurrentModelPt = widgetToModel(e->position());
            // Cursor tells whether releasing here would make a message: a
            // pointing hand over a valid target (lifeline / callable
            // activation), the no-drop cursor over empty space or an illegal
            // target. Both are native Win32 cursors (DPI-correct on secondary
            // monitors -- see the Qt native-cursor note), unlike the bitmap
            // Drag*Cursor family.
            SequenceDiagramShape* pTgt = nullptr;
            if (_pSD)
            {
                const CbPoint mp(qRound(_msgDragCurrentModelPt.x()),
                                 qRound(_msgDragCurrentModelPt.y()));
                pTgt = hitTest(mp);
            }
            _msgDragOverTarget = canMakeMessage(_msgDragSource, pTgt);
            setCursor(_msgDragOverTarget ? Qt::PointingHandCursor
                                         : Qt::ForbiddenCursor);
            update();
            e->accept();
            return;
        }
    }
    if (_sigMovePotential)
    {
        beginSignalMoveIfReady(e->position());
        if (_sigMoveActive)
        {
            _sigMoveBothEnds = (e->modifiers() & Qt::AltModifier) != 0;
            _sigMoveCurrentModelPt = widgetToModel(e->position());
            update();
            e->accept();
            return;
        }
    }
    if (_textDragPotential)
    {
        beginTextDragIfReady(e->position());
        if (_textDragActive)
        {
            _textDragCurrentModelPt = widgetToModel(e->position());
            update();
            e->accept();
            return;
        }
    }
    if (_noteDragPotential)
    {
        beginNoteDragIfReady(e->position());
        if (_noteDragActive)
        {
            _noteDragCurrentModelPt = widgetToModel(e->position());
            update();
            e->accept();
            return;
        }
    }
    if (_llResizePotential)
    {
        beginClassLifeLineResizeIfReady(e->position());
        if (_llResizeActive)
        {
            _llResizeCurrentModelPt = widgetToModel(e->position());
            update();
            e->accept();
            return;
        }
    }
    if (_boxSelectPotential)
    {
        beginBoxSelectIfReady(e->position());
        if (_boxSelectActive && _rubberBand)
        {
            _rubberBand->setGeometry(
                QRect(_boxSelectPress.toPoint(), e->position().toPoint())
                    .normalized());
            e->accept();
            return;
        }
    }
    // Hover -- no button (or any non-middle button) held. Update the
    // cursor based on what's under the pointer.
    updateHoverCursor(e->position());
    QWidget::mouseMoveEvent(e);
}

void SequenceDiagramCanvas::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        // The mousePressEvent that fired just before us may have set up
        // drag/box-select potential; openShapeAt is about to spin a modal
        // dialog and the release event may not arrive cleanly afterwards.
        // Clear any pending interaction state first.
        _dragItems.clear();
        _dragPotential = false;
        _dragActive    = false;
        cancelBoxSelect();
        cancelMessageDrag();
        cancelSignalMove();
        cancelTextDrag();
        cancelNoteDrag();
        cancelClassLifeLineResize();

        const bool ctrl = (e->modifiers() & Qt::ControlModifier) != 0;
        openShapeAt(e->position(), ctrl);
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

void SequenceDiagramCanvas::mouseReleaseEvent(QMouseEvent* e)
{
    if (_panning && e->button() == Qt::MiddleButton)
    {
        _panning = false;
        unsetCursor();
        // Restore the hover cursor for whatever the mouse is now over.
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_dragActive || _dragPotential))
    {
        if (_dragActive)
            commitDrag();
        _dragItems.clear();
        _dragActive    = false;
        _dragPotential = false;
        unsetCursor();
        updateHoverCursor(e->position());   // back to OpenHand if still over a shape
        update();
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_msgDragActive || _msgDragPotential))
    {
        if (_msgDragActive)
            commitMessageDrag(e->position());   // make the message (if valid target)
        else
            cancelMessageDrag();                // no drag -> just a click/select
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_sigMoveActive || _sigMovePotential))
    {
        if (_sigMoveActive)
        {
            _sigMoveBothEnds = (e->modifiers() & Qt::AltModifier) != 0;
            commitSignalMove();                 // move the chosen activation
        }
        else
            cancelSignalMove();                 // no drag -> just a click/select
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_textDragActive || _textDragPotential))
    {
        if (_textDragActive)
            commitTextDrag();                   // move the text block
        else
            cancelTextDrag();                   // no drag -> just a click/select
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_noteDragActive || _noteDragPotential))
    {
        if (_noteDragActive)
            commitNoteDrag();                   // move / resize the note
        else
            cancelNoteDrag();                   // no drag -> just a click/select
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton && (_llResizeActive || _llResizePotential))
    {
        if (_llResizeActive)
            commitClassLifeLineResize();        // set manual width
        else
            cancelClassLifeLineResize();        // no drag -> just a click/select
        updateHoverCursor(e->position());
        e->accept();
        return;
    }
    if (e->button() == Qt::LeftButton &&
        (_boxSelectActive || _boxSelectPotential))
    {
        if (_boxSelectActive)
            finishBoxSelect(_boxSelectAdditive);
        else
            cancelBoxSelect();           // didn't pass threshold -- treat as click
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}

// Cursor must drop back to default when the mouse leaves the widget --
// otherwise SizeAllCursor leaks out into the rest of the UI.
void SequenceDiagramCanvas::leaveEvent(QEvent* /*event*/)
{
    if (placementActive())
    {
        _placementHasPos = false;   // hide the ghost while outside the canvas
        update();
    }
    if (!_panning)
        unsetCursor();
}

// Keyboard:
//   Esc       -- cancel in-progress drag; otherwise clear selection
//   Del       -- delete each selected shape (mirrors MFC OnDelete)
//   arrows    -- navigate among related shapes (single-selection)
//   Ctrl+L/R  -- swap selected lifeline with neighbour
//   Ctrl+U/D  -- swap selected child-activation with sibling
//   Ctrl+0    -- reset zoom + pan
//   Ctrl++/-  -- zoom in/out around centre
// Ctrl+Shift+<letter> Add* accelerators -- fired from keyPressEvent when the canvas
// has focus (the context-menu items carry the same shortcut for discoverability).
// Gated by the current selection exactly like the menu (pAddBase = the selected
// lifeline's class; pAddArgMethod = the selected activation's method). Class / Lifeline
// / Note are always available. Returns true if it handled the key.
bool SequenceDiagramCanvas::triggerAddShortcut(int key)
{
    SequenceDiagramShape* pSingle = singleSelectedShape();
    BaseClass* pAddBase      = nullptr;
    Method*    pAddArgMethod = nullptr;
    if (pSingle)
    {
        if (ClassLifeLineShape* pCLL = pSingle->GetClassLifeLine())
            pAddBase = pCLL->GetBaseClass();
        if (ChildActivationShape* pCA = pSingle->GetChildActivation())
            pAddArgMethod = pCA->GetMethod();
    }
    switch (key)
    {
    case Qt::Key_L: beginPlacement(PlacementKind::LifeLine); return true;
    case Qt::Key_N: beginPlacement(PlacementKind::Note);     return true;
    case Qt::Key_K:  // Message -- no mnemonic free (M/S/A all tree-reserved); K is a clean free key
        if (canAddMessage()) { addMessage(); return true; }
        return false;
    case Qt::Key_C:
        if (_pSD->GetParent()->OnAddClass(true)) { addClass(); return true; }
        return false;
    case Qt::Key_B:
        if (pAddBase && pAddBase->OnAddMember(true))         { pAddBase->OnAddMember(false);         afterModelAdd(); return true; }
        return false;
    case Qt::Key_M:
        if (pAddBase && pAddBase->OnAddMethod(true))         { pAddBase->OnAddMethod(false);         afterModelAdd(); return true; }
        return false;
    case Qt::Key_U:
        if (pAddBase && pAddBase->OnAddConstructor(true))    { pAddBase->OnAddConstructor(false);    afterModelAdd(); return true; }
        return false;
    case Qt::Key_A:
        if (pAddArgMethod && pAddArgMethod->OnAddArgument(true)) { pAddArgMethod->OnAddArgument(false); afterModelAdd(); return true; }
        return false;
    case Qt::Key_V:
        if (pAddBase && pAddBase->OnAddVirtuals(true))       { pAddBase->OnAddVirtuals(false);       afterModelAdd(); return true; }
        return false;
    case Qt::Key_S:
        if (pAddBase && pAddBase->OnAddIsClassMethods(true)) { pAddBase->OnAddIsClassMethods(false); afterModelAdd(); return true; }
        return false;
    }
    return false;
}

// Select every shape in the diagram (Ctrl+A) -- matches the CD. Replaces the current
// selection; the SequenceDiagramShapeIterator walks all shapes.
void SequenceDiagramCanvas::selectAll()
{
    if (!_pSD || !_pViewModel)
        return;
    _pViewModel->DeleteAllSelected();
    SequenceDiagram::SequenceDiagramShapeIterator iShape(_pSD);
    while (++iShape)
        (void)new SequenceDiagramViewModelSelection(_pViewModel, iShape.Get());
    update();
}

void SequenceDiagramCanvas::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (placementActive())
            cancelPlacement();
        else if (_dragActive || _dragPotential)
            cancelDrag();
        else if (_msgDragActive || _msgDragPotential)
            cancelMessageDrag();
        else if (_sigMoveActive || _sigMovePotential)
            cancelSignalMove();
        else if (_textDragActive || _textDragPotential)
            cancelTextDrag();
        else if (_noteDragActive || _noteDragPotential)
            cancelNoteDrag();
        else if (_llResizeActive || _llResizePotential)
            cancelClassLifeLineResize();
        else if (_boxSelectActive || _boxSelectPotential)
            cancelBoxSelect();
        else
            clearSelection();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace)
    {
        deleteSelected();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
        openSelected((e->modifiers() & Qt::ControlModifier) != 0);
        e->accept();
        return;
    }

    const bool ctrl = (e->modifiers() & Qt::ControlModifier) != 0;

    if (ctrl)
    {
        // Ctrl+Shift+<letter> = Add* accelerators (same scheme as the tree/CD; shown in
        // the context menu), gated by selection. Ctrl+A (no Shift) = Select All. Plain
        // arrows navigate; Ctrl+arrows swap -- both below / in the else branch.
        if ((e->modifiers() & Qt::ShiftModifier) && triggerAddShortcut(e->key()))
        {
            e->accept();
            return;
        }
        switch (e->key())
        {
        case Qt::Key_A:
            selectAll();
            e->accept();
            return;
        case Qt::Key_0:
            resetView();
            e->accept();
            return;
        case Qt::Key_Plus:
        case Qt::Key_Equal:                          // unshifted '+' on US/EU
            zoomAt(1.15, QPointF(width() / 2.0, height() / 2.0));
            e->accept();
            return;
        case Qt::Key_Minus:
            zoomAt(1.0 / 1.15, QPointF(width() / 2.0, height() / 2.0));
            e->accept();
            return;
        case Qt::Key_Left:
            swapLifeLineLeft();  e->accept(); return;
        case Qt::Key_Right:
            swapLifeLineRight(); e->accept(); return;
        case Qt::Key_Up:
            swapChildActivationUp();   e->accept(); return;
        case Qt::Key_Down:
            swapChildActivationDown(); e->accept(); return;
        case Qt::Key_Z:
            undo(); e->accept(); return;
        case Qt::Key_Y:
            redo(); e->accept(); return;
        default:
            break;
        }
    }
    else  // plain arrow -> navigate among related shapes
    {
        switch (e->key())
        {
        case Qt::Key_Left:  navigateLeft();  e->accept(); return;
        case Qt::Key_Right: navigateRight(); e->accept(); return;
        case Qt::Key_Up:    navigateUp();    e->accept(); return;
        case Qt::Key_Down:  navigateDown();  e->accept(); return;
        default:
            break;
        }
    }
    QWidget::keyPressEvent(e);
}

// ---------------------------------------------------------------------------
// SequenceDiagramQtView (window)
// ---------------------------------------------------------------------------
SequenceDiagramQtView::SequenceDiagramQtView(SequenceDiagram* pSD,
                                             QWidget* parent)
    : QDialog(parent)
    , _canvas(new SequenceDiagramCanvas(pSD, this))
{
    setWindowTitle(QString("SD: %1")
                       .arg(pSD ? toQ(pSD->GetName()) : QString()));

    // A roomy default; the user can resize freely.
    resize(900, 700);

    // Toolbar on top; canvas + two scrollbars in a grid below: toolbar
    // (0,0 span 2), canvas (1,0), vert (1,1), horiz (2,0), empty corner
    // (2,1). Same scroll pattern as MFC's CScrollView.
    auto* hbar = new QScrollBar(Qt::Horizontal, this);
    auto* vbar = new QScrollBar(Qt::Vertical,   this);

    // Per-window toolbar (each view carries its own relevant bar): the Add
    // actions first, then zoom.
    // The SD context-menu Add set is exactly Lifeline / Note / Class -- mirror
    // it here (icon-only with model icons + tooltips), then zoom.
    auto* tb = new QToolBar(this);
    tb->setMovable(false);
    tb->setIconSize(QSize(CB_TOOLBAR_ICON_PX, CB_TOOLBAR_ICON_PX));
    tb->setStyleSheet("QToolBar{border:0;spacing:1px;}"
                      "QToolButton{padding:1px;}");
    if (tb->layout())
        tb->layout()->setContentsMargins(0, 0, 0, 0);
    QAction* al = tb->addAction(Qt_ToolBarIcon(TG_ADD_LIFELINE), "Add Lifeline", this,
                                [this] { _canvas->armAddLifeLinePlacement(); });
    al->setToolTip("Add a class lifeline -- click the lifeline row to place it");
    QAction* an = tb->addAction(Qt_ToolBarIcon(TG_ADD_NOTE), "Add Note", this,
                                [this] { _canvas->armAddNotePlacement(); });
    an->setToolTip("Add a note -- click the diagram to place it");
    QAction* ac = tb->addAction(Qt_ToolBarIcon(TG_ADD_CLASS), "Add Class", this,
                                [this] { _canvas->addClassFromToolBar(); });
    ac->setToolTip("Add a new class to the model and place its lifeline");

    // Edit: Delete (selected) + Undo/Redo, exact MFC glyphs at the shared
    // CB_TOOLBAR_ICON_PX -- same height as the Add buttons. Delete greys out with
    // no selection; Undo/Redo grey out per the undo/redo stack (refreshEditActions,
    // driven by the canvas editActionsChanged signal on every model change).
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

    auto* lay = new QGridLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(tb,      0, 0, 1, 2);
    lay->addWidget(_canvas, 1, 0);
    lay->addWidget(vbar,    1, 1);
    lay->addWidget(hbar,    2, 0);
    lay->setRowStretch(1,    1);
    lay->setColumnStretch(0, 1);

    _canvas->bindScrollBars(hbar, vbar);

    connect(_canvas, &SequenceDiagramCanvas::editActionsChanged,
            this, &SequenceDiagramQtView::refreshEditActions);
    refreshEditActions();
}

void SequenceDiagramQtView::refreshEditActions()
{
    if (_deleteAction) _deleteAction->setEnabled(_canvas->hasSelection());
    refreshUndoRedoEnables();
}

// Enable-only refresh of this view's Undo/Redo buttons -- invoked by name from
// the shell on every open view when any view broadcasts a state change, so all
// views of a model agree on undo/redo availability.
void SequenceDiagramQtView::refreshUndoRedoEnables()
{
    if (_undoAction)   _undoAction->setEnabled(_canvas->canEditUndo());
    if (_redoAction)   _redoAction->setEnabled(_canvas->canEditRedo());
}

SequenceDiagramQtView::~SequenceDiagramQtView() = default;

void SequenceDiagramQtView::exportSvg()
{
    QString def = _canvas->diagramName();
    if (def.isEmpty())
        def = "sequencediagram";
    // Qt's own file dialog on every platform -- consistent with the File
    // One door for every CB file dialog (Cb_FileDialogOptions): it picks the
    // backend AND refreshes the icon theme -- see QtDesktopTheme.h.
    QString path = QFileDialog::getSaveFileName(
        this, "Export Diagram as SVG", def + ".svg", "SVG files (*.svg)",
        nullptr, Cb_FileDialogOptions());
    if (path.isEmpty())
        return;
    if (!path.endsWith(".svg", Qt::CaseInsensitive))
        path += ".svg";
    // Tight crop (shapes' bounding rect + margin), not the full page -- a
    // small diagram exported at page extent is mostly whitespace, which is
    // useless when embedding into a document.
    if (!_canvas->exportSvg(path, /*tight=*/true))
        QMessageBox::warning(this, "Export SVG",
            "SVG export failed -- the Qt Svg module may be unavailable in this build.");
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
// Non-modal: the call returns immediately. The window self-destructs on
// close via Qt::WA_DeleteOnClose; multiple Qt views over the same diagram
// (or different diagrams) coexist freely.
void Qt_ShowSequenceDiagramView(SequenceDiagram* pSD, void* ownerHwnd)
{
    Qt_EnsureApplication();

    auto* w = new SequenceDiagramQtView(pSD);
    // Prefer a dockable shell dock (floating by default, tab/dock like a tree);
    // fall back to a standalone top-level window only if there's no shell.
    if (!Qt_HostDiagramDock(w))
    {
        w->setAttribute(Qt::WA_DeleteOnClose, true);
        Qt_ShowModeless(*w, ownerHwnd);
    }
}

// Pipe-API backend (see qt/QtSequenceDiagramView.h): dialog-free SVG export.
// Reuse the diagram's open canvas when present; else open the view through
// the same path as a tree double-click, then export.
bool Qt_ExportSequenceDiagramSvg(SequenceDiagram* pSD, const char* path,
                                  bool tight, int margin)
{
    Qt_EnsureApplication();

    auto findCanvas = [pSD]() -> SequenceDiagramCanvas* {
        const QWidgetList widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
            if (auto* c = dynamic_cast<SequenceDiagramCanvas*>(w))
                if (c->diagram() == pSD)
                    return c;
        return nullptr;
    };

    SequenceDiagramCanvas* pCanvas = findCanvas();
    if (!pCanvas)
    {
        Qt_ShowSequenceDiagramView(pSD, nullptr);
        pCanvas = findCanvas();
    }
    if (!pCanvas)
        return false;
    return pCanvas->exportSvg(QString::fromLocal8Bit(path), tight, margin);
}
