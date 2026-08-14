/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DataModelDoc.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DataModelDoc'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
* Philips Digital Video Systems, Eindhoven, The Netherlands.
* Distributed under the GNU General Public License (GPL)
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
#include "ClassBuilderDoc.h"
//@END_USER2


// Static members
bool DataModelDoc::_membersOnly = false;
int DataModelDoc::_objectVersion = 0;


/*@NOTE_41
Constructor needed for serialization, not meant to use for other purposes!
*/
DataModelDoc::DataModelDoc() //@INIT_41
    : CbObject()
    , _pDoc(NULL)
    , _currentUndoCount(0)
    , _maxUndoCount(10)
    , _isRedoing(0)
    , _isUndoing(0)
    , _version(1)
    , _nextObjectId(0)
    , _additionalAllowed("")
    , _activationNoMethodPenColor(Cb_RGB(64, 128, 128))
    , _activationPenColor(Cb_RGB(0, 0, 0))
    , _lifeLinePenColor(Cb_RGB(0, 0, 0))
    , _lifeLineTextColor(Cb_RGB(0, 0, 0))
    , _SDNoteShapePenColor(Cb_RGB(0, 0, 0))
    , _SDNoteShapeTextColor(Cb_RGB(0, 0, 0))
    , _signalNoMethodPenColor(Cb_RGB(64, 128, 128))
    , _signalPenColor(Cb_RGB(0, 0, 0))
    , _signalTextColor(Cb_RGB(0, 0, 0))
    , _classPenColor(Cb_RGB(0, 0, 0))
    , _classTextColor(Cb_RGB(0, 0, 0))
    , _memberTextColor(Cb_RGB(0, 0, 0))
    , _methodTextColor(Cb_RGB(0, 0, 0))
    , _relationPenColor(Cb_RGB(0, 0, 0))
    , _relationTextColor(Cb_RGB(0, 0, 0))
    , _dependencyPenColor(Cb_RGB(0, 0, 0))
    , _dependencyTextColor(Cb_RGB(0, 0, 0))
    , _relationDiagramOnlyPenColor(Cb_RGB(128, 128, 128))
    , _relationDiagramOnlyTextColor(Cb_RGB(0, 0, 0))
    , _inheritPenColor(Cb_RGB(0, 0, 0))
    , _criticalRelationPenColor(Cb_RGB(255, 0, 0))
    , _noteShapePenColor(Cb_RGB(0, 0, 0))
    , _noteShapeTextColor(Cb_RGB(0, 0, 0))
    , _activationUser3PenColor(Cb_RGB(0, 64, 128))
    , _activationInitialPenColor(Cb_RGB(128, 0, 0))
    , _lastSearch()
    , _matchCase(true)
    , _methodNameList("CopyValuesFrom\015\012"
"CopyValuesFrom=\015\012"
"operator !=\015\012"
"operator <\015\012"
"operator >\015\012"
"operator <=\015\012"
"operator >=\015\012"
"operator +\015\012"
"operator -\015\012"
"operator *\015\012"
"operator /\015\012"
"operator %\015\012"
"operator +=\015\012"
"operator -=\015\012"
"operator *=\015\012"
"operator /=\015\012"
"operator %=\015\012"
"operator !\015\012"
"operator &&\015\012"
"operator ||\015\012"
"operator ++\015\012"
"operator --\015\012"
"operator &\015\012"
"operator |\015\012"
"operator ^\015\012"
"operator ~\015\012"
"operator &=\015\012"
"operator |=\015\012"
"operator ^=\015\012"
"operator <<\015\012"
"operator >>\015\012"
"operator <<=\015\012"
"operator >>=\015\012"
"operator ->\015\012"
"operator ()\015\012"
"operator []\015\012"
"operator new\015\012"
"operator delete\015\012"
"operator bool\015\012"
"operator int\015\012")
    , _similarLinesList("@ = X.@;\015\012"
"@ == X.@ &&\015\012"
"@ != X.@ ||\015\012"
"Set@(X.Get@());\015\012"
"os << \"@: \" << @ << endl;\015\012")
    , _commentInitialCode()
{//@CODE_41
    SerializeConstructorInclude();

    // Put in your own code

    // The undo snapshot/restore (UndoChange) deserializes through CbArchive,
    // whose read side gates every evolution-grouped field on
    // `N <= _objectVersion`. Saturate the shared static here so a freshly
    // created (never-loaded) document has working undo -- a document LOAD
    // overrides it via DataModelDoc::Serialize with the file's _version, which
    // is correct for evolution gating during the load itself.
    DataModelDocObject::_objectVersion = INT_MAX;
}//@CODE_41


/*@NOTE_39
Destructor method
*/
DataModelDoc::~DataModelDoc()
{//@CODE_39
    DestructorInclude();

    // Put in your own code
}//@CODE_39


void DataModelDoc::AddClassDiagramShapes(ClassDiagram* pClassDiagram)
{//@CODE_35098
    ClassDiagram::ClassDiagramShapeIterator iClassDiagramShape(pClassDiagram);
    while (++iClassDiagramShape)
    {
        AddDataModelDocObjectLast(iClassDiagramShape);
    }
}//@CODE_35098


void DataModelDoc::AddSequenceDiagramShapes(SequenceDiagram* pSequenceDiagram)
{//@CODE_35096
    SequenceDiagram::SequenceDiagramShapeIterator iSequenceDiagramShape(pSequenceDiagram);
    while (++iSequenceDiagramShape)
    {
        AddDataModelDocObjectLast(iSequenceDiagramShape);
    }
}//@CODE_35096


bool DataModelDoc::CanRedo()
{//@CODE_40949
    return GetDocument()->CanRedo();
}//@CODE_40949


bool DataModelDoc::CanUndo()
{//@CODE_40948
    return GetDocument()->CanUndo();
}//@CODE_40948


/*@NOTE_5095
Clean the redo stack, unless we are undoing.
*/
void DataModelDoc::CleanRedo()
{//@CODE_5095
    if (!_isRedoing && !_isUndoing)
    {
        RedoBaseIterator iRedoBase(this);
        while (++iRedoBase)
        {
            delete iRedoBase;
        }
    }
}//@CODE_5095


