// qt/ClassDiagramQtView.h -- the Qt-side ClassDiagram view.
//
// M1: non-modal top-level window, READ-ONLY. The canvas paints the ClassDiagram
// through CbPainter_QPainter (fit-to-window) and owns a ClassDiagramViewModel --
// a non-serialized model object representing "one open Qt view of this diagram".
// The model registers the canvas's static RefreshCanvas() as its refresh
// callback, so model updates (MFC-initiated edits routed through
// ClassDiagram::UpdateClassDiagramViews) propagate back as a Qt repaint.
// Selection / drag / double-click-to-dialog land in M2+.
//
// Read-only subset of qt/SequenceDiagramQtView.h.
#pragma once

#include <QDialog>
#include <QLineF>
#include <QList>
#include <QPair>
#include <QPointF>
#include <QPolygonF>
#include <QVector>
#include <QWidget>

class QAction;

#include "CbGeometry.h"   // CbRect (model rect, MFC-free) -- drag press-rect member
#include "QtDiagramDropTarget.h"   // cross-view drop/ghost interface

class Gti;
class ClassDiagram;
class ClassDiagramViewModel;
class ClassDiagramViewModelSelection;
class ClassDiagramShape;
class ClassShape;
class NoteShape;
class NoteShapePoint;
class ConnectionShape;
class ConnectionSegment;
class CbPainter;
class UndoBase;
class RedoBase;
QT_BEGIN_NAMESPACE
class QRubberBand;
class QPainter;
class QContextMenuEvent;
class QScrollBar;
class QToolButton;
QT_END_NAMESPACE

// The paint surface.
class ClassDiagramCanvas : public QWidget, public DiagramDropTarget
{
    Q_OBJECT
public:
    explicit ClassDiagramCanvas(ClassDiagram* pClassDiagram,
                                QWidget* parent = nullptr);
    ~ClassDiagramCanvas() override;

    // Called by the ViewModel's close callback when the model side tears it
    // down (cascade from ClassDiagram dtor / document close). Public because
    // the static callback dispatches to it.
    void notifyModelGone();

    // Cross-view drop: a class Ctrl-dragged from the main tree and released over
    // this canvas is added to the diagram as a ClassShape at the drop point
    // (mirrors the MFC CClassBuilderView::OnLButtonUp diagram branch). Returns
    // false if pGti can't land here (DropTarget) -- the caller then falls back to
    // the in-tree drop. Public: invoked by the tree view's drop dispatch.
    bool dropFromTree(Gti* pGti, QPoint globalPos) override;

    // Footprint ghost while a class is Ctrl-dragged over this canvas (before the
    // drop): showDropGhost validates DropTarget and previews where the ClassShape
    // would land (250x40, like the Class placement ghost); clearDropGhost removes
    // it. Driven by the tree view's drag (MainTreeQtView::updateDragTarget).
    bool showDropGhost(Gti* pGti, QPoint globalPos) override;
    void clearDropGhost() override;

    // Toolbar-driven zoom (the QtDiagramZoom bridge): +1 step in / -1 step out
    // anchored at the widget centre, 0 = reset to fit-to-window. Same 1.15 step
    // as the keyboard/wheel zoom.
    void applyToolbarZoom(int op);

    // Toolbar Add actions -- the SAME set and handlers as the context menu's
    // "Add" submenu. runAddItem performs; addItemEnabled is the menu's gate
    // (so the toolbar greys out what the menu would). selectionChanged() lets
    // the owning view refresh the button enables live.
    enum AddItem {
        AddClass, AddNote, AddInheritance, AddRelation,
        AddRelationDiagramOnly, AddDependency,
        AddMember, AddMethod, AddConstructor, AddArgument,
        AddVirtualMethods, AddIsClassMethods
    };
    void runAddItem(int item);
    bool addItemEnabled(int item);

