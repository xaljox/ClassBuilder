// qt/SequenceDiagramQtView.h -- the Qt-side SequenceDiagram view.
//
// M2.0: non-modal top-level Qt window. The canvas paints the SequenceDiagram
// through CbPainter_QPainter; interaction (selection, drag, hit-test) lands
// in M2.1+.
//
// The window owns a SequenceDiagramViewModel (a non-serialized CB model
// object representing "one open view of this diagram"). The model registers
// the canvas's static RefreshCanvas() as its refresh callback so model
// updates -- both MFC-initiated and Qt-initiated -- propagate back to the
// canvas as a Qt repaint. Per-view selection + multi-shape drag will hang
// off the same ViewModel in M2.2+.
#pragma once

#include <QDialog>
#include <QPointF>
#include <QVector>
#include <QWidget>

#include "CbGeometry.h"             // CbRect (model rect, MFC-free)
#include "QtDiagramDropTarget.h"    // cross-view drop/ghost interface

QT_BEGIN_NAMESPACE
class QScrollBar;
class QRubberBand;
class QAction;
QT_END_NAMESPACE

class SequenceDiagram;
class SequenceDiagramViewModel;
class SequenceDiagramViewModelSelection;
class SequenceDiagramShape;
class SignalShape;
class ChildActivationShape;
class SDNoteShape;
class SDNoteShapePoint;
class ClassLifeLineShape;
class UndoBase;
class RedoBase;
QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

// The paint surface. Splits out so paintEvent has somewhere natural to live.
class SequenceDiagramCanvas : public QWidget, public DiagramDropTarget
{
    Q_OBJECT
public:
    explicit SequenceDiagramCanvas(SequenceDiagram* pSD,
                                   QWidget* parent = nullptr);
    ~SequenceDiagramCanvas() override;

    // Pair the canvas with externally-laid-out scrollbars; the canvas
    // takes care of syncing their range/value with its pan/zoom state.
    void bindScrollBars(QScrollBar* h, QScrollBar* v);

    // Called by the ViewModel's close callback when the model side tears
    // it down (cascade from SequenceDiagram dtor / document close). The
    // canvas can no longer touch the model -- null the back-pointers and
    // schedule the Qt window close. Public because the static callback
    // dispatches to it.
    void notifyModelGone();

    // Cross-view drop (tree Ctrl-drag): a class -> ClassLifeLineShape, an actor ->
    // ActorLifeLineShape at the cursor's X on the lifeline row (mirrors the MFC
    // CClassBuilderView::OnLButtonUp SD branch). showDropGhost previews a 250 x
    // lifeline-height footprint. Driven by MainTreeQtView's drag.
    bool dropFromTree(Gti* pGti, QPoint globalPos) override;
    bool showDropGhost(Gti* pGti, QPoint globalPos) override;
    void clearDropGhost() override;

    // Toolbar-driven zoom (the QtDiagramZoom bridge): +1 step in / -1 step out
    // anchored at the widget centre, 0 = reset to fit-to-window. Same 1.15 step
    // as the keyboard/wheel zoom.
    void applyToolbarZoom(int op);

    // Toolbar entries, mirroring the context menu's Add items: arm an
    // interactive placement (next left-click drops it), or auto-place a new
    // class lifeline.
    void armAddLifeLinePlacement();
    void armAddNotePlacement();
    void addClassFromToolBar();

    // Toolbar Edit buttons (same actions as the Del key and Ctrl+Z / Ctrl+Y).
    // Delete gates on hasSelection (GetSelectedCount != 0); Undo/Redo gate on the
    // undo/redo STACK (canUndo/canRedo), refreshed via editActionsChanged on every
    // model change (RefreshCanvas).
    void editDeleteSelected() { deleteSelected(); }
    void editUndo()           { undo(); }
    void editRedo()           { redo(); }
    bool hasSelection() const;                       // gates Delete
    bool canEditUndo() const { return canUndo(); }   // gates Undo (stack state)
    bool canEditRedo() const { return canRedo(); }   // gates Redo (stack state)

