// ClassBuilderDoc.cpp : implementation of the CClassBuilderDoc class
//
// MFC-FREE since the Qt-shell switch (2026-06-09) -- see ClassBuilderDoc.h.
#include "stdafx.h"

#include "ClassBuilderDoc.h"

#include "qt/QtDataModelDialog.h"
#include "qt/QtSaveSourceDialog.h"
#include "qt/QtReadSourceDialog.h"
#include "qt/QtProjectSettingsDialog.h"
#include "qt/QtAddSerializeDialog.h"
#include "CbCommandServer.h"
#include "MainFrm.h"

#ifdef _WIN32
#include <direct.h>   // _chdir (POSIX chdir via CbWinTypes.h on non-Windows)
#endif
#include <fstream>
#include <string>
#include "CbZstdStream.h"

namespace {
    // RAII bracket for CMainFrame::s_loadInProgress. Construct at the top
    // of any code path that mutates a CClassBuilderDoc under the message
    // pump; the counter increments on entry and decrements on every exit
    // path (including thrown exceptions). The app-activate CheckUpdates
    // walk skips entirely while it's > 0.
    struct LoadInProgressGuard
    {
        LoadInProgressGuard()  { ++CMainFrame::s_loadInProgress; }
        ~LoadInProgressGuard() { --CMainFrame::s_loadInProgress; }
    };

    bool hasCbzExtension(const char* path)
    {
        std::string s(path ? path : "");
        const size_t dot = s.rfind('.');
        if (dot == std::string::npos)
            return false;
        std::string ext = s.substr(dot + 1);
        for (char& c : ext) c = (char)tolower((unsigned char)c);
        return ext == "cbz";
    }
}

/////////////////////////////////////////////////////////////////////////////
// CClassBuilderDoc construction/destruction

struct CClassBuilderDoc::Impl
{
    DataModelDoc dataModelDoc;
};

CClassBuilderDoc::CClassBuilderDoc()
    : m_impl(new Impl)
{
    m_impl->dataModelDoc.SetDocument(this);
}

CClassBuilderDoc::~CClassBuilderDoc()
{
    // Free the model on destruction. ~DataModelDoc does not cascade-delete
    // the model objects (they're freed only by the explicit walks in
    // DeleteContents); calling it here is idempotent.
    DeleteContents();
    delete m_impl;
}

DataModelDoc& CClassBuilderDoc::GetDataModelDoc()
{
    return m_impl->dataModelDoc;
}

void CClassBuilderDoc::notifyStateChanged()
{
    if (m_stateChangedFn)
        m_stateChangedFn(m_stateChangedCtx);
}

/////////////////////////////////////////////////////////////////////////////
// View refresh

void CClassBuilderDoc::LockAllViews()
{
    // Defer view refreshes until the matching UnLockAndUpdateAllViews. Nestable
    // (count), so a bulk op can lock around an inner op that also locks.
    ++m_viewLockCount;
}

void CClassBuilderDoc::NotifyTreeViews() { setDirty(true,  false, false); }
void CClassBuilderDoc::NotifyCdViews()   { setDirty(false, true,  false); }
void CClassBuilderDoc::NotifySdViews()   { setDirty(false, false, true ); }

// Compatibility wrapper for the ~25 NotifyStructureChanged call sites not yet
// migrated to the 3-view API. (NotifyGeometryChanged was removed -- no callers
// left once the chokepoints moved to the three Notify*Views.)
void CClassBuilderDoc::NotifyStructureChanged() { setDirty(true, true, true); }

void CClassBuilderDoc::setDirty(bool tree, bool cd, bool sd)
{
    // Record which view type(s) the change kicks; a pure notification, never
    // repaints inline. Locked: the outermost unlock requests the one flush (a
    // bulk op hits this once per changed object -- coalescing was the 40s
    // rename). Unlocked: ONE queued flush via the shell hook; mutations hold the
    // main thread, so it lands after the running operation -- the same moment a
    // lock would have flushed.
    if (tree) m_treeDirty = true;
    if (cd)   m_cdDirty   = true;
    if (sd)   m_sdDirty   = true;
    if (m_viewLockCount > 0)
        return;
    RequestFlush();
}