    // Render this diagram to a standalone .svg (vector, selection-free).
    // Returns false if the Qt Svg module is unavailable or the write failed.
    // The shapes paint through CbPainter_QPainter exactly as on screen --
    // only the paint device differs (QSvgGenerator vs the widget). tight=true
    // crops to GetBoundingRect() of the diagram's shapes, inflated by margin
    // model-units on every side, instead of the diagram's fixed page extent
    // -- a small diagram at page extent is mostly whitespace, useless when
    // embedding into a document. The toolbar's Export SVG button always asks
    // for tight=true; the pipe API's export_diagram_svg defaults to
    // tight=false (page extent) but can opt in.
    bool exportSvg(const QString& path, bool tight = false, int margin = 50);
    QString diagramName() const;          // default export file name
    ClassDiagram* diagram() const { return _pCD; }   // identity (pipe SVG export)

    // Invoked by the owning view's toolbar buttons (same actions as the Del key
    // and Ctrl+Z / Ctrl+Y). undo()/redo() self-guard on canUndo()/canRedo(), so
    // the toolbar buttons can stay always-enabled (no stale-disable). hasSelection
    // gates the Delete button.
    void editDeleteSelected() { deleteSelected(); }
    void editUndo()           { undo(); }
    void editRedo()           { redo(); }
    bool hasSelection() const;            // gates Delete
    bool canEditUndo() const { return canUndo(); }   // gates Undo (stack state)
    bool canEditRedo() const { return canRedo(); }   // gates Redo (stack state)

    // Align selected shapes to the last-selected (anchor) Class/Note. The six
    // options appear both in the context-menu "Align" submenu and the CD
    // toolbar's Align dropdown, so AlignKind + alignSelected are public.
    // ClassDiagramShape::Align{Left..Bottom}; needs >=2 alignable shapes
    // (canAlign), which also gates the toolbar button.
    enum class AlignKind { Left, Center, Right, Top, Middle, Bottom };
    void alignSelected(AlignKind which);
    bool canAlign() const;                // alignableSelectedCount() >= 2

    // Pair the canvas with externally-laid-out scrollbars (like the SD canvas);
    // the canvas syncs their range/value with its pan/zoom and pans on drag.
    void bindScrollBars(QScrollBar* h, QScrollBar* v);

signals:
    void selectionChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;            // trackpad pinch -> zoom (macOS native gesture)
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    // Static callbacks the ViewModel invokes (ctx == this).
    static void RefreshCanvas(void* ctx);   // model changed -> post a repaint
    static void CloseCanvas(void* ctx);      // ViewModel destroyed -> let go + close

    // Page-size + fit-to-window transform inputs.
    struct FitInfo
    {
        qreal pageW;     // model page width  (A4 default if the diagram has none)
        qreal pageH;     // model page height
        qreal fitScale;  // fit-to-window scale (aspect preserved)
        qreal originX;   // centering offset in widget coords (>= 0)
        qreal originY;
    };
    FitInfo computeFit() const;

    // Ctrl+wheel zoom anchored at the cursor; middle-drag pan; left double-click
    // resets to fit-to-window. Mirrors the SequenceDiagram canvas gestures.
    void zoomAt(qreal factor, QPointF anchor);
    void resetView();

    // Scrollbar sync (mirrors the SD canvas): range/value from the current
    // fit/zoom/pan; dragging a bar pans. Same Y-up convention as the SD.
    void updateScrollBars();
    void onScrollH(int v);
    void onScrollV(int v);

    // M2 selection (per-Qt-view, via ClassDiagramViewModelSelection): hit-test,
    // click / Ctrl-toggle / Shift-add, box-select, and a highlight overlay.
    // Mirrors the SequenceDiagram canvas. No geometry mutation here.
    QPointF widgetToModel(QPointF widgetPt) const;
    ClassDiagramShape* hitTest(QPointF modelPt) const;
    ClassDiagramViewModelSelection* findSelection(ClassDiagramShape* pShape) const;
    void clearSelection();
    void selectAll();                 // Ctrl+A: select all top-level shapes