void DataModelDoc::ConvertNonCSymbols(CbString& str)
{//@CODE_23051
    for (int i = 0; i < str.GetLength(); i++)
    {
        if (!__iscsym(str[i]))
            str.SetAt(i, '_');
    }
}//@CODE_23051


bool DataModelDoc::DeletedInOpenUndoStep(DataModelDocObject* pDataModelDocObject)
{//@CODE_41496
    // Walk the undo stack backward over the CURRENT (not-yet-marked) step
    // only; if the object already has an UndoDelete OR UndoSubDelete there, an
    // earlier delete in this step removed it (directly or via cascade).
    UndoBaseIterator iUndoBase(this);
    while (--iUndoBase)
    {
        UndoBase* pUndoBase = iUndoBase.Get();
        if (pUndoBase->GetLast())
            break;
        if (pUndoBase->GetDataModelDocObject() == pDataModelDocObject &&
            (dynamic_cast<UndoDelete*>(pUndoBase) || dynamic_cast<UndoSubDelete*>(pUndoBase)))
            return true;
    }
    return false;
}//@CODE_41496


BaseClass* DataModelDoc::FindBaseClass(const CbString& rName)
{//@CODE_1282
    BaseClassIterator iBaseClass(this);
    while (++iBaseClass)
    {
        if (rName == iBaseClass->GetName())
        {
            return iBaseClass;
        }
    }

    return 0;
}//@CODE_1282


DataModelDocObject* DataModelDoc::FindDataModelDocObject(unsigned int id)
{//@CODE_1286
    DataModelDocObjectIterator iDataModelDocObject(this);
    while (++iDataModelDocObject)
    {
        if (id == iDataModelDocObject->GetId())
        {
            return iDataModelDocObject;
        }
    }

    return 0;
}//@CODE_1286


/*@NOTE_7572
Find type with same name as 'pType' if not found, create similair type in current project.
*/
ExternClass* DataModelDoc::FindOrDuplicateExternClass(Type* pType)
{//@CODE_7572
    Type* pExternClass = FindType(pType->GetName());

    if (!pExternClass || pExternClass->IsOtherType())
    {
        pExternClass = new ExternClass(this, (ExternClass*)pType);
    }
    
    return (ExternClass*)pExternClass;
}//@CODE_7572


/*@NOTE_7570
Find type with same name as 'pType' if not found, create similair type in current project.
*/
Type* DataModelDoc::FindOrDuplicateType(Type* pType)
{//@CODE_7570
    Type* value = FindType(pType->GetName());

    if (!value)
    {
        OtherType* pOtherType = dynamic_cast<OtherType*>(pType);
        ExternClass* pExternClass = dynamic_cast<ExternClass*>(pType);
        if (pOtherType)
        {
            value = new OtherType(this, pOtherType);
        }
        else if (pExternClass)
        {
            value = new ExternClass(this, pExternClass);
        }
    }
    
    return value;
}//@CODE_7570


TreeViewModel* DataModelDoc::FindTreeViewModel(Gti* subTree)
{//@CODE_40930
    TreeViewModelIterator iTreeViewModel(this);
    while (++iTreeViewModel)
    {
        if (subTree == iTreeViewModel->GetSubTree())
        {
            return iTreeViewModel;
        }
    }

    return 0;
}//@CODE_40930


Type* DataModelDoc::FindType(const CbString& rName)
{//@CODE_1284
    TypeIterator iType(this);
    while (++iType)
    {
        if (rName == iType->GetName())
        {
            return iType;
        }
    }

    return 0;
}//@CODE_1284


UndoBase* DataModelDoc::GetLastMarked()
{//@CODE_35410
    // Return the most recent *level-1* (user-visible) undo marker only;
    // skip level-2 sub-batch markers, which are internal to an open
    // undo session and don't represent a new user step. Callers like
    // NoteShapePoint use the returned pointer to detect when a fresh
    // user step has opened — sub-markers would give a false positive.
    UndoBaseIterator iUndoBase(this);
    while (--iUndoBase)
    {
        if (iUndoBase->GetLast() == 1)
        {
            return iUndoBase;
        }
    }

    return 0;
}//@CODE_35410


/*@NOTE_3166
Return string containing the path to the document where also the source files are
placed.
*/
CbString DataModelDoc::GetPath()
{//@CODE_3166
    CbString value = GetDocument()->GetPathName();

    int bs = value.ReverseFind('\\');
    int fs = value.ReverseFind('/');
    int index = (bs > fs) ? bs : fs;
    if (index != -1)
    {
        value = value.Left(index);
    }

    if (value.GetLength() == 2 && value[1] == ':')
    {
        value += "\\";
    }

    return value;
}//@CODE_3166


CbString DataModelDoc::GetPathName()
{//@CODE_40951
    return GetDocument()->GetPathName();
}//@CODE_40951


CbString DataModelDoc::GetTitle()
{//@CODE_40950
    return GetDocument()->GetTitle();
}//@CODE_40950


bool DataModelDoc::HasNonCSymbols(const CbString& str)
{//@CODE_23053
    for (int i = 0; i < str.GetLength(); i++)
    {
        if (!__iscsym(str[i]) && _additionalAllowed.Find(str[i]) == -1)
            return true;
    }

    return false;
}//@CODE_23053


void DataModelDoc::LockAllViews()
{//@CODE_40966
    GetDocument()->LockAllViews();
}//@CODE_40966