void CClassBuilderDoc::RequestFlush()
{
    // The view-type kinds are already accumulated in m_treeDirty/m_cdDirty/
    // m_sdDirty; this just coalesces to ONE flush.
    if (m_refreshQueued)
        return;
    if (m_postFlushFn)
    {
        m_refreshQueued = true;
        m_postFlushFn(m_postFlushCtx, this);   // shell posts FlushQueuedRefresh
        return;
    }
    FlushQueuedRefresh();   // no shell hook (headless): degrade to inline
}

void CClassBuilderDoc::FlushQueuedRefresh()
{
    // Read + clear the 3 view-type flags, then kick exactly those view types.
    // (The undo-stack walk in notifyStateChanged that crashed Select-Classes was
    // a freed-entry read from the old paint-time purge; that purge is gone with
    // the _isDrawing/EndDraw scaffold, so this always walks a consistent stack.)
    m_refreshQueued = false;
    const bool tree = m_treeDirty, cd = m_cdDirty, sd = m_sdDirty;
    m_treeDirty = m_cdDirty = m_sdDirty = false;

    // Tree mirrors rebuild wholesale (state-preserving Qt mirrors); the canvas
    // repaints are posted idempotent QWidget::update()s.
    if (tree)
        m_impl->dataModelDoc.UpdateTreeViews();
    if (cd)
        m_impl->dataModelDoc.UpdateClassDiagramViews();
    if (sd)
        m_impl->dataModelDoc.UpdateSequenceDiagramViews();

    // Refresh the shell's Undo/Redo/Save toolbar (+ dirty-flag titles) once per
    // coalesced op. Undo/Redo enablement tracks the undo STACK, not the dirty
    // flag (a second edit after an undo is undoable but causes no dirty
    // transition), so it must refresh here -- the single funnel per op.
    notifyStateChanged();
}

void CClassBuilderDoc::UnLockAndUpdateAllViews()
{
    if (m_viewLockCount > 0)
        --m_viewLockCount;
    if (m_viewLockCount > 0)
        return;

    // Outermost unlock: the lock-scope notifies already accumulated the dirty
    // view types in m_treeDirty/m_cdDirty/m_sdDirty; coalesce them into ONE
    // flush. RequestFlush dedups against a flush already queued by an unlocked
    // notify earlier in the same op; FlushQueuedRefresh reads + clears the flags.
    if (m_treeDirty || m_cdDirty || m_sdDirty)
        RequestFlush();
}

/////////////////////////////////////////////////////////////////////////////
// Dirty flag / naming

void CClassBuilderDoc::SetModifiedFlag(int bModified)
{
    if (m_modified != bModified)
    {
        m_modified = bModified;
        notifyStateChanged();
    }
}

void CClassBuilderDoc::SetPathName(const char* path, int /*addToMru*/)
{
    // Normalize to backslashes at this single chokepoint. The Qt file dialog
    // hands back forward-slash paths ("C:/.../Foo.cbz"), but GetPath() splits
    // the directory on '\\' only -- a forward-slash path made GetPath() return
    // the whole file path, so the codegen _chdir() failed and generated
    // sources landed in the launch CWD instead of the .cbz's folder.
    std::string s(path ? path : "");
    for (char& c : s)
        if (c == '/')
            c = '\\';
    m_pathName = s.c_str();

    // Title = filename without directory.
    const size_t slash = s.find_last_of('\\');
    m_title = (slash != std::string::npos ? s.substr(slash + 1) : s).c_str();
    notifyStateChanged();
}

void CClassBuilderDoc::SetTitle(const char* title)
{
    m_title = title ? title : "";
    notifyStateChanged();
}

/////////////////////////////////////////////////////////////////////////////
// Lifecycle

// Wipe the in-memory model so the same doc instance can be reloaded cleanly.
// Loading over an existing model would simply ADD the new objects on top of
// the old ones (every owned-relation read appends), doubling the model.
void CClassBuilderDoc::DeleteContents()
{
    m_impl->dataModelDoc.DeleteAllUndoBase();
    m_impl->dataModelDoc.DeleteAllRedoBase();
    m_impl->dataModelDoc.DeleteAllDataModelDocObject();
}