    // Drag-to-create: hold R / I / D / O and drag from one class to another to
    // create a Relation / Inheritance / Dependency / diagram-only-Relation between
    // them (mirrors the SD message-drag idea). D/O reuse the create-on-OK bridges;
    // R/I mirror CClassDiagramView::OnEditAddrelation/OnEditAddinherit (new model
    // object -> OnEditAttributes -> Add()/RollBack). The key arms the mode (cross
    // cursor); the drag's source/target classes are the connection endpoints.
    enum class CreateKind { None, Relation, Inherit, Dependency, DiagramOnly };
    CreateKind  _createKey        = CreateKind::None;   // armed by a held R/I/D/O key
    bool        _createDragActive = false;
    ClassShape* _createSource     = nullptr;
    QPointF     _createCurrentModelPt;
    void createConnectionByDrag(CreateKind kind, ClassShape* from, ClassShape* to);
    void paintCreateDragGhost(QPainter& qp);
    // While a create dialog (R/I) is modal the R/I key is still physically held;
    // its auto-repeat would type into the dialog's name field. An app-level event
    // filter, on only for the dialog's lifetime, swallows those auto-repeats.
    bool _swallowCreateAutoRepeat = false;

    // M4 context menu (right-click) -- Open / Edit Attributes / Delete, per-shape
    // Line/Text colour, and the document-level Colour Templates submenu. Routes
    // to the shapes' own OnOpen/OnEditAttributes/OnDelete + DataModelDoc setters.
    ClassDiagramShape* singleSelectedShape() const;
    // Ctrl+Shift+<letter> Add* accelerators (keyPressEvent) -- gated like the context
    // menu; returns true if it handled the key. See docs/Accelerator_Audit.md.
    bool triggerAddShortcut(int key);
    // Up/Down within-class row navigation (header <-> members <-> methods); true if
    // it moved/consumed (a class row was selected). See docs/Accelerator_Audit.md.
    bool navigateClassRow(bool down);
    void openSelected(bool ctrlHeld);
    void deleteSelected();
    void changeLineColor();
    void changeTextColor();
    void changeTemplateColor(int entryIndex);

    // "Add" submenu -- create a diagram-only connection (Relation Diagram Only /
    // Dependency). The shape is built ONLY on the dialog's OK (its create mode),
    // so Cancel adds nothing -- no transient shape, no rollback. The 1-2 class
    // selection just pre-seeds the dialog's From/To; always available otherwise.
    // (The bare model "Relation"/"Inherit" adds are a separate model-level path,
    // not yet ported.)
    enum class AddConnectionKind { DiagramOnlyRelation, Dependency };
    bool selectedFromToClassShapes(ClassShape*& pFrom, ClassShape*& pTo) const;
    void addConnectionFromSelection(AddConnectionKind kind);

    // View toggles -- mirror the MFC CD menu (CClassDiagramView::OnClassdiagram*).
    // The first three act on the single selected shape (ClassShape/RelationShape);
    // Hide/ShowHidden act on the selected connections / the diagram's Hidden list.
    void toggleAutoWidth();           // ClassShape::SetAutoWidth
    void toggleShowMethodArguments(); // ClassShape::SetVerbosity (0/1)
    void toggleShowRelationNames();   // RelationShape::SetVerbosity (0/1)
    void hideSelectedConnections();   // ConnectionShape::Hide on each selected
    void showHiddenConnections();     // ConnectionShape::Show via HiddenIterator

    // Select Classes / Select Members&Methods -- mirror the MFC handlers; both
    // drive existing Qt bridge dialogs. Select Classes auto-places (Grid) when
    // the diagram was empty; Members&Methods is per-ClassShape or diagram-wide.
    void selectClasses();
    void selectMembersAndMethods();