/*@NOTE_5096
Group the undo's on the undo stack by marking the last on stack as last,  any new object
placed on  stack will be part of a new undoable action. An undo or a redo will always
undo or redo the whole group. The pointer to the last marked object is returned, this
pointer can be used as argument to the Undo method.

*/
UndoBase* DataModelDoc::MarkLastUndo(int level)
{//@CODE_5096
    // Two-level markers:
    //   level == 1 -- end of a user-visible undo step (the boundary Undo/
    //                Redo walks back to). Counts toward _maxUndoCount.
    //   level == 2 -- internal sub-step boundary inside an open user step.
    //                Stops the SaveState dedup walk (so repeated SaveState on
    //                the same object across sub-steps produces fresh snapshots),
    //                but Undo/Redo walks past it. Used to make multi-swap list
    //                reorders undoable by atomising each swap into its own
    //                sub-batch.
    //
    // A later MarkLastUndo(1) upgrades a level-2 marker to level-1 on the same
    // UndoBase; lower-level calls cannot downgrade.

    // Paint-neutrality: recompute diagram geometry HERE, at the operation
    // boundary, instead of inside Draw. RecalculateRect -> SetRect self-gates
    // (no size change -> no SaveState), so this is a free no-op when nothing
    // moved; when a class box does resize, the connection reroute it triggers
    // is pushed onto the stack NOW -- before we set the level-1 marker below --
    // so edit + reroute fold into ONE undo step. Skipped during replay:
    // segments are restored from their own entries then, never recomputed.
    if (level == 1 && !GetIsUndoing() && !GetIsRedoing())
        RecalculateAllDiagrams();

    UndoBase* pTop = GetLastUndoBase();
    if (!pTop) return NULL;

    int oldLevel = pTop->GetLast();
    if (level <= oldLevel) return pTop;

    pTop->SetLast(level);

    if (level == 1 && oldLevel != 1)
    {
        _currentUndoCount++;

        // Make sure we do not keep too many things on the stack.
        UndoBaseIterator iUndoBase(this);
        while (++iUndoBase && _currentUndoCount > _maxUndoCount)
        {
            if (iUndoBase->GetLast() == 1)
            {
                _currentUndoCount--;
            }
            delete iUndoBase;
        }
    }

    return pTop;
}//@CODE_5096


void DataModelDoc::NotifyCdViews()
{//@CODE_41372
    GetDocument()->NotifyCdViews();
}//@CODE_41372


void DataModelDoc::NotifySdViews()
{//@CODE_41373
    GetDocument()->NotifySdViews();
}//@CODE_41373


void DataModelDoc::NotifyStateChanged()
{//@CODE_40941
    if (GetDocument())
        GetDocument()->notifyStateChanged();
}//@CODE_40941


void DataModelDoc::NotifyStructureChanged()
{//@CODE_40961
    GetDocument()->NotifyStructureChanged();
}//@CODE_40961


void DataModelDoc::NotifyTreeViews()
{//@CODE_41371
    GetDocument()->NotifyTreeViews();
}//@CODE_41371


int DataModelDoc::OnSaveDocument(const char* path)
{//@CODE_40952
    return GetDocument()->OnSaveDocument(path);
}//@CODE_40952


/*@NOTE_41186
Recompute the derived geometry of every class diagram -- box sizes and the
connection reroutes a resize triggers. Changes the model; does NOT draw it.
Called at the edit boundary (DataModelDoc::MarkLastUndo, gated off during
replay/paint) and once right after a document loads (CClassBuilderDoc::
OnOpenDocument, before DeleteAllUndoBase). The load call refreshes stored-stale
geometry to canonical BEFORE any user edit -- without it the first edit's
recompute folds spurious segment churn onto that edit's own undo step (even a
plain selection), which then trips the list-integrity assert on undo.
*/
void DataModelDoc::RecalculateAllDiagrams(bool cd, bool sd)
{//@CODE_41186
    // cd / sd default true (load + MarkLastUndo recompute both); undo/redo passes
    // the touched-view flags so an undo of a CD-only edit does not re-derive SD.
    if (cd)
    {
        DataModelDoc::ClassDiagramIterator iClassDiagram(this);
        while (++iClassDiagram)
            iClassDiagram->RecalculateDiagram();
    }

    // SD geometry is derived: re-derive via RecalculateDiagram (recompute only, no
    // note-follow). No UpdateSequenceDiagramViews here -- the flush repaints.
    if (sd)
    {
        DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
        while (++iSequenceDiagram)
            iSequenceDiagram->RecalculateDiagram();
    }
}//@CODE_41186


/*@NOTE_5094
Redo the last Undo, returns the number of objects redoed.
*/
int DataModelDoc::Redo()
{//@CODE_5094
    int result = 0;

    {
        // RAII view lock + wait cursor (replaces the raw Lock/Unlock +
        // cursor pair). The block scope keeps today's ordering: the dtor's
        // coalesced refresh runs while _isRedoing is still set.
        CbViewLock lock(this);
        _isRedoing = 1;
        bool tree = false, cd = false, sd = false;
        RedoBaseIterator iRedoBase(this);
        // Walk past sub-markers (_last == 2); only level-1 markers bound
        // the user-visible redo step. See MarkLastUndo for the two-level
        // marker design.
        while (--iRedoBase && iRedoBase->GetLast() != 1)
        {
            iRedoBase->AccumulateTouches(tree, cd, sd);
            iRedoBase->Restore();
            result++;
        }

        if (iRedoBase)
        {
            iRedoBase->AccumulateTouches(tree, cd, sd);
            iRedoBase->Restore();
            result++;
        }

        if (result)
        {
            _currentUndoCount++;

            // Re-derive geometry the snapshot couldn't restore (see Undo): the
            // member/method text positions are derived, not snapshotted. Still
            // inside _isRedoing, so inner SaveStates stay gated.
            RecalculateAllDiagrams(cd, sd);

            // Derived refresh: a redo that restored anything refreshes by
            // construction (flag now, the lock dtor flushes). Kick each view
            // type any restored entry in the batch touches.
            if (tree)
                NotifyTreeViews();
            if (cd)
                NotifyCdViews();
            if (sd)
                NotifySdViews();
        }
    }
    _isRedoing = 0;

    return result;
}//@CODE_5094


/*@NOTE_5874
Refresh the objectId's from all DataModelDocObjec's and set member _nextObjectId
at a valid value.
*/
void DataModelDoc::RefreshObjectIds()
{//@CODE_5874
    _nextObjectId = 0;
    DataModelDocObjectIterator iDataModelDocObject(this);
    while (++iDataModelDocObject)
    {
        iDataModelDocObject->_id = _nextObjectId++;
    }
}//@CODE_5874


void DataModelDoc::ReplaceReference(ConnectionSegment* pOld,
                                    ConnectionSegment* pNew)
{//@CODE_41196
    if (_firstDataModelDocObject == pOld) _firstDataModelDocObject = pNew;
    if (_lastDataModelDocObject == pOld) _lastDataModelDocObject = pNew;
}//@CODE_41196