int CClassBuilderDoc::OnNewDocument()
{
    LoadInProgressGuard loadGuard;

    DeleteContents();
    m_pathName = "";
    m_firstSave = true;

    DataModel* pDataModel = new DataModel(&m_impl->dataModelDoc);

    // Pipe-driven creation (new_model_basic / new_model_serialize) stashes
    // params on CbCommandServer before asking for a new doc; when present,
    // skip the wizard and apply them directly.
    CbCommandServer::PendingNewModelParams* pParams =
        CbCommandServer::GetPendingNewModelParams();
    if (pParams)
    {
        pDataModel->SetName(pParams->name);
        pDataModel->SetHFile(pParams->hFile);
        pDataModel->SetSerialize(pParams->serialize ? 1 : 0);
        pDataModel->SetUndoRedo(pParams->undoRedo ? 1 : 0);

        {
            CbViewLock lock(&GetDataModelDoc());
            pDataModel->Init(pParams->className);
            GetDataModelDoc().DeleteAllUndoBase();
        }

        SetTitle(pParams->name);
        SetModifiedFlag(1);
        return 1;
    }

    void* ownerHwnd = Cb_GetMainHwnd();
    DataModelDialogResult dlgResult =
        Qt_ShowDataModelDialog(pDataModel, "", ownerHwnd);
    if (dlgResult.accepted)
    {
        // The Qt dialog has already applied every edit to the model.
        {
            CbViewLock lock(&GetDataModelDoc());
            pDataModel->Init(CbString(dlgResult.className.c_str()));
            GetDataModelDoc().DeleteAllUndoBase();
        }

        SetTitle((const char*)pDataModel->GetName());
        SetModifiedFlag(1);
        return 1;
    }
    else
    {
        delete pDataModel;
        return 0;
    }
}

int CClassBuilderDoc::OnOpenDocument(const char* path)
{
    LoadInProgressGuard loadGuard;
    Cb_BeginWaitCursor();

    // Debug trace of the document being loaded (compiled out in Release).
    TRACE("OnOpenDocument: %s\n", path);

    if (!hasCbzExtension(path))
    {
        // Legacy MFC CArchive (.CBD) load path is no longer supported. Convert
        // the file to .cbz first (use ClassBuilderStatic.exe for batch convert).
        Cb_EndWaitCursor();
        std::string msg = std::string("Cannot open '") + path + "'.\n\n"
            "The legacy .CBD format is no longer supported. Convert it "
            "to .CBZ first (e.g. with ClassBuilderStatic.exe).";
        CbMessageBox(msg.c_str(), CBMB_OK | CBMB_ICONERROR);
        return 0;
    }

    DeleteContents();
    std::ifstream is(path, std::ios::binary);
    if (!is)
    {
        Cb_EndWaitCursor();
        std::string msg = std::string("Failed to open '") + path + "' for reading.";
        CbMessageBox(msg.c_str(), CBMB_OK | CBMB_ICONERROR);
        return 0;
    }
    try
    {
        CbZstdInBuf zbuf(is);
        std::istream zis(&zbuf);
        CbArchive ar(zis);
        m_impl->dataModelDoc.Serialize(ar);
        // Saturate _objectVersion so the undo/redo evolution gates treat
        // a freshly loaded document as current. (The legacy CBD->CBZ
        // version-migration fixups live in ClassBuilderStatic.exe.)
        m_impl->dataModelDoc._objectVersion = INT_MAX;
    }
    catch (int code)
    {
        Cb_EndWaitCursor();
        char buf[768];
        if (code == CB_ARCHIVE_BAD_STREAM)
            sprintf_s(buf, "Cannot open '%s'.\n\nThe file is corrupt, or was "
                "written by an incompatible ClassBuilder version (its serialized "
                "layout does not match this build).", path);
        else
            sprintf_s(buf, "CbArchive load failed for '%s' (code %d).", path, code);
        CbMessageBox(buf, CBMB_OK | CBMB_ICONERROR);
        return 0;
    }
    catch (...)
    {
        Cb_EndWaitCursor();
        std::string msg = std::string("CbArchive load failed for '") + path +
                          "' (unknown exception).";
        CbMessageBox(msg.c_str(), CBMB_OK | CBMB_ICONERROR);
        return 0;
    }
    SetModifiedFlag(0);

    // _newClassPrefix is not serialized -- seed it from the stored prefix.
    DataModel* pDataModel = m_impl->dataModelDoc.GetDataModel();
    pDataModel->SetNewClassPrefix(pDataModel->GetClassPrefix());

    // Refresh derived diagram geometry now that the whole model is loaded. The
    // stored boxes/segments can predate the current RecalculateRect (e.g. a
    // model built via the pipe), so a box "resizes" on first recompute and its
    // connection reroutes. Do it HERE, not in a paint (paints stay neutral) and
    // not at the first user edit (the reroute would otherwise land segment churn
    // on that edit's undo step -- even a plain selection). Loading is not an
    // undoable edit, so the churn this produces is wiped by the
    // DeleteAllUndoBase below.
    m_impl->dataModelDoc.RecalculateAllDiagrams();

    m_impl->dataModelDoc.DeleteAllUndoBase();
    SetPathName(path);
    SetModifiedFlag(0);     // SetPathName notified; make the flag state explicit
    Cb_EndWaitCursor();

    return 1;
}

