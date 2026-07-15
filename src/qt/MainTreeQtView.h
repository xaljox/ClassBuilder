// qt/MainTreeQtView.h -- the Qt-side main class tree (read-only mirror).
//
// M0 of the main-tree port. A top-level non-modal QDialog wrapping a
// CbTreeWidget. It walks the document the same way CClassBuilderView::RedrawTree
// does -- DataModel, then each MetaGroup, then the ExternClasses / OtherTypes /
// Actors roots -- recursing each Gti's Parent/Child tree via Gti::ChildIterator
// and rendering GetItemText() + Qt_ModelIcon(GetIcon()). Read-only: no
// selection, context menu, or drag yet (M1+).
//
// The default CClassBuilderView visibility flags show everything, so a plain
// ChildIterator walk reproduces the default MFC tree node-for-node. The
// per-type Show() visibility gates (Class/Member/Method phase + access filters)
// are deliberately NOT replicated here -- they belong with the M4 view-toggle
// slice. The only gate honoured is Gti::GetAdded(), which Gti::Show() also
// checks (a not-yet-added node is invisible in both trees).
#pragma once

#include <QDialog>
#include <QSet>
#include <QHash>
#include <QList>
#include <QPair>
#include <QSize>
#include <QPoint>
#include <QCursor>

#include "QtSearchDialog.h"   // SearchParams (Ctrl+F find state, per view)

class DataModelDoc;
class Gti;
class DiagramDropTarget;
class TreeViewModel;
class CbTreeWidget;
class MainTreeWidget;
class QTreeWidgetItem;
class QDialog;
class QGroupBox;
class QAction;

class MainTreeQtView : public QDialog
{
    Q_OBJECT
public:
    // subTree != null scopes the mirror to that node + its descendants (the Qt
    // equivalent of the MFC "New Sub Window"); null mirrors the whole document.
    explicit MainTreeQtView(DataModelDoc* pDataModelDoc, void* ownerHwnd,
                            Gti* subTree = nullptr, QWidget* parent = nullptr);
    ~MainTreeQtView() override;

    // Full repopulate from the model. Expansion + selection are preserved across
    // the rebuild by remembering the Gti* of each expanded / selected row (the
    // model objects survive the rebuild), so a live MFC-side edit that triggers
    // a refresh doesn't collapse the mirror.
    void rebuild();

    // Select + reveal the row of a model object (expands collapsed ancestors,
    // scrolls to it). False when the object has no row in this mirror.
    bool selectGti(Gti* pGti);

    DataModelDoc* dataModelDoc() const { return _pDoc; }

    // Re-evaluate the toolbar Add-buttons' enabled state against the current
    // selection (same gating as the context menu's checkOnly pass).
    void updateToolBarEnables();

    // Show/hide this tree's own Undo/Redo toolbar buttons. They are HIDDEN while
    // the tree is docked (the main window's Undo/Redo are right there) and shown
    // only when the tree is floated away. Wired to the hosting dock's
    // topLevelChanged in QtShellWindow.
    void setFloating(bool floating);

public slots:
    // Re-evaluate just this tree's Undo/Redo button enables against its doc's
    // CanUndo/CanRedo. Invoked BY NAME from the shell (so it needs no include of
    // this header) to keep every open view of a model in sync -- a per-view
    // action only refreshes the acting view otherwise.
    void refreshUndoRedoEnables();

protected:
    // Manual drag-drop (M3): mirrors the MFC OnBegindrag/OnMouseMove/OnLButtonUp
    // transaction on the tree's viewport (Qt's async QDrag would leave the model
    // half-mutated across its blocking loop).
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    // Qt -> MFC: a user selection change drives the MFC tree's m_current; a
    // double-click opens (or, with Ctrl/Alt, edits) via the MFC routing.
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

    // M2: native context menu (Open / Edit Attributes / Add submenu), driving
    // the model OnXxx methods on the right-clicked node (DataModel if none).
    void onContextMenu(const QPoint& pos);

    // M4: non-modal checkbox dialog flipping this view's TreeViewModel filters.
    void showFilterDialog();

    // M4 refresh migration: the VM's callbacks (ctx == this). The model fires
    // Refresh() on change (-> posted rebuild) and cascade-deletes the VM on doc
    // close (-> CloseTree -> notifyModelGone -> close the window). Mirrors SD/CD.
    static void RefreshTree(void* ctx);
    static void CloseTree(void* ctx);
    void notifyModelGone();

    // Queue ONE coalesced rebuild() on the event loop (refresh-audit step 2):
    // the model fires RefreshTree mid-mutation, and a synchronous full
    // QTreeWidget rebuild from inside a model op (worst case: mid-paint) is the
    // audit's reentrancy hazard. Posting also collapses refresh bursts.
    void scheduleRebuild();

    // Del key on the focused tree -> delete the current node (OnDelete).
    void onDeleteKey();

    // Ctrl+C / Ctrl+V on the focused tree -> Gti::OnCopy / OnPaste, sharing the
    // one Gti::_pGtiCopy buffer with the MFC tree (so copy/paste crosses trees).
    void onCopyKey();
    void onPasteKey();