    // Render this diagram to a standalone .svg at its page extent (vector,
    // selection-free). Returns false if the Qt Svg module is unavailable or
    // the write failed. Same shape Draw as on screen; only the device differs.
    bool exportSvg(const QString& path);
    QString diagramName() const;          // default export file name

signals:
    // Emitted once per coalesced model change (from RefreshCanvas) so the owning
    // view can re-evaluate its toolbar Edit-button enables (selection + stack).
    void editActionsChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    // The static callbacks the ViewModel invokes. Plain C-style fn-pointer
    // signature -- ctx is the canvas-this passed at ViewModel construction.
    // RefreshCanvas: model-side change happened, post a Qt repaint.
    // CloseCanvas:   ViewModel is being destroyed (SD cascade) -- the
    //                canvas must let go of its model pointers and close.
    static void RefreshCanvas(void* ctx);
    static void CloseCanvas(void* ctx);

    // The page-size + fit-to-window transform inputs derived from the model
    // and the current widget size. Used by both paintEvent and the scrollbar
    // sync so the two stay in lock-step.
    struct FitInfo
    {
        qreal pageW;     // model page width  (default A4 if SD has none set)
        qreal pageH;     // model page height
        qreal fitScale;  // fit-to-window scale (single value, aspect preserved)
        qreal originX;   // centering offset in widget coords (>= 0)
        qreal originY;
        qreal scaledW;   // pageW * fitScale * _zoom  (rendered page width)
        qreal scaledH;
    };
    FitInfo computeFit() const;

    // Inverse of the paintEvent transform: widget-coord -> model-coord (the
    // model uses Y-up, so this returns CbPoint-style coords directly usable
    // with SequenceDiagram::GetHitShape / Shape::PointInShape).
    QPointF widgetToModel(QPointF widgetPt) const;

    // GetHitShape wrapper that parks a headless measure painter so a signal's
    // name/label/return TEXT rects are hit-testable off-view (not just its
    // arrow). All canvas hit-tests go through this.
    SequenceDiagramShape* hitTest(const CbPoint& modelPt) const;

    // Hover update: hit-test at the given widget-coord, set the canvas
    // cursor to reflect what a click would do. Pure hover -- no model
    // mutation, no selection change.
    void updateHoverCursor(QPointF widgetPt);

    // Find the existing selection junction for `pShape` in this canvas's
    // ViewModel, or nullptr if `pShape` isn't selected here.
    SequenceDiagramViewModelSelection* findSelection(SequenceDiagramShape* pShape) const;

    // Draw a translucent overlay around every selected shape AFTER the
    // model has been painted -- second-pass highlight per the M2 plan.
    void paintSelectionOverlay(QPainter& qp);

    // Keyboard (M2.6): plain arrows navigate among related shapes (mirrors
    // MFC OnKeyLeft/Right/Up/Down), Ctrl+L/R swaps a lifeline with its
    // neighbour, Ctrl+U/D swaps a child-activation with its sibling, Del
    // removes selected shapes, Esc clears selection (or cancels a drag).
    SequenceDiagramShape* singleSelectedShape() const;
    // Ctrl+Shift+<letter> Add* accelerators (keyPressEvent) -- gated like the context
    // menu; true if it handled the key. Same scheme as the tree/CD. + Ctrl+A select-all.
    bool triggerAddShortcut(int key);
    void selectAll();
    void replaceSelection(SequenceDiagramShape* pNew);
    void navigateLeft();
    void navigateRight();
    void navigateUp();
    void navigateDown();
    void swapLifeLineLeft();
    void swapLifeLineRight();
    void swapChildActivationUp();
    void swapChildActivationDown();
    // Undo/Redo (M7) -- Ctrl+Z / Ctrl+Y. can* mirror the MFC OnUpdateEditUndo/
    // Redo guard (+ never undo this view's own SD); undo/redo apply + repaint.
    bool canUndo() const;   // both defer to the doc's CanUndo/CanRedo (one shared rule)
    bool canRedo() const;
    void undo();
    void redo();
    // Checkable view toggles (mirror OnSequenceDiagramAutowidth /
    // OnShowmethodarguments / OnShowmethodscope). Auto Width acts on the
    // selected class lifeline; the two Show toggles act on the selected
    // child-activation's sender signal.
    void toggleAutoWidth();
    void toggleShowMethodArguments();
    void toggleShowMethodScope();
    // Collapse 2+ child-activations on one lifeline into a single SDNote
    // holding their signal/method names (mirrors OnSDCollapseToNote).
    void collapseToNote();
    // Pop a QColorDialog seeded with the selection's common colour and apply
    // the pick to every selected shape that uses it (mirror
    // OnSequenceDiagramChangecolor / OnSequenceDiagramChangetextcolor).
    void changeLineColor();
    void changeTextColor();
    // Color Templates submenu -- document-level default colour per shape type
    // (mirror the OnSequencediagramChange*Color handlers). Indexes the file-
    // static kTemplateColors[] table in the .cpp.
    void changeTemplateColor(int entryIndex);
    // Common tail after an Add-submenu model command (undo-mark + refresh +
    // raise this window back over the MFC-owned dialog).
    void afterModelAdd();
    // Add Message (port of OnAddMessage) -- selection-based: single lifeline ->
    // root activation; single activation -> nested call (+ SignalDialog); two
    // selected -> message/reparent between them. canAddMessage mirrors
    // OnUpdateAddMessage; twoSelected resolves the first/last selected shapes.
    void addMessage();
    bool canAddMessage() const;
    // Add Class (port of OnEditAddclass) -- shows the add-class dialog
    // (GetParent()->OnAddClass) then auto-places a ClassLifeLineShape to the
    // right of the last lifeline and opens its spec. No mouse tracking.
    void addClass();
    bool twoSelected(SequenceDiagramShape*& pFirst,
                     SequenceDiagramShape*& pLast) const;
    void deleteSelected();
    void clearSelection();