/*@NOTE_5814
Same as Undo, but puts nothing on the Redo stack.
*/
int DataModelDoc::RollBack(UndoBase* pUndoBase, bool silent)
{//@CODE_5814
    // RAII view lock + wait cursor; the silent path passes null = deliberate
    // no-op lock (the case CbViewLock's null-doc support was designed for).
    // The dtor's coalesced refresh runs at return, where the raw Unlock sat.
    CbViewLock lock(silent ? (DataModelDoc*)0 : this);
    _isUndoing = 1;

    // Remember the current situtation of the redo stack
    RedoBase* pLastRedoBase = GetLastRedoBase();

    // Undo the actions, tracking the batch kind for the derived refresh.
    bool tree = false, cd = false, sd = false;
    int result = 0;
    if (pUndoBase)
    {
        UndoBaseIterator iUndoBase(this);
        while (--iUndoBase && iUndoBase.Get() != pUndoBase)
        {
            iUndoBase->AccumulateTouches(tree, cd, sd);
            iUndoBase->Restore();
            result++;
        }
    }
    else
    {
        UndoBaseIterator iUndoBase(this);
        if (--iUndoBase)
        {
            // If this session was already counted, decrement counter
            if (iUndoBase->GetLast() == 1)
                _currentUndoCount--;

            iUndoBase->AccumulateTouches(tree, cd, sd);
            iUndoBase->Restore();
            result++;
            // Walk past sub-markers (_last == 2); stop at level-1.
            while (--iUndoBase && iUndoBase->GetLast() != 1)
            {
                iUndoBase->AccumulateTouches(tree, cd, sd);
                iUndoBase->Restore();
                result++;
            }
        }
    }

    // Real rollbacks re-derive geometry like Undo (member/method text positions
    // are derived, not restored); silent drag-preview rollbacks skip it -- the
    // caller paints its own preview. Still inside _isUndoing, so SaveStates stay
    // gated.
    if (!silent && result)
        RecalculateAllDiagrams(cd, sd);
   _isUndoing = 0;

    // Bring redo stack back in previous position
    RedoBaseIterator iRedoBase(this);
    while (--iRedoBase && iRedoBase.Get() != pLastRedoBase)
    {
        delete iRedoBase;
    }

    // See Undo(): a rollback that restored anything refreshes every view by
    // construction. NOT on the silent path -- silent rollbacks are the
    // per-mouse-move ghost rebuilds (drag previews), which must stay free of
    // view refreshes; their callers paint their own preview.
    if (!silent && result)
    {
        if (tree)
            NotifyTreeViews();
        if (cd)
            NotifyCdViews();
        if (sd)
            NotifySdViews();
    }

    return result;   // lock dtor: coalesced refresh + arrow cursor (non-silent)
}//@CODE_5814


/*@NOTE_34866
Save the state of the current object, it is checked if it isn't already on
stack in the last open undo session.
*/
void DataModelDoc::SaveState()
{//@CODE_34866
    if (!GetIsUndoing() && !GetIsRedoing())
    {
        (void)new UndoChangeDoc(this);
        GetDocument()->SetModifiedFlag();

        // Derived refresh: the doc-settings snapshot is the ambiguous case
        // (colours feed diagrams, other fields elsewhere) -- conservative full
        // refresh, it is a rare operation.
        NotifyStructureChanged();
    }
}//@CODE_34866


/*@NOTE_37878
Memory-stream snapshot used by UndoChangeDoc/RedoChangeDoc.
Sets `_membersOnly` so the codegen-emitted Serialize body skips
SERIALIZE_ALL_OBJECTS, then delegates to Serialize.
*/
void DataModelDoc::SerializeMembersOnly(CbArchive& archive)
{//@CODE_37878
    _membersOnly = true;
    Serialize(archive);
    _membersOnly = false;
}//@CODE_37878


void DataModelDoc::SetModifiedFlag(int bModified)
{//@CODE_40954
    GetDocument()->SetModifiedFlag(bModified);
}//@CODE_40954


/*@NOTE_5093
Undo the last recorded change, returns the number of objects undoed. If argument is 
supplied al is ondone until the marker.
*/
int DataModelDoc::Undo(UndoBase* pUndoBase)
{//@CODE_5093
    int result = 0;

    {
        // RAII view lock + wait cursor (replaces the raw Lock/Unlock +
        // cursor pair). The block scope keeps today's ordering: the dtor's
        // coalesced refresh runs while _isUndoing is still set.
        CbViewLock lock(this);
        _isUndoing = 1;
        bool tree = false, cd = false, sd = false;

        if (pUndoBase)
        {
            UndoBaseIterator iUndoBase(this);
            while (--iUndoBase && iUndoBase.Get() != pUndoBase)
            {
                iUndoBase->AccumulateTouches(tree, cd, sd);
                iUndoBase->Restore();
                result++;
            }
        }
        else
        {
            UndoBaseIterator iUndoBase(this);
            if (--iUndoBase)
            {
                iUndoBase->AccumulateTouches(tree, cd, sd);
                iUndoBase->Restore();
                result++;
                // Walk past sub-markers (_last == 2); stop at level-1.
                while (--iUndoBase && iUndoBase->GetLast() != 1)
                {
                    iUndoBase->AccumulateTouches(tree, cd, sd);
                    iUndoBase->Restore();
                    result++;
                }
            }
        }

        if (result)
        {
            _currentUndoCount--;

            // Re-derive geometry the snapshot couldn't restore: member/method
            // text positions are DERIVED (base Shape::SetRect, never
            // SaveState'd), so restoring the box leaves the text where the
            // edit-kick put it. _isUndoing is still set -> every inner SaveState
            // is gated (no new entries); the restored boxes are canonical -> no
            // segment reroute fires, the members just re-flow onto them.
            RecalculateAllDiagrams(cd, sd);

            // Derived refresh: an undo that restored anything must refresh by
            // construction (snapshot restores bypass the setters). Kick each
            // view type any restored entry in the batch touches. Flags while
            // the lock is held -- the dtor fires the ONE coalesced flush.
            if (tree)
                NotifyTreeViews();
            if (cd)
                NotifyCdViews();
            if (sd)
                NotifySdViews();
        }
    }
    _isUndoing = 0;

    return result;
}//@CODE_5093


void DataModelDoc::UnLockAndUpdateAllViews()
{//@CODE_40967
    GetDocument()->UnLockAndUpdateAllViews();
}//@CODE_40967