    // Layout / navigation -- mirror the remaining MFC CD handlers. All drive
    // model methods or existing Qt bridge dialogs.
    //   optimizePlacement   -> Grid::Place (whole diagram; needs >=2 class shapes)
    //   reorderMembersMethods -> Qt_ShowClassShapeOrderDialog (single class)
    //   inheritsFrom / inheritedBy -> Qt_Show{InheritsFrom,InheritedBy}Dialog (read-only)
    // (MFC "New Sub Window" opens a new MFC tree-view MDI child -- no Qt analogue,
    //  intentionally omitted; revisit at the all-Qt end state.)
    // (alignSelected / AlignKind / canAlign are public above -- toolbar + menu.)
    void optimizePlacement();
    void reorderMembersMethods();
    void inheritsFrom();
    void inheritedBy();

    // Add submenu -- model-level adds (mirror CClassDiagramView::OnEditAdd*). Each
    // opens the Qt edit dialog via the Gti's OnAddXxx (rolls back on cancel) and
    // creates the diagram shapes for the newly-appended members/methods. Bare
    // Relation/Inheritance create from a SELECTED class (1 -> self, 2 -> from/to).
    void addMember();
    void addMethod();
    void addConstructor();
    void addArgument();
    void addVirtualMethods();
    void addIsClassMethods();
    void addBareRelation();
    void addBareInheritance();
    // Edit items (single selection): user sections (Class), context (any Gti),
    // exception specification (Method). Dialogs open via the model.
    void editUserSections();
    void editContext();
    void editExceptionSpecification();
    // The last-selected alignable shape (MFC GetLastSelected anchor) + how many
    // alignable shapes are selected (the Align submenu's enable gate).
    ClassDiagramShape* lastSelectedAlignShape() const;
    int                alignableSelectedCount() const;

    // Hover-cursor feedback (mirrors the SD canvas's updateHoverCursor) over a
    // selected shape's handles -- the affordance for the upcoming drag. No
    // body->move cursor yet (drag-move isn't implemented -- don't over-promise).
    //   class/note L/R resize handle            -> SizeHor
    //   connection middle-segment handle        -> SizeHor (vertical seg) /
    //                                               SizeVer (horizontal seg)
    //   connection endpoint (first/last segment) -> SizeAll (slides round the
    //                                               class perimeter)
    void updateHoverCursor(QPointF widgetPt);
    bool resolveHandleCursor(QPointF modelPt, Qt::CursorShape& shapeOut) const;

    // Class/Note move + resize drag -- the gesture the hover cursors advertise.
    // Mirrors the SD note/lifeline drag (commitNoteDrag / commitClassLifeLineResize):
    // the model is UNTOUCHED during the drag; a dashed ghost previews the would-be
    // rect; commit on release is exactly ONE undo step (ClassShape/NoteShape::SetRect
    // self-SaveStates AND reroutes attached connections, MarkLastUndo closes it).
    // A manual class resize turns AutoWidth off (like the SD lifeline). Only class +
    // note here; connection-segment drag is the next slice.
    enum class ShapeDragPart { Body, ResizeLeft, ResizeRight };
    bool selectedShapeResizeAt(QPointF modelPt, ShapeDragPart& partOut,
                               ClassDiagramShape*& shapeOut) const;
    CbRect shapeDragRect() const;
    void beginShapeDragIfReady(QPointF widgetPos);
    void commitShapeDrag();
    void cancelShapeDrag();
    void paintShapeDragGhost(QPainter& qp);