int CClassBuilderDoc::OnSaveDocument(const char* pathIn)
{
    std::string path(pathIn ? pathIn : "");

    // Keep the old save-dialog quirk fix: strip an appended ".cbd" from
    // "Foo.cbz.cbd" so the user's intent (write as CBZ) is honoured.
    {
        std::string lc = path;
        for (char& c : lc) c = (char)tolower((unsigned char)c);
        if (lc.size() > 8 && lc.compare(lc.size() - 8, 8, ".cbz.cbd") == 0)
        {
            path = path.substr(0, path.size() - 4);
            SetPathName(path.c_str());
        }
    }

    if (!hasCbzExtension(path.c_str()))
    {
        std::string msg = std::string("Cannot save '") + path + "'.\n\n"
            "The legacy .CBD format is no longer supported. "
            "Save as .CBZ instead (File - Save As).";
        CbMessageBox(msg.c_str(), CBMB_OK | CBMB_ICONERROR);
        return 0;
    }

    // Backup dance: first save of the session goes to .~~CBZ (and removes the
    // stale .~CBZ), subsequent saves rotate the current file to .~CBZ.
    const size_t dot = path.rfind('.');
    if (dot != std::string::npos)
    {
        std::string backupFile = path.substr(0, dot);
        if (m_firstSave)
        {
            remove((backupFile + ".~CBZ").c_str());
            backupFile += ".~~CBZ";
        }
        else
        {
            backupFile += ".~CBZ";
        }
        remove(backupFile.c_str());
        rename(path.c_str(), backupFile.c_str());
    }
    m_firstSave = false;

    {
        std::ofstream os(path.c_str(), std::ios::binary | std::ios::trunc);
        if (!os) return 0;
        try
        {
            CbZstdOutBuf zbuf(os);
            std::ostream zos(&zbuf);
            CbArchive ar(zos);
            m_impl->dataModelDoc.Serialize(ar);
            // ar / zos / zbuf destructors run in reverse order; zbuf's
            // destructor calls ZSTD_e_end and writes the compressed tail.
        }
        catch (...)
        {
            return 0;
        }
        SetModifiedFlag(0);
        return 1;
    }
}

int CClassBuilderDoc::WriteRecoveryCbz(const char* path)
{
    // Used ONLY by the GUI-crash emergency save. Mirrors OnSaveDocument's inner
    // write (zstd-framed CbArchive) but deliberately OMITS the backup rename and
    // the trailing SetModifiedFlag(0) -> notifyStateChanged, which would touch
    // the corrupt Qt dock layout and fault again. Model-only -> safe post-crash.
    std::ofstream os(path, std::ios::binary | std::ios::trunc);
    if (!os) return 0;
    try
    {
        CbZstdOutBuf zbuf(os);
        std::ostream zos(&zbuf);
        CbArchive ar(zos);
        m_impl->dataModelDoc.Serialize(ar);
    }
    catch (...)
    {
        return 0;
    }
    return 1;
}

bool CClassBuilderDoc::DoSave()
{
    if (m_pathName.IsEmpty())
        return false;            // untitled -- the shell must run Save As
    return OnSaveDocument((const char*)m_pathName) != 0;
}

/////////////////////////////////////////////////////////////////////////////
// Undo / Redo

void CClassBuilderDoc::Undo()
{
    // DataModelDoc::Undo owns the whole contract now: RAII view lock + wait
    // cursor around the replay, and -- when anything was restored -- the
    // coalesced refresh of EVERY view of this model (snapshot restores bypass
    // the setters, so the refresh must be forced there, not per entry point).
    GetDataModelDoc().Undo();
}