void DataModelDoc::UpdateClassDiagramViews()
{//@CODE_41384
    // Repaint every open class-diagram canvas. The model owns its diagrams, so
    // the iteration lives here -- mirrors UpdateTreeViews, giving the framework
    // FlushQueuedRefresh three symmetric DataModelDoc calls (tree/cd/sd).
    ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        iClassDiagram->UpdateClassDiagramViews();
    }
}//@CODE_41384


void DataModelDoc::UpdateSequenceDiagramViews()
{//@CODE_41385
    // Repaint every open sequence-diagram canvas (see UpdateClassDiagramViews).
    SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        iSequenceDiagram->UpdateSequenceDiagramViews();
    }
}//@CODE_41385


void DataModelDoc::UpdateTreeViews()
{//@CODE_40968
    TreeViewModelIterator iTreeViewModel(this);
    while (++iTreeViewModel)
    {
        iTreeViewModel->Refresh();
    }
}//@CODE_40968


/*@NOTE_34876
Set the value of member '_activationNoMethodPenColor' to 'activationNoMethodPenColor'.
*/
void DataModelDoc::SetActivationNoMethodPenColor(CbColorRef activationNoMethodPenColor)
{//@CODE_34876
    if (_activationNoMethodPenColor != activationNoMethodPenColor)
    {
        SaveState();
        _activationNoMethodPenColor = activationNoMethodPenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            ChildActivationShape* pChildActivationShape = 
                dynamic_cast<ChildActivationShape*>(iSequenceDiagramShape.Get());
            if (pChildActivationShape && activationNoMethodPenColor !=
                pChildActivationShape->GetActivationNoMethodPenColor())
            {
                pChildActivationShape->SaveState(1);
                pChildActivationShape->SetActivationNoMethodPenColor(activationNoMethodPenColor);
            }
        }
    }
}//@CODE_34876


/*@NOTE_34880
Set the value of member '_activationPenColor' to 'activationPenColor'.
*/
void DataModelDoc::SetActivationPenColor(CbColorRef activationPenColor)
{//@CODE_34880
    if (_activationPenColor != activationPenColor)
    {
        SaveState();
        _activationPenColor = activationPenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            ChildActivationShape* pChildActivationShape = 
                dynamic_cast<ChildActivationShape*>(iSequenceDiagramShape.Get());
            if (pChildActivationShape && activationPenColor !=
                pChildActivationShape->Shape::GetPenColor())
            {
                pChildActivationShape->SaveState(1);
                pChildActivationShape->Shape::SetPenColor(activationPenColor);
            }
        }
    }
}//@CODE_34880


/*@NOTE_35355
Set the value of member '_activationUser3PenColor' to 'activationUser3PenColor'.
*/
void DataModelDoc::SetActivationUser3PenColor(CbColorRef activationUser3PenColor)
{//@CODE_35355
    if (_activationUser3PenColor != activationUser3PenColor)
    {
        SaveState();
        _activationUser3PenColor = activationUser3PenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            ChildActivationShape* pChildActivationShape = 
                dynamic_cast<ChildActivationShape*>(iSequenceDiagramShape.Get());
            if (pChildActivationShape && activationUser3PenColor !=
                pChildActivationShape->GetActivationUser3PenColor())
            {
                pChildActivationShape->SaveState(1);
                pChildActivationShape->SetActivationUser3PenColor(activationUser3PenColor);
            }
        }
    }
}//@CODE_35355


/*@NOTE_34932
Set the value of member '_classPenColor' to 'classPenColor'.
*/
void DataModelDoc::SetClassPenColor(CbColorRef classPenColor)
{//@CODE_34932
    if (_classPenColor != classPenColor)
    {
        SaveState();
        _classPenColor = classPenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            ClassShape* pClassShape = 
                dynamic_cast<ClassShape*>(iClassDiagramShape.Get());
            if (pClassShape && classPenColor !=
                pClassShape->Shape::GetPenColor())
            {
                pClassShape->SaveState(1);
                pClassShape->Shape::SetPenColor(classPenColor);
            }
        }
    }
}//@CODE_34932


/*@NOTE_34937
Set the value of member '_classTextColor' to 'classTextColor'.
*/
void DataModelDoc::SetClassTextColor(CbColorRef classTextColor)
{//@CODE_34937
    if (_classTextColor != classTextColor)
    {
        SaveState();
        _classTextColor = classTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            ClassShape* pClassShape = 
                dynamic_cast<ClassShape*>(iClassDiagramShape.Get());
            if (pClassShape && classTextColor !=
                pClassShape->Shape::GetTextColor())
            {
                pClassShape->SaveState(1);
                pClassShape->Shape::SetTextColor(classTextColor);
            }
        }
    }
}//@CODE_34937


/*@NOTE_35042
Set the value of member '_criticalRelationPenColor' to 'criticalRelationPenColor'.
*/
void DataModelDoc::SetCriticalRelationPenColor(CbColorRef criticalRelationPenColor)
{//@CODE_35042
    if (_criticalRelationPenColor != criticalRelationPenColor)
    {
        SaveState();
        _criticalRelationPenColor = criticalRelationPenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            RelationShape* pRelationShape = 
                dynamic_cast<RelationShape*>(iClassDiagramShape.Get());
            if (pRelationShape && criticalRelationPenColor !=
                pRelationShape->GetCriticalPenColor())
            {
                pRelationShape->SaveState(1);
                pRelationShape->SetCriticalPenColor(criticalRelationPenColor);
            }
        }
    }
}//@CODE_35042


/*@NOTE_34958
Set the value of member '_dependencyPenColor' to 'dependencyPenColor'.
*/
void DataModelDoc::SetDependencyPenColor(CbColorRef dependencyPenColor)
{//@CODE_34958
    if (_dependencyPenColor != dependencyPenColor)
    {
        SaveState();
        _dependencyPenColor = dependencyPenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            DependencyShape* pDependencyShape = 
                dynamic_cast<DependencyShape*>(iClassDiagramShape.Get());
            if (pDependencyShape && dependencyPenColor !=
                pDependencyShape->Shape::GetPenColor())
            {
                pDependencyShape->SaveState(1);
                pDependencyShape->Shape::SetPenColor(dependencyPenColor);
            }
        }
    }
}//@CODE_34958