    // Add Note / Lifeline (port of OnAddNote / OnAddLifeline -> SDNoteShape::
    // TrackAdd / LifeLineShape::TrackAdd). MFC runs a modal mouse-capture loop
    // that XOR-draws a rubber-band and places the shape on the next click;
    // that fights the Qt event loop, so the port models it as a non-modal
    // "placement mode": the menu arms _placementKind, the next left-click in
    // the canvas drops the shape (Note then opens its spec dialog; Lifeline
    // shows the actor/class picker on the click, then places), a ghost rect
    // tracks the cursor, and Esc / right-click cancels.
    enum class PlacementKind { None, Note, LifeLine };
    void beginPlacement(PlacementKind kind);
    void finishPlacementAt(QPointF widgetPt);
    void cancelPlacement();
    void paintPlacementGhost(QPainter& qp);
    bool placementActive() const { return _placementKind != PlacementKind::None; }

    // Cross-view drop ghost (tree Ctrl-drag preview); model-space cursor X, painted
    // like the lifeline placement footprint. See showDropGhost / clearDropGhost.
    bool    _dropGhostActive = false;
    QPointF _dropGhostModelPt;
    void    paintDropGhost(QPainter& qp);

    // Open-shape dispatch (M3): hit-test, force-select that shape, then
    // call the shape's own `OnOpen()` (or `OnEditAttributes()` with Ctrl).
    // The shape methods already wire to the right `Qt_Show*Dialog` -- this
    // is pure routing. raiseToFront() pulls this view back above the MFC
    // window after the (MFC-owned) dialog closes.
    void openShapeAt(QPointF widgetPt, bool ctrlHeld);
    void openSelected(bool ctrlHeld);   // Enter key path -- same dispatch on single-selected.
    void raiseToFront();

    // Box-select (M2.5): on left-button drag from empty space, draw a
    // rubber band; on release, select every shape whose bounding rect is
    // fully enclosed by the band (mirrors MFC BoxSelectTrack). With
    // Ctrl/Shift modifier on the initial press, additive; otherwise the
    // selection was already cleared on press (see mousePressEvent).
    void beginBoxSelectIfReady(QPointF widgetPos);
    void finishBoxSelect(bool additive);
    void cancelBoxSelect();

    // Overlay-style drag (M2.3, lifeline kinds only): the model is left
    // untouched during the drag and only mutated once at release via
    // SetRect -- exactly one SaveState per drag, matching MFC's XOR
    // Track* pattern. ESC drops the overlay, no model change.
    void beginDragIfReady(QPointF widgetPos);   // potential -> active
    void commitDrag();                          // release: apply SetRects
    void cancelDrag();                          // ESC: drop, no model change
    void paintDragGhost(QPainter& qp);
    static bool isDraggableKind(SequenceDiagramShape* p);