void CClassBuilderDoc::Redo()
{
    GetDataModelDoc().Redo();   // see Undo() -- the model side owns the contract
}

bool CClassBuilderDoc::CanUndo()
{
    // Enablement is "is there a step on the stack" -- nothing more. It must NOT
    // walk the entries and dereference their objects (the old code dynamic_cast'd
    // each entry's object to special-case open diagrams, which crashed on any
    // stale entry pointer and reached into the model from a paint-time refresh).
    return GetDataModelDoc().GetUndoBaseCount() > 0;
}

bool CClassBuilderDoc::CanRedo()
{
    return GetDataModelDoc().GetRedoBaseCount() > 0;
}

/////////////////////////////////////////////////////////////////////////////
// Project / source commands

void CClassBuilderDoc::FileSaveSource()
{
    _chdir(m_impl->dataModelDoc.GetPath());
    Qt_ShowSaveSourceDialog(m_impl->dataModelDoc.GetDataModel(), Cb_GetMainHwnd());
}

void CClassBuilderDoc::FileReadSource()
{
    _chdir(m_impl->dataModelDoc.GetPath());
    Qt_ShowReadSourceDialog(m_impl->dataModelDoc.GetDataModel(), Cb_GetMainHwnd());
    GetDataModelDoc().MarkLastUndo();
}

void CClassBuilderDoc::FileDeleteSource()
{
    if (CbMessageBox("Are you sure you want to delete the source files",
                      CBMB_ICONQUESTION | CBMB_YESNO) == CBMB_IDYES)
    {
        _chdir(m_impl->dataModelDoc.GetPath());

        DataModel* pDataModel = m_impl->dataModelDoc.GetDataModel();
        pDataModel->CheckUpdates();

        remove(pDataModel->GetHFile());

        DataModel::ClassIterator iClass(pDataModel);
        while (++iClass)
        {
            remove(iClass->GetHFile());
            remove(iClass->GetCppFile());
        }
    }
}

void CClassBuilderDoc::ProjectSettings()
{
    Qt_ShowProjectSettingsDialog(&GetDataModelDoc(), Cb_GetMainHwnd());
}

void CClassBuilderDoc::ProjectRefreshObjectIds()
{
    GetDataModelDoc().RefreshObjectIds();
    DataModelDoc::DataModelDocObjectIterator iObject(GetDataModelDoc());
    while (++iObject)
    {
        Gti* pGti = dynamic_cast<Gti*>(iObject.Get());
        if (pGti && !pGti->GetAdded())
        {
            pGti->Add();
        }
    }
}

void CClassBuilderDoc::ProjectAddSerialize()
{
    std::string className;
    // The Qt dialog validates the name and asks the "can not be undone"
    // confirmation itself; OK here means the user has confirmed.
    if (Qt_ShowAddSerializeDialog(&GetDataModelDoc(), className, Cb_GetMainHwnd()))
    {
        GetDataModelDoc().GetDataModel()->AddSerialize(
            CbString(className.c_str()));
    }
}

bool CClassBuilderDoc::CanAddSerialize()
{
    return !GetDataModelDoc().GetDataModel()->GetSerialize();
}

/////////////////////////////////////////////////////////////////////////////
// Qt main-tree bridge entry point

// Complete a drag started Qt-side with pDrag->Drag() (which removed the node /
// set pDropDefault): under LockAllViews, Drop onto pTarget (move / Ctrl-copy)
// unless pTarget is the origin (then RollBack), then UnLockAndUpdateAllViews +
// MarkLastUndo. Lives doc-side because the Lock/UnLock pair are doc methods.
// (Declared in qt/QtMainTreeView.h.)
void Qt_MainTreeDrop(DataModelDoc* pDataModelDoc, Gti* pDrag, Gti* pTarget,
                     bool ctrl, Gti* pDropDefault)
{
    if (!pDataModelDoc || !pDrag)
        return;

    if (pTarget)
    {
        CbViewLock lock(pDataModelDoc);
        if (ctrl || pTarget != pDropDefault)
            pDrag->Drop(ctrl, pTarget);
        else
            pDataModelDoc->RollBack();          // dropped at origin -> undo the Drag()
    }
    pDataModelDoc->MarkLastUndo();
}