/*@NOTE_34961
Set the value of member '_dependencyTextColor' to 'dependencyTextColor'.
*/
void DataModelDoc::SetDependencyTextColor(CbColorRef dependencyTextColor)
{//@CODE_34961
    if (_dependencyTextColor != dependencyTextColor)
    {
        SaveState();
        _dependencyTextColor = dependencyTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            DependencyShape* pDependencyShape = 
                dynamic_cast<DependencyShape*>(iClassDiagramShape.Get());
            if (pDependencyShape && dependencyTextColor !=
                pDependencyShape->Shape::GetTextColor())
            {
                pDependencyShape->SaveState(1);
                pDependencyShape->Shape::SetTextColor(dependencyTextColor);
            }
        }
    }
}//@CODE_34961


/*@NOTE_34998
Set the value of member '_inheritPenColor' to 'inheritPenColor'.
*/
void DataModelDoc::SetInheritPenColor(CbColorRef inheritPenColor)
{//@CODE_34998
    if (_inheritPenColor != inheritPenColor)
    {
        SaveState();
        _inheritPenColor = inheritPenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            InheritShape* pInheritShape = 
                dynamic_cast<InheritShape*>(iClassDiagramShape.Get());
            if (pInheritShape && inheritPenColor !=
                pInheritShape->Shape::GetPenColor())
            {
                pInheritShape->SaveState(1);
                pInheritShape->Shape::SetPenColor(inheritPenColor);
            }
        }
    }
}//@CODE_34998


/*@NOTE_34884
Set the value of member '_lifeLinePenColor' to 'lifeLinePenColor'.
*/
void DataModelDoc::SetLifeLinePenColor(CbColorRef lifeLinePenColor)
{//@CODE_34884
    if (_lifeLinePenColor != lifeLinePenColor)
    {
        SaveState();
        _lifeLinePenColor = lifeLinePenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            LifeLineShape* pLifeLineShape = 
                dynamic_cast<LifeLineShape*>(iSequenceDiagramShape.Get());
            if (pLifeLineShape && lifeLinePenColor !=
                pLifeLineShape->Shape::GetPenColor())
            {
                pLifeLineShape->SaveState(1);
                pLifeLineShape->Shape::SetPenColor(lifeLinePenColor);
            }
        }
    }
}//@CODE_34884


/*@NOTE_34888
Set the value of member '_lifeLineTextColor' to 'lifeLineTextColor'.
*/
void DataModelDoc::SetLifeLineTextColor(CbColorRef lifeLineTextColor)
{//@CODE_34888
    if (_lifeLineTextColor != lifeLineTextColor)
    {
        SaveState();
        _lifeLineTextColor = lifeLineTextColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            LifeLineShape* pLifeLineShape = 
                dynamic_cast<LifeLineShape*>(iSequenceDiagramShape.Get());
            if (pLifeLineShape && lifeLineTextColor !=
                pLifeLineShape->Shape::GetTextColor())
            {
                pLifeLineShape->SaveState(1);
                pLifeLineShape->Shape::SetTextColor(lifeLineTextColor);
            }
        }
    }
}//@CODE_34888


/*@NOTE_34941
Set the value of member '_memberTextColor' to 'memberTextColor'.
*/
void DataModelDoc::SetMemberTextColor(CbColorRef memberTextColor)
{//@CODE_34941
    if (_memberTextColor != memberTextColor)
    {
        SaveState();
        _memberTextColor = memberTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            MemberShape* pMemberShape = 
                dynamic_cast<MemberShape*>(iClassDiagramShape.Get());
            if (pMemberShape && memberTextColor !=
                pMemberShape->Shape::GetTextColor())
            {
                pMemberShape->SaveState(1);
                pMemberShape->Shape::SetTextColor(memberTextColor);
            }
        }
    }
}//@CODE_34941


/*@NOTE_34945
Set the value of member '_methodTextColor' to 'methodTextColor'.
*/
void DataModelDoc::SetMethodTextColor(CbColorRef methodTextColor)
{//@CODE_34945
    if (_methodTextColor != methodTextColor)
    {
        SaveState();
        _methodTextColor = methodTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            MethodShape* pMethodShape = 
                dynamic_cast<MethodShape*>(iClassDiagramShape.Get());
            if (pMethodShape && methodTextColor !=
                pMethodShape->Shape::GetTextColor())
            {
                pMethodShape->SaveState(1);
                pMethodShape->Shape::SetTextColor(methodTextColor);
            }
        }
    }
}//@CODE_34945


unsigned int DataModelDoc::GetNextObjectId()
{//@CODE_1162
    return _nextObjectId++;
}//@CODE_1162


/*@NOTE_35052
Set the value of member '_noteShapePenColor' to 'noteShapePenColor'.
*/
void DataModelDoc::SetNoteShapePenColor(CbColorRef noteShapePenColor)
{//@CODE_35052
    if (_noteShapePenColor != noteShapePenColor)
    {
        SaveState();
        _noteShapePenColor = noteShapePenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            NoteShape* pNoteShape = 
                dynamic_cast<NoteShape*>(iClassDiagramShape.Get());
            if (pNoteShape && noteShapePenColor !=
                pNoteShape->Shape::GetPenColor())
            {
                pNoteShape->SaveState(1);
                pNoteShape->Shape::SetPenColor(noteShapePenColor);
            }
        }
    }
}//@CODE_35052


/*@NOTE_35056
Set the value of member '_noteShapeTextColor' to 'noteShapeTextColor'.
*/
void DataModelDoc::SetNoteShapeTextColor(CbColorRef noteShapeTextColor)
{//@CODE_35056
    if (_noteShapeTextColor != noteShapeTextColor)
    {
        SaveState();
        _noteShapeTextColor = noteShapeTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            NoteShape* pNoteShape = 
                dynamic_cast<NoteShape*>(iClassDiagramShape.Get());
            if (pNoteShape && noteShapeTextColor !=
                pNoteShape->Shape::GetTextColor())
            {
                pNoteShape->SaveState(1);
                pNoteShape->Shape::SetTextColor(noteShapeTextColor);
            }
        }
    }
}//@CODE_35056