    // Message-drag: the interactive form of Add Message. Drag FROM an
    // activation (the one shape kind that isn't move-draggable, so there's no
    // collision with lifeline-move / box-select) ONTO a lifeline or another
    // activation to create the call. A dashed connector line tracks the
    // cursor; on release over a valid target, the {source,target} pair is
    // selected and addMessage() runs (reusing all its dialog/undo logic).
    void beginMessageDragIfReady(QPointF widgetPos);   // potential -> active
    void commitMessageDrag(QPointF widgetPos);         // release: make message
    void cancelMessageDrag();                          // ESC / invalid: drop
    void paintMessageDragLine(QPainter& qp);
    bool canMakeMessage(SequenceDiagramShape* pSource,
                        SequenceDiagramShape* pTarget) const;
    // Drag form of message creation: source + target passed directly (NOT via
    // the selection), so the lifeline drop target is never selected/highlighted.
    // Selects only the resulting message on success.
    void createMessageFromDrag(SequenceDiagramShape* pSource,
                               SequenceDiagramShape* pTarget);

    // Signal vertical move: drag a message ARROW up/down to reposition its
    // RECEIVING activation (same model effect as MFC's activation drag, but
    // grabbed via the message -- so the activation-grab stays free for the
    // message-create gesture). Vertical only, clamped by GetMaxUpOffset; the
    // model is touched once on release (overlay-style, like the lifeline drag).
    void beginSignalMoveIfReady(QPointF widgetPos);   // potential -> active
    void commitSignalMove();                          // release: apply offset
    void cancelSignalMove();                          // ESC: drop, no change
    void paintSignalMoveGhost(QPainter& qp);
    int  signalMoveOffsetY() const;   // clamped vertical model delta for the drag
    // Which activation the message-arrow drag moves: the RECEIVING activation
    // normally, the SENDING one when Alt is held (it just picks which end; the
    // layout cascade shifts everything below the chosen activation either way).
    // NB the visible top-level activation bars ARE ChildActivationShapes and move
    // fine; the only non-movable thing is the invisible RootActivationShape
    // container (GetOffset()==0, no Track) -- which never has a grabbable arrow,
    // so the receiver fallback for that case is just a safety net.
    ChildActivationShape* signalMoveTarget() const;

    // Signal text sub-part drag: a press on a name/label/return TEXT block drags
    // that block (free move of its own offset), distinct from a press on the
    // arrow line (which does the activation move above). Text takes PRIORITY
    // where the name rect overlaps the arrow active area (matches MFC Track,
    // whose bare arrow line isn't draggable). signalPartAt resolves which part a
    // model point hits.
    enum class SignalPart { None, Name, Label, Return, Arrow };
    SignalPart signalPartAt(SignalShape* pSig, const CbPoint& modelPt) const;
    CbRect     textDragRect() const;                  // dragged sub-part's rect
    void beginTextDragIfReady(QPointF widgetPos);     // potential -> active
    void commitTextDrag();                            // release: apply offset
    void cancelTextDrag();                            // ESC: drop, no change
    void paintTextDragGhost(QPainter& qp);