    // Connection middle-segment + note connector-point drag -- the "interior"
    // route/anchor edits, same potential->active->ghost->commit-on-release pattern
    // (model untouched until release, one undo step). A middle segment moves
    // PERPENDICULAR via ConnectionSegment::Move (self-SaveStates its neighbours); a
    // note point moves freely via NoteShapePoint::SetPoint (self-SaveStates, spawns/
    // deletes corners). The connection ENDPOINT slide (first/last handle, round the
    // class perimeter) is a separate later slice -- its handle is not armed here.
    enum class HandleDragKind { NotePoint, ConnSegment, ConnEndpoint };
    bool selectedHandleAt(QPointF modelPt);
    void beginHandleDragIfReady(QPointF widgetPos);
    void commitHandleDrag();
    void cancelHandleDrag();
    void paintHandleDragGhost(QPainter& qp);
    // ConnSegment is a GHOST drag (model untouched, real segment stays solid, the
    // dashed preview shows the new spot) -- so the attached note line never looks
    // detached and everything commits together on release. connSegmentClampedDelta
    // is the grid-snapped perpendicular move clamped to the legal reach;
    // canMoveConnSegment is a non-mutating replica of ConnectionSegment::Move's
    // accept test (the min-distance-from-perimeter limit) used to do the clamp.
    CbSize connSegmentClampedDelta() const;   // clamps via ConnectionSegment::CanMove (model)
    void   drawConnSegmentGhost(QPainter& qp, ConnectionSegment* seg, CbSize delta) const;
    // "Move all" (Alt held at release): every collinear segment of a sibling
    // connection from the same From class moves with the dragged one (common with
    // inheritance, where many connections share a trunk). collinearSiblingSegments
    // finds them (coincident routing prefix up to the dragged segment);
    // moveAllModifierActive polls the live Alt state for the preview.
    QVector<ConnectionSegment*> collinearSiblingSegments() const;
    bool   moveAllModifierActive() const;
    // Endpoint-slide ghost: to preview the FULL rerouted path (not just a leg) we
    // briefly apply SlideEndpoint on the real connection, collect its segment points,
    // then RollBack so the model is untouched -- the collected polyline is the dashed
    // ghost while the original connection stays solid (mirrors MFC's temp-clone XOR).
    void   computeEndpointGhost();

    // Ctrl+Z / Ctrl+Y -- drive the document undo/redo from inside the Qt window
    // (DataModelDoc::Undo/Redo). canUndo/canRedo defer to the doc's CanUndo/CanRedo
    // -- the single, simple "is there a step on the stack" rule shared by all views.
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void paintSelectionOverlay(QPainter& qp, CbPainter& painter);
    void beginBoxSelectIfReady(QPointF widgetPos);
    void finishBoxSelect(bool additive);
    void cancelBoxSelect();

    // M3: double-click opens the hit shape's dialog (Ctrl = edit attributes),
    // routing to the shape's own OnOpen / OnEditAttributes (ported Qt dialogs).
    void openShapeAt(QPointF widgetPt, bool ctrlHeld);
    void raiseToFront();

    ClassDiagram*          _pCD;
    ClassDiagramViewModel* _pViewModel;

    // User zoom / pan, composed on top of the fit-to-window transform. Defaults
    // (_zoom=1, _pan=0,0) reproduce plain fit-to-window exactly.
    qreal   _zoom    = 1.0;
    QPointF _pan;
    bool    _panning = false;   // middle-button drag in progress
    QPointF _lastPanPos;

    // Externally-laid-out scrollbars (see bindScrollBars), synced to pan/zoom.
    QScrollBar* _hbar = nullptr;
    QScrollBar* _vbar = nullptr;
    bool        _ignoreScrollSignals = false;   // re-entrancy guard on programmatic value set

    // Box-select state (left-drag from empty space). _boxSelectAdditive captures
    // the modifier at press; the band selects shapes fully enclosed on release.
    QRubberBand* _rubberBand        = nullptr;
    QPointF      _boxSelectPress;
    bool         _boxSelectPotential = false;
    bool         _boxSelectActive    = false;
    bool         _boxSelectAdditive  = false;