void DataModelDoc::SetDocument(CClassBuilderDoc* pPDoc)
{//@CODE_1157
    _pDoc = pPDoc;
}//@CODE_1157


CClassBuilderDoc* DataModelDoc::GetDocument()
{//@CODE_1156
    return _pDoc;
}//@CODE_1156


/*@NOTE_34994
Set the value of member '_relationDiagramOnlyPenColor' to 'relationDiagramOnlyPenColor'.
*/
void DataModelDoc::SetRelationDiagramOnlyPenColor(CbColorRef relationDiagramOnlyPenColor)
{//@CODE_34994
    if (_relationDiagramOnlyPenColor != relationDiagramOnlyPenColor)
    {
        SaveState();
        _relationDiagramOnlyPenColor = relationDiagramOnlyPenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            RelationDiagramOnlyShape* pRelationDiagramOnlyShape = 
                dynamic_cast<RelationDiagramOnlyShape*>(iClassDiagramShape.Get());
            if (pRelationDiagramOnlyShape && relationDiagramOnlyPenColor !=
                pRelationDiagramOnlyShape->Shape::GetPenColor())
            {
                pRelationDiagramOnlyShape->SaveState(1);
                pRelationDiagramOnlyShape->Shape::SetPenColor(relationDiagramOnlyPenColor);
            }
        }
    }
}//@CODE_34994


/*@NOTE_34966
Set the value of member '_relationDiagramOnlyTextColor' to 'relationDiagramOnlyTextColor'.
*/
void DataModelDoc::SetRelationDiagramOnlyTextColor(CbColorRef relationDiagramOnlyTextColor)
{//@CODE_34966
    if (_relationDiagramOnlyTextColor != relationDiagramOnlyTextColor)
    {
        SaveState();
        _relationDiagramOnlyTextColor = relationDiagramOnlyTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            RelationDiagramOnlyShape* pRelationDiagramOnlyShape = 
                dynamic_cast<RelationDiagramOnlyShape*>(iClassDiagramShape.Get());
            if (pRelationDiagramOnlyShape && relationDiagramOnlyTextColor !=
                pRelationDiagramOnlyShape->Shape::GetTextColor())
            {
                pRelationDiagramOnlyShape->SaveState(1);
                pRelationDiagramOnlyShape->Shape::SetTextColor(relationDiagramOnlyTextColor);
            }
        }
    }
}//@CODE_34966


/*@NOTE_34949
Set the value of member '_dependencyTextColor' to 'relationPenColor'.
*/
void DataModelDoc::SetRelationPenColor(CbColorRef relationPenColor)
{//@CODE_34949
    if (_relationPenColor != relationPenColor)
    {
        SaveState();
        _relationPenColor = relationPenColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            RelationShape* pRelationShape = 
                dynamic_cast<RelationShape*>(iClassDiagramShape.Get());
            if (pRelationShape && relationPenColor !=
                pRelationShape->Shape::GetPenColor())
            {
                pRelationShape->SaveState(1);
                pRelationShape->Shape::SetPenColor(relationPenColor);
            }
        }
    }
}//@CODE_34949


/*@NOTE_34953
Set the value of member '_dependencyTextColor' to 'relationTextColor'.
*/
void DataModelDoc::SetRelationTextColor(CbColorRef relationTextColor)
{//@CODE_34953
    if (_relationTextColor != relationTextColor)
    {
        SaveState();
        _relationTextColor = relationTextColor;
    }
    
    DataModelDoc::ClassDiagramIterator iClassDiagram(this);
    while (++iClassDiagram)
    {
        ClassDiagram::ClassDiagramShapeIterator 
            iClassDiagramShape(iClassDiagram);
        while (++iClassDiagramShape)
        {
            RelationShape* pRelationShape = 
                dynamic_cast<RelationShape*>(iClassDiagramShape.Get());
            if (pRelationShape && relationTextColor !=
                pRelationShape->Shape::GetTextColor())
            {
                pRelationShape->SaveState(1);
                pRelationShape->Shape::SetTextColor(relationTextColor);
            }
        }
    }
}//@CODE_34953


/*@NOTE_34892
Set the value of member '_SDNoteShapePenColor' to 'SDNoteShapePenColor'.
*/
void DataModelDoc::SetSDNoteShapePenColor(CbColorRef SDNoteShapePenColor)
{//@CODE_34892
    if (_SDNoteShapePenColor != SDNoteShapePenColor)
    {
        SaveState();
        _SDNoteShapePenColor = SDNoteShapePenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            SDNoteShape* pSDNoteShape = 
                dynamic_cast<SDNoteShape*>(iSequenceDiagramShape.Get());
            if (pSDNoteShape && SDNoteShapePenColor !=
                pSDNoteShape->Shape::GetPenColor())
            {
                pSDNoteShape->SaveState(1);
                pSDNoteShape->Shape::SetPenColor(SDNoteShapePenColor);
            }
        }
    }
}//@CODE_34892


/*@NOTE_34896
Set the value of member '_SDNoteShapeTextColor' to 'SDNoteShapeTextColor'.
*/
void DataModelDoc::SetSDNoteShapeTextColor(CbColorRef SDNoteShapeTextColor)
{//@CODE_34896
    if (_SDNoteShapeTextColor != SDNoteShapeTextColor)
    {
        SaveState();
        _SDNoteShapeTextColor = SDNoteShapeTextColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            SDNoteShape* pSDNoteShape = 
                dynamic_cast<SDNoteShape*>(iSequenceDiagramShape.Get());
            if (pSDNoteShape && SDNoteShapeTextColor !=
                pSDNoteShape->Shape::GetTextColor())
            {
                pSDNoteShape->SaveState(1);
                pSDNoteShape->Shape::SetTextColor(SDNoteShapeTextColor);
            }
        }
    }
}//@CODE_34896


/*@NOTE_34900
Set the value of member '_signalNoMethodPenColor' to 'signalNoMethodPenColor'.
*/
void DataModelDoc::SetSignalNoMethodPenColor(CbColorRef signalNoMethodPenColor)
{//@CODE_34900
    if (_signalNoMethodPenColor != signalNoMethodPenColor)
    {
        SaveState();
        _signalNoMethodPenColor = signalNoMethodPenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            SignalShape* pSignalShape = 
                dynamic_cast<SignalShape*>(iSequenceDiagramShape.Get());
            if (pSignalShape && signalNoMethodPenColor !=
                pSignalShape->GetSignalNoMethodPenColor())
            {
                pSignalShape->SaveState(1);
                pSignalShape->SetSignalNoMethodPenColor(signalNoMethodPenColor);
            }
        }
    }
}//@CODE_34900