    // Note sub-part drag: move the whole note (body) or resize via the left /
    // right edge handles. Overlay-style (model touched once on release via
    // SetRect). Attach-line point drag comes in a follow-up.
    enum class NotePart { None, Body, ResizeLeft, ResizeRight, Point };
    NotePart notePartAt(SDNoteShape* pNote, const CbPoint& modelPt) const;
    // A SELECTED note whose left/right resize handle is under modelPt -- checked
    // directly (not via GetHitShape) because the handles straddle the note edge
    // and their outer half is OUTSIDE the rect (so GetHitShape would miss it).
    SDNoteShape* selectedNoteResizeAt(const CbPoint& modelPt, NotePart& part) const;
    // The attach-line point of `pNote` under modelPt, or null. Dragging it calls
    // SDNoteShapePoint::SetPoint, which itself spawns a new corner point (drag
    // the corner out) or deletes the point (drag it back inside) -- so the
    // canvas just moves the grabbed point.
    SDNoteShapePoint* notePointAt(SDNoteShape* pNote, const CbPoint& modelPt) const;
    // A SELECTED note's end-point under modelPt (point-beats-line + selected-
    // shape priority). Checked ahead of hitTest in cursor + press so a point
    // sitting ON a signal arrow isn't stolen by the signal's up/down move.
    SDNoteShapePoint* selectedNotePointAt(const CbPoint& modelPt,
                                          SDNoteShape*& pNoteOut) const;
    // Snap a note point's Y exactly onto a nearby signal arrow line; X is left
    // as-is. No-op if no signal line is within `tol` model units. With arrow Y's
    // now grid-aligned (ParentActivationShape::RecalculateRect), this is just
    // cheap insurance for slanted arrows -- normal points already coincide.
    CbPoint snapNotePointToSignal(CbPoint p, int tol) const;
    CbRect   noteDragRect() const;                    // would-be rect for the drag
    void beginNoteDragIfReady(QPointF widgetPos);
    void commitNoteDrag();
    void cancelNoteDrag();
    void paintNoteDragGhost(QPainter& qp);

    // Class-lifeline resize: drag the left/right edge handle of a SELECTED class
    // lifeline to set a manual width (commit turns off auto-width). Same pattern
    // as the note resize; the lifeline body stays move-draggable (horizontal).
    bool classLifeLineHandleAt(ClassLifeLineShape* pLL, const CbPoint& modelPt,
                               bool& right) const;
    // A SELECTED class lifeline whose L/R handle is under modelPt -- checked
    // directly (handles straddle the edge, outer half outside the rect, so
    // GetHitShape would miss them), exactly like selectedNoteResizeAt.
    ClassLifeLineShape* selectedClassLifeLineResizeAt(const CbPoint& modelPt,
                                                      bool& right) const;
    CbRect classLifeLineResizeRect() const;           // would-be rect for the drag
    void beginClassLifeLineResizeIfReady(QPointF widgetPos);
    void commitClassLifeLineResize();
    void cancelClassLifeLineResize();
    void paintClassLifeLineResizeGhost(QPainter& qp);

    // Apply a zoom step (factor>1 zooms in, <1 zooms out) anchored at the
    // given widget-coord point so it stays put under the cursor.
    void zoomAt(qreal factor, QPointF anchor);
    // Reset zoom + pan -- back to pure fit-to-window.
    void resetView();

    // Push current state into the bound scrollbars; bidirectional with the
    // _ignoreScrollSignals latch so we don't echo our own writes back as
    // user-driven pan changes.
    void updateScrollBars();
    void onScrollH(int v);
    void onScrollV(int v);

    SequenceDiagram*          _pSD;
    SequenceDiagramViewModel* _pViewModel;

    // Selection signature (count + single-selected shape ptr); paintEvent emits
    // editActionsChanged when it changes so the toolbar's Delete enable tracks
    // selection (the model-change RefreshCanvas emit alone misses selection-only
    // changes). ~0 forces a first emit.
    quintptr _editSelSig = ~quintptr(0);

    // User zoom / pan, composed on top of the fit-to-window transform.
    // Defaults (_zoom=1, _pan=0,0) reproduce the original fit-to-window
    // behaviour exactly -- the leading translate/scale become no-ops.
    qreal   _zoom        = 1.0;
    QPointF _pan;
    // Middle-button drag state.
    bool    _panning     = false;
    QPointF _lastPanPos;

    QScrollBar* _hbar    = nullptr;
    QScrollBar* _vbar    = nullptr;
    bool _ignoreScrollSignals = false;

    // Set in the canvas dtor BEFORE delete _pViewModel, so the cascade
    // close-callback fired from ~SequenceDiagramViewModel sees we're
    // already going down and no-ops.
    bool _destructing = false;

    // Drag state.
    struct DragItem
    {
        SequenceDiagramShape* shape;
        CbRect                originalRect;
    };
    QVector<DragItem> _dragItems;
    QPointF _pressWidgetPos;
    QPointF _pressModelPt;
    QPointF _dragCurrentModelPt;
    bool    _dragPotential = false;   // press on draggable; not yet moved
    bool    _dragActive    = false;   // movement threshold crossed

