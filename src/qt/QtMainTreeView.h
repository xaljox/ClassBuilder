// qt/QtMainTreeView.h -- MFC-callable entry points for the Qt main class tree.
//
// M0: a non-modal, READ-ONLY top-level Qt window that mirrors the whole
// document's class tree (the CClassBuilderView CTreeView). It is the first
// slice of porting the main navigation tree to Qt -- it sits side-by-side with
// the MFC tree so the two can be compared node-for-node while the interactive
// layers (selection, context menu, drag-drop) are ported one milestone at a
// time. Interaction lands in M1+.
//
// Lifetime / live-mirror: like the SD/CD Qt views, the window is outside MFC's
// view set, so the document's UnLockAndUpdateAllViews catch-all calls
// Qt_RefreshDocumentTrees to repaint it (a global registry of open tree windows
// keyed on the DataModelDoc -- no per-object ViewModel, the tree is
// document-wide). See qt/MainTreeQtView.cpp.
#pragma once

class DataModelDoc;
class Gti;

// Open a new Qt mirror of pDataModelDoc's class tree. ownerHwnd is the MFC main
// window (for modeless owner + centering), as with the diagram views.
void Qt_ShowMainTreeView(DataModelDoc* pDataModelDoc, void* ownerHwnd);

// Like Qt_ShowMainTreeView, but a no-op if a Qt tree for this doc is already
// open (one Qt window per doc). Called from CClassBuilderView::OnInitialUpdate
// to make the Qt tree the primary tree shown when a document opens (M-B of the
// MFC-main-tree retirement). The "Show in Qt tree" context entry still uses the
// unguarded Qt_ShowMainTreeView to deliberately allow extra mirrors.
void Qt_EnsureMainTreeView(DataModelDoc* pDataModelDoc, void* ownerHwnd);

// (The M1 bidirectional selection/navigation bridge -- Qt_MainTreeNavigate /
//  Qt_MainTreeSetCurrent -- is gone with the MFC tree: the Qt tree drives the
//  model directly and IS the selection.)

// Qt tree -> MFC. IMPLEMENTED ON THE MFC SIDE (ClassBuilderView.cpp): complete a
// drag started with pDrag->Drag() (which removed it / set pDefault). Mirrors the
// in-tree branch of CClassBuilderView::OnLButtonUp: under LockAllViews, Drop onto
// pTarget (a real move/copy) unless pTarget is the origin (then RollBack), then
// UnLockAndUpdateAllViews + MarkLastUndo. The Lock/UnLock are MFC-doc methods, so
// this must run MFC-side; begin (Drag) + DropTarget validation stay Qt-side.
void Qt_MainTreeDrop(DataModelDoc* pDataModelDoc, Gti* pDrag, Gti* pTarget,
                     bool ctrl, Gti* pDropDefault);