/*@NOTE_34904
Set the value of member '_signalPenColor' to 'signalPenColor'.
*/
void DataModelDoc::SetSignalPenColor(CbColorRef signalPenColor)
{//@CODE_34904
    _signalPenColor = signalPenColor;
    if (_signalPenColor != signalPenColor)
    {
        SaveState();
        _signalPenColor = signalPenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            SignalShape* pSignalShape = 
                dynamic_cast<SignalShape*>(iSequenceDiagramShape.Get());
            if (pSignalShape && signalPenColor !=
                pSignalShape->Shape::GetPenColor())
            {
                pSignalShape->SaveState(1);
                pSignalShape->Shape::SetPenColor(signalPenColor);
            }
        }
    }
}//@CODE_34904


/*@NOTE_34911
Set the value of member '_signalTextColor' to 'signalTextColor'.
*/
void DataModelDoc::SetSignalTextColor(CbColorRef signalTextColor)
{//@CODE_34911
    if (_signalTextColor != signalTextColor)
    {
        SaveState();
        _signalTextColor = signalTextColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            SignalShape* pSignalShape = 
                dynamic_cast<SignalShape*>(iSequenceDiagramShape.Get());
            if (pSignalShape && signalTextColor !=
                pSignalShape->Shape::GetTextColor())
            {
                pSignalShape->SaveState(1);
                pSignalShape->Shape::SetTextColor(signalTextColor);
            }
        }
    }
}//@CODE_34911


int DataModelDoc::GetVersion()
{//@CODE_1159
    return _version;
}//@CODE_1159


void DataModelDoc::SetVersion(int version)
{//@CODE_1160
    _version = version;
}//@CODE_1160


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_38
Method which must be called first in a constructor
*/
void DataModelDoc::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
    INIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
    INIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
    INIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
    INIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
    INIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)
}


/*@NOTE_40
Method which must be called first in a destructor
*/
void DataModelDoc::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
    EXIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
    EXIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
    EXIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
    EXIT_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
    EXIT_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)
}


/*@NOTE_43
Serialize the members only to a CbObject object
*/
void DataModelDoc::Serialize(CbArchive& archive)
{
    if (archive.IsStoring())
    {
        archive << _version;
        archive << _nextObjectId;
        archive << _maxUndoCount;
        archive << _additionalAllowed;
        archive << _activationNoMethodPenColor;
        archive << _activationPenColor;
        archive << _lifeLinePenColor;
        archive << _lifeLineTextColor;
        archive << _SDNoteShapePenColor;
        archive << _SDNoteShapeTextColor;
        archive << _signalNoMethodPenColor;
        archive << _signalPenColor;
        archive << _signalTextColor;
        archive << _classPenColor;
        archive << _classTextColor;
        archive << _memberTextColor;
        archive << _methodTextColor;
        archive << _relationPenColor;
        archive << _relationTextColor;
        archive << _dependencyPenColor;
        archive << _dependencyTextColor;
        archive << _relationDiagramOnlyPenColor;
        archive << _relationDiagramOnlyTextColor;
        archive << _inheritPenColor;
        archive << _criticalRelationPenColor;
        archive << _noteShapePenColor;
        archive << _noteShapeTextColor;
        archive << _activationUser3PenColor;
        archive << _lastSearch;
        archive << _matchCase;
        archive << _methodNameList;
        archive << _similarLinesList;
        archive << _activationInitialPenColor;
        archive << _commentInitialCode;
    }
    else
    {
        archive >> _version;
        DataModelDocObject::_objectVersion = _version;
        if (1 <= _version)
        {
            archive >> _nextObjectId;
            archive >> _maxUndoCount;
            archive >> _additionalAllowed;
            archive >> _activationNoMethodPenColor;
            archive >> _activationPenColor;
            archive >> _lifeLinePenColor;
            archive >> _lifeLineTextColor;
            archive >> _SDNoteShapePenColor;
            archive >> _SDNoteShapeTextColor;
            archive >> _signalNoMethodPenColor;
            archive >> _signalPenColor;
            archive >> _signalTextColor;
            archive >> _classPenColor;
            archive >> _classTextColor;
            archive >> _memberTextColor;
            archive >> _methodTextColor;
            archive >> _relationPenColor;
            archive >> _relationTextColor;
            archive >> _dependencyPenColor;
            archive >> _dependencyTextColor;
            archive >> _relationDiagramOnlyPenColor;
            archive >> _relationDiagramOnlyTextColor;
            archive >> _inheritPenColor;
            archive >> _criticalRelationPenColor;
            archive >> _noteShapePenColor;
            archive >> _noteShapeTextColor;
            archive >> _activationUser3PenColor;
            archive >> _lastSearch;
            archive >> _matchCase;
            archive >> _methodNameList;
            archive >> _similarLinesList;
            archive >> _activationInitialPenColor;
            archive >> _commentInitialCode;
        }
    }
    if (!_membersOnly)
    {
        SERIALIZE_ALL_OBJECTS(CbObject, DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject);
    }
}


/*@NOTE_42
Method which must be called first in a serialize constructor
*/
void DataModelDoc::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
    INIT_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
    INIT_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
    INIT_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
    INIT_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
    INIT_MULTI_ACTIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)
}


/*@NOTE_45
Serialize the relations to a CbObject object
*/
void DataModelDoc::SerializeRelations(CbArchive& archive,
                                      DataModelDocObject* pointerArray[])
{
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
        WRITE_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
        WRITE_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
        WRITE_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
        WRITE_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
        WRITE_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
    }
    else
    {
        if (1 <= _version)
        {
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
            READ_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
            READ_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
            READ_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
            READ_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
            READ_SINGLE_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(DataModelDoc)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
METHODS_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
METHODS_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
METHODS_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
METHODS_ITERATOR_NOFILTER_MULTI_ACTIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
METHODS_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
METHODS_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)
METHODS_ITERATOR_MULTI_ACTIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