    // Box-select state. _boxSelectAdditive captures the modifier state at
    // press time -- the user can't change their mind mid-drag.
    QRubberBand* _rubberBand        = nullptr;
    QPointF      _boxSelectPress;     // widget coords
    bool         _boxSelectPotential = false;
    bool         _boxSelectActive    = false;
    bool         _boxSelectAdditive  = false;

    // Placement mode (Add Note / Lifeline). _placementModelPt follows the
    // cursor for the ghost; _placementHasPos gates it until the first move.
    PlacementKind _placementKind    = PlacementKind::None;
    QPointF       _placementModelPt;
    bool          _placementHasPos  = false;

    // Message-drag state. The source is the pressed activation; the connector
    // line runs from where it was grabbed to the current cursor (model coords).
    SequenceDiagramShape* _msgDragSource = nullptr;
    QPointF _msgDragStartModelPt;
    QPointF _msgDragCurrentModelPt;
    bool    _msgDragPotential = false;   // press on activation; not yet moved
    bool    _msgDragActive    = false;   // movement threshold crossed
    bool    _msgDragOverTarget = false;  // cursor currently over a valid target

    // Signal vertical-move state. The grabbed message arrow; its receiving
    // activation is re-fetched via _sigMoveSignal->GetChildActivation().
    SignalShape* _sigMoveSignal = nullptr;
    QPointF _sigMoveStartModelPt;
    QPointF _sigMoveCurrentModelPt;
    bool    _sigMovePotential = false;   // press on a signal arrow; not yet moved
    bool    _sigMoveActive    = false;   // movement threshold crossed
    bool    _sigMoveBothEnds  = false;   // Alt held -> move the SENDING activation

    // Signal text-drag state. The grabbed signal + which sub-part; free move.
    SignalShape* _textDragSig  = nullptr;
    SignalPart   _textDragPart = SignalPart::None;
    QPointF _textDragStartModelPt;
    QPointF _textDragCurrentModelPt;
    bool    _textDragPotential = false;
    bool    _textDragActive    = false;

    // Note-drag state. _noteDragPressRect is the note's rect at press time.
    SDNoteShape*      _noteDragNote  = nullptr;
    NotePart          _noteDragPart  = NotePart::None;
    SDNoteShapePoint* _noteDragPoint = nullptr;   // grabbed attach-line point
    CbRect            _noteDragPressRect;
    QPointF _noteDragStartModelPt;
    QPointF _noteDragCurrentModelPt;
    bool    _noteDragPotential = false;
    bool    _noteDragActive    = false;

    // Class-lifeline resize state. _llResizeRight picks the dragged edge.
    ClassLifeLineShape* _llResizeShape = nullptr;
    bool    _llResizeRight = false;
    CbRect  _llResizePressRect;
    QPointF _llResizeStartModelPt;
    QPointF _llResizeCurrentModelPt;
    bool    _llResizePotential = false;
    bool    _llResizeActive    = false;
};

// The hosting window. Top-level, non-modal, self-destructs on close (the
// owner sets Qt::WA_DeleteOnClose via Qt_ShowModeless / the Qt_Show entry).
class SequenceDiagramQtView : public QDialog
{
    Q_OBJECT
public:
    explicit SequenceDiagramQtView(SequenceDiagram* pSD,
                                   QWidget* parent = nullptr);
    ~SequenceDiagramQtView();

public slots:
    // Enable-only refresh of this view's Undo/Redo buttons; the shell invokes it
    // by name on every open view when any view broadcasts a state change, so all
    // views of a model agree on undo/redo availability (see MainTreeQtView).
    void refreshUndoRedoEnables();

protected:
    // Registers the canvas as the Zoom-toolbar target on window activation
    // (see qt/QtDiagramZoom.h).
    bool event(QEvent* e) override;

private:
    void exportSvg();           // file dialog + canvas->exportSvg
    void refreshEditActions();  // Delete (selection) + Undo/Redo (stack) enables

    SequenceDiagramCanvas* _canvas;
    QAction* _deleteAction = nullptr;
    QAction* _undoAction   = nullptr;
    QAction* _redoAction   = nullptr;
};