    // Ctrl+F opens the Qt Find dialog and selects the first match; F3 repeats
    // with the stored params -- mirrors CClassBuilderView::OnEditFind/OnEditRepeat
    // (Gti::FindStringFiltered from the current node). Find state is per view.
    void onFindKey();
    void onRepeatKey();
    void findSelectFrom(Gti* pStart);   // FindStringFiltered + select/scroll to hit

private:
    // Drag helpers (M3). beginDrag calls Gti::Drag (model removes the node for a
    // move); updateDragTarget validates the hovered node via Gti::DropTarget;
    // finishDrag completes via the Qt_MainTreeDrop bridge.
    bool beginDrag();
    void updateDragTarget(const QPoint& viewportPos);
    void finishDrag();
    QCursor makeDragCursor(Gti* pGti, bool copy) const;

    // Open a ClassDiagram/SequenceDiagram node in its Qt view (not the MFC MDI
    // child) -- returns true if pGti was a diagram and a view was opened.
    bool openDiagramViewIfDiagram(Gti* pGti);

    // M4 view filters: mirror the per-type Show() gates (Member/Method access,
    // phase). Returns false to hide the node (and its subtree). The Qt mirror
    // owns its own filter state (like MFC sub-windows each do), toggled from the
    // View menu; defaults match the MFC view (everything shown).
    bool nodeVisible(Gti* pGti) const;

    // Mirror of Gti::Show: skip !GetAdded(), else add the row and recurse the
    // Gti Parent/Child children. parent == nullptr makes a top-level row.
    void addGtiRecursive(Gti* pGti, QTreeWidgetItem* parent);

    // Expansion/selection capture + restore keyed on the stored Gti*.
    void captureState(QTreeWidgetItem* item, QSet<quintptr>& expanded,
                      quintptr& selected) const;
    void restoreState(QTreeWidgetItem* item, const QSet<quintptr>& expanded,
                      quintptr selected, QTreeWidgetItem*& selItemOut) const;

    MainTreeWidget* _tree;
    DataModelDoc*   _pDoc;
    void*           _ownerHwnd;   // MFC main window, for opening Qt diagram views

    // Phase state markers (the MFC tree's state image). When the model has phase
    // support, each row shows the phase icon left of its type icon -- so the
    // tree's icon area is widened to two glyphs and the icons composited.
    QSize _baseIconSize;        // one glyph, as CbTreeWidget sized it
    bool  _phaseSupport = false;

    // Row lookup by model Gti* (for the MFC->Qt selection echo + rebuild state
    // restore). Rebuilt from scratch in rebuild().
    QHash<quintptr, QTreeWidgetItem*> _itemByGti;

    // True while applying a selection that came FROM the model side (rebuild
    // restore or Qt_MainTreeSetCurrent) -- suppresses the Qt->MFC bridge so the
    // echo can't loop.
    bool _applyingExternalSelection = false;

    // A model-fired rebuild is already queued on the event loop; further
    // RefreshTree calls are no-ops until it runs (scheduleRebuild).
    bool _rebuildQueued = false;

    // Toolbar Add-buttons, paired with their TreeAction (stored as int -- the
    // enum is cpp-local). Enabled state follows the selection.
    QList<QPair<int, QAction*>> _toolBarActions;

    // This tree's own Undo/Redo (+ their leading separator), hidden while docked
    // and shown only when floated (setFloating). Enable tracks the doc's undo/
    // redo STACK (CanUndo/CanRedo), refreshed in updateToolBarEnables.
    QAction* _undoAction    = nullptr;
    QAction* _redoAction    = nullptr;
    QAction* _undoRedoSep   = nullptr;

    // M4 view filters live in this view's own TreeViewModel (model-side, non-
    // serialized, owned passively by the doc -- this window manages its lifetime).
    // nodeVisible() reads it via Gti::ShownByFilter; the Filters dialog flips it.
    TreeViewModel* _vm = nullptr;
    bool           _destructing = false;           // guards the VM teardown race
    QDialog*       _filterDialog = nullptr;        // single non-modal instance
    QGroupBox*     _filterPhasesGroup = nullptr;   // re-enabled live on phase-support change
    SearchParams   _searchParams;                  // Ctrl+F find state (per view)

    // Manual drag-drop state (M3). _dragGti is the dragged node; _dragDefault its
    // origin (Drag()'s out-param); _dragTarget the current valid drop node (or
    // origin). _dragCtrl fixes move/copy at drag start (as the MFC tree does).
    bool   _dragArmed   = false;   // left-pressed a draggable row, threshold not yet crossed
    bool   _dragActive  = false;   // Drag() succeeded -- gesture in progress
    QPoint _dragPressPos;
    Gti*   _dragGti      = nullptr;
    Gti*   _dragDefault  = nullptr;
    Gti*   _dragTarget   = nullptr;
    bool   _dragCtrl     = false;

    // The Qt diagram canvas (CD or SD) currently showing this drag's drop ghost
    // (Ctrl+drag hovering a diagram), so we can clear it when the cursor leaves /
    // drops. Held as the drop-target interface, not a concrete canvas type.
    DiagramDropTarget* _ghostCanvas = nullptr;
};