    // Class/Note drag state. _shapeDragPressRect is the shape's rect at press time
    // (ref for the move offset / the un-dragged edge of a resize). potential/active
    // = armed-on-press / threshold-crossed (4px), same dual-flag as the SD drags.
    ClassDiagramShape* _shapeDragShape    = nullptr;   // a ClassShape or NoteShape
    ShapeDragPart      _shapeDragPart      = ShapeDragPart::Body;
    CbRect             _shapeDragPressRect;
    QPointF            _shapeDragPressWidgetPos;
    QPointF            _shapeDragStartModelPt;
    QPointF            _shapeDragCurrentModelPt;
    bool               _shapeDragPotential = false;
    bool               _shapeDragActive    = false;

    // Connection middle-segment / note connector-point drag state. The reference
    // for the move is the grabbed handle's ORIGINAL position (model untouched
    // during the drag), so no press-model-point is stored.
    HandleDragKind     _handleDragKind      = HandleDragKind::NotePoint;
    NoteShape*         _handleDragNote      = nullptr;   // NotePoint: owning note
    NoteShapePoint*    _handleDragPoint     = nullptr;   // NotePoint: grabbed point
    ConnectionShape*   _handleDragConn      = nullptr;   // ConnSegment/ConnEndpoint: owning conn
    ConnectionSegment* _handleDragSeg       = nullptr;   // ConnSegment: grabbed seg
    bool               _handleDragAtStart   = false;     // ConnEndpoint: start vs end
    // ConnEndpoint ghost: the rerouted connection's actual Draw output (lines +
    // decoration polygons), captured via a recording CbPainter and replayed dashed
    // so the moved endpoint's arrow/triangle/diamond follows (the polyline alone
    // dropped them).
    QVector<QLineF>    _endpointGhostLines;
    QVector<QPolygonF> _endpointGhostPolys;
    CbPoint            _endpointGhostTarget;            // snapped pt the ghost is built for
    bool               _endpointGhostValid  = false;   // throttle: skip rebuild if target unchanged
    QPointF            _handleDragPressWidgetPos;
    QPointF            _handleDragCurrentModelPt;
    bool               _handleDragPotential = false;
    bool               _handleDragActive    = false;

    // Connection text drag -- relocate a connection's text label (relation from/to
    // name & cardinality, dependency name & stereotype, diagram-only from/to name &
    // cardinality). Mirrors the SD signal-text drag (_textDrag* in the SD canvas):
    // only on a SELECTED connection, hover over its text -> SizeAll cursor, drag ->
    // dashed-rect ghost -> commit on release. Rects are measured headless via
    // CbPainter_QFontMetrics (same font/metrics the model Draw paints with), so the
    // hit/ghost rects match the painted text. The move commits through the model's
    // own Set*Point setter (+ SaveState + SetInitial(false)), exactly like the MFC
    // RelationShape/DependencyShape/RelationDiagramOnlyShape Track* methods.
    enum class ConnTextPart { None, FromName, ToName, FromUml, ToUml,
                              DepName, DepStereotype };
    ClassDiagramShape* _connTextShape = nullptr;   // the dragged connection shape
    ConnTextPart       _connTextPart  = ConnTextPart::None;
    QPointF            _connTextStartModelPt;
    QPointF            _connTextCurrentModelPt;
    QPointF            _connTextPressWidgetPos;
    bool               _connTextPotential = false;
    bool               _connTextActive    = false;
    CbRect       connTextRect(ClassDiagramShape* p, ConnTextPart part) const;
    ConnTextPart connTextPartAt(ClassDiagramShape* p, const CbPoint& modelPt) const;
    bool selectedConnTextAt(QPointF modelPt, ClassDiagramShape*& shapeOut,
                            ConnTextPart& partOut) const;
    void beginConnTextDragIfReady(QPointF widgetPos);
    void commitConnTextDrag();
    void cancelConnTextDrag();
    void paintConnTextDragGhost(QPainter& qp);

