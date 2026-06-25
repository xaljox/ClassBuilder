// ClassBuilderDoc.h : interface of the CClassBuilderDoc class
//
// MFC-FREE since the Qt-shell switch (2026-06-09). The class keeps its name
// and its model-facing API (NotifyStructureChanged / SetModifiedFlag /
// LockAllViews / UnLockAndUpdateAllViews / GetPathName / GetTitle /
// OnSaveDocument) so the generated model sources compile unchanged -- but it
// is now a plain class owned by the Qt shell (qt/QtShellWindow), not an MFC
// CDocument.
//
// One document per app instance (the MDI multi-doc ability was dropped with
// the MFC shell; run a second ClassBuilder for a second model).
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "CbString.h"

class DataModelDoc;   // the model container (generated, MFC-free)

class CClassBuilderDoc
{
public:
    CClassBuilderDoc();
    ~CClassBuilderDoc();

    DataModelDoc& GetDataModelDoc();

    // --- Model-facing view refresh API ------------------------------------
    void LockAllViews();
    void UnLockAndUpdateAllViews();

    // The program has 3 view types; each gets an independent "refresh me" kick.
    // The model chokepoints (SaveState / UndoNew / Delete / Undo / Redo) decide
    // WHICH to call from the changed object's Touches{Tree,Cd,Sd} predicates --
    // a tree object usually also touches CD and SD, so several may fire for one
    // change. Each is a pure notification: it records the fact and schedules ONE
    // coalesced flush (at the outermost unlock, or queued via the shell hook
    // when no lock is open). The decision lives in Touches + accumulation; from
    // here it is just a kick to a view type.
    void NotifyTreeViews();   // rebuild the open tree mirrors
    void NotifyCdViews();     // repaint the open class-diagram canvases
    void NotifySdViews();     // repaint the open sequence-diagram canvases

    // Compatibility wrapper for the scattered call sites not yet migrated to the
    // 3-view API (the ~25 NotifyStructureChanged kicks; NotifyGeometryChanged was
    // removed once the chokepoints moved to the three Notify*Views).
    // (NotifyStructureChanged was UpdateAllViews until 2026-06-12.)
    void NotifyStructureChanged();   // = Tree + Cd + Sd

    // --- Dirty flag / naming ----------------------------------------------
    void SetModifiedFlag(int bModified = 1);
    int  IsModified() const { return m_modified; }

    CbString GetPathName() const { return m_pathName; }
    void     SetPathName(const char* path, int addToMru = 0);
    CbString GetTitle() const { return m_title; }
    void     SetTitle(const char* title);

    // --- Document lifecycle (called by the Qt shell + pipe server) --------
    // New document: interactive runs the DataModel wizard dialog; the pipe
    // path stashes PendingNewModelParams on CbCommandServer first (same flow
    // as the MFC OnNewDocument).
    int  OnNewDocument();
    int  OnOpenDocument(const char* path);    // .cbz only
    int  OnSaveDocument(const char* path);    // .cbz only (with backup dance)
    // Bare model write for crash recovery ONLY: serialize straight to a fresh
    // .cbz with NO backup dance and NO SetModifiedFlag/view notification (which
    // would touch a corrupt Qt layout after a GUI fault). Returns 1 on success.
    int  WriteRecoveryCbz(const char* path);
    void DeleteContents();

    // Save to the current path, or invoke the shell's Save-As when untitled.
    // Returns false if the user cancelled or the write failed.
    bool DoSave();

    // --- Undo / Redo (menu glue; logic preserved from the MFC handlers) ---
    void Undo();
    void Redo();
    bool CanUndo();
    bool CanRedo();

    // --- Project / source commands (menu glue) ----------------------------
    void FileSaveSource();
    void FileReadSource();
    void FileDeleteSource();
    void ProjectSettings();
    void ProjectRefreshObjectIds();
    void ProjectAddSerialize();
    bool CanAddSerialize();

    // The Qt shell registers a hook to learn about title / dirty changes
    // (window caption updates). Null is fine.
    typedef void (*StateChangedFn)(void* ctx);
    void SetStateChangedHook(StateChangedFn fn, void* ctx)
    {
        m_stateChangedFn  = fn;
        m_stateChangedCtx = ctx;
    }

    // Broadcast a view-state change to the shell hook. The Qt views call this
    // after an action so the shell re-evaluates EVERY open view's undo/redo
    // toolbar -- a per-diagram action only refreshes the acting view and never
    // reaches the refresh funnel (the only other notifier), so the other views
    // of the same model would otherwise keep stale undo/redo button state.
    // Public for that reason.
    void notifyStateChanged();

    // The queued half of the refresh flusher. This TU sits inside the Qt
    // quarantine (no Qt includes), so the shell registers the actual posting:
    // the hook must arrange for FlushQueuedRefresh() to run on the event loop,
    // DROPPING the call if the document was closed before it fires (the shell
    // owns both facts). With no hook registered the flush degrades to inline
    // (headless safety).
    typedef void (*PostFlushFn)(void* ctx, CClassBuilderDoc* pDoc);
    void SetPostFlushHook(PostFlushFn fn, void* ctx)
    {
        m_postFlushFn  = fn;
        m_postFlushCtx = ctx;
    }
    void FlushQueuedRefresh();   // the posted hook callback lands here

private:

    // Defined in the .cpp (DataModelDoc is incomplete here to keep this
    // header light for the 150 model TUs that include it).
    struct Impl;
    Impl* m_impl;

    CbString m_pathName;
    CbString m_title;
    int      m_modified  = 0;
    bool     m_firstSave = true;

    // View-refresh coalescing, tracked per view type so a pure CD edit need not
    // rebuild the trees or repaint SD (the precision lives in the Touches
    // predicates that set these). A bulk op (rename touching K objects,
    // undo/redo replay) would otherwise refresh K times = O(K x model);
    // LockAllViews defers the flush while > 0, and the matching
    // UnLockAndUpdateAllViews does ONE coalesced flush when the count hits 0.
    int      m_viewLockCount = 0;
    bool     m_treeDirty = false;
    bool     m_cdDirty   = false;
    bool     m_sdDirty   = false;

    StateChangedFn m_stateChangedFn  = nullptr;
    void*          m_stateChangedCtx = nullptr;

    // The refresh funnel. setDirty records the kicked view type(s) and (when no
    // lock is open) asks for a flush; RequestFlush coalesces to ONE posted flush
    // via the shell hook (or inline if none registered); FlushQueuedRefresh reads
    // the 3 flags and repaints exactly the view types that changed. Private:
    // nothing outside this class repaints.
    void setDirty(bool tree, bool cd, bool sd);
    void RequestFlush();

    bool        m_refreshQueued = false;
    PostFlushFn m_postFlushFn   = nullptr;
    void*       m_postFlushCtx  = nullptr;
};