    // Member/method in-class reorder drag -- drag a selected MemberShape/MethodShape
    // up/down within its class box to change its order. An insertion LINE (not a box)
    // ghost shows the landing slot; nothing moves until release, then
    // ClassShape::Move{Member,Method}Shape{Before,Last}. A member only reorders among
    // members, a method among methods (separate sections). Commits as one undo step.
    ClassDiagramShape* _memReorderShape = nullptr;   // the dragged member/method shape
    ClassShape*        _memReorderClass = nullptr;   // its owning class
    bool               _memReorderIsMethod = false;
    QPointF            _memReorderPressWidgetPos;
    QPointF            _memReorderCurrentModelPt;
    bool               _memReorderPotential = false;
    bool               _memReorderActive    = false;
    // Same-kind siblings EXCLUDING the dragged shape (display order) + the insertion
    // index k among them for the current cursor Y. Returns false if not reordering.
    bool memberReorderTarget(QVector<ClassDiagramShape*>& others, int& k) const;
    // True if pc, or one of its members/methods, is selected (controls the
    // class-header 4-way move cursor / affordance).
    bool classOrMemberSelected(ClassShape* pc) const;
    void beginMemberReorderIfReady(QPointF widgetPos);
    void commitMemberReorder();
    void cancelMemberReorder();
    void paintMemberReorderGhost(QPainter& qp);

    // Add Class / Add Note -- interactive placement (port of OnEditAddclass /
    // OnClassdiagramAddNote, which run a modal TrackAdd loop). Modeled as a
    // non-modal placement mode like the SD canvas: the menu arms _placementKind,
    // the next left-click drops the shape at the cursor (Note then opens its dialog,
    // cancel -> rollback; Class opens the add-class dialog then places a ClassShape),
    // mouse-move paints a footprint ghost, Esc / right-click cancels.
    enum class PlacementKind { None, Class, Note };
    PlacementKind _placementKind   = PlacementKind::None;
    bool          _placementHasPos = false;
    QPointF       _placementModelPt;

    // Cheap selection signature (count + single-selected shape ptr); paintEvent
    // emits selectionChanged() when it changes, so a toolbar can refresh its
    // Add-button enables without a dedicated notification from every edit path.
    quintptr      _addSelSig = ~quintptr(0);
    void          emitSelectionChangedIfNeeded();
    bool placementActive() const { return _placementKind != PlacementKind::None; }
    void beginPlacement(PlacementKind kind);
    void cancelPlacement();
    void finishPlacementAt(QPointF widgetPt);
    void paintPlacementGhost(QPainter& qp);

    // Cross-view drop ghost (tree Ctrl-drag preview); model-space anchor, painted
    // like the Class placement footprint. See showDropGhost / clearDropGhost.
    bool    _dropGhostActive = false;
    QPointF _dropGhostModelPt;
    void    paintDropGhost(QPainter& qp);

    // Set in the dtor BEFORE delete _pViewModel so the cascade close-callback
    // fired from ~ClassDiagramViewModel sees we're already going down and no-ops.
    bool _destructing = false;
};

// The hosting window. Top-level, non-modal, self-destructs on close (the owner
// sets Qt::WA_DeleteOnClose via the Qt_Show entry).
class ClassDiagramQtView : public QDialog
{
    Q_OBJECT
public:
    explicit ClassDiagramQtView(ClassDiagram* pClassDiagram,
                                QWidget* parent = nullptr);
    ~ClassDiagramQtView();

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
    void refreshAddActions();   // re-evaluate the Add buttons' enable state
    void exportSvg();           // file dialog + canvas->exportSvg

    ClassDiagramCanvas* _canvas;
    QList<QPair<int, QAction*>> _addActions;
    QAction* _deleteAction = nullptr;   // enable-gated on selection
    QAction* _undoAction   = nullptr;   // enable-gated on undo-stack (canUndo)
    QAction* _redoAction   = nullptr;   // enable-gated on redo-stack (canRedo)
    QToolButton* _alignButton = nullptr; // Align dropdown; enable-gated (>=2 alignable)
};
