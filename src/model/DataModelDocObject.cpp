/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DataModelDocObject.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DataModelDocObject'
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
#include <iostream>
#include <sstream>
#include <typeinfo>
using namespace std;
#include "ClassBuilderDoc.h"
//@END_USER2


// Static members
int DataModelDocObject::_objectVersion = 0;


DataModelDocObject::DataModelDocObject(DataModelDoc* pDataModelDoc) //@INIT_55
    : CbObject()
    , _id(pDataModelDoc->GetNextObjectId())
{//@CODE_55
    ConstructorInclude(pDataModelDoc);

    // Put in your own code

    (void)new UndoNew(this);
}//@CODE_55


/*@NOTE_1552
Constructor needed for putting a new object in the old one's context
*/
DataModelDocObject::DataModelDocObject(DataModelDocObject* pOld) //@INIT_1552
   // Do nothing
{//@CODE_1552
    ReplaceConstructorInclude(pOld);

    _id = pOld->_id;

    (void)new UndoNew(this);
}//@CODE_1552


/*@NOTE_29
Constructor needed for serialization, not meant to use for other purposes!
*/
DataModelDocObject::DataModelDocObject() //@INIT_29
    : CbObject()
{//@CODE_29
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_29


/*@NOTE_27
Destructor method
*/
DataModelDocObject::~DataModelDocObject()
{//@CODE_27
    DestructorInclude();

    // Put in your own code
}//@CODE_27


/*@NOTE_35089
Copy the settings from one object to the other using serialisation.
*/
void DataModelDocObject::CopyState(DataModelDocObject* pDataModelDocObject)
{//@CODE_35089
    // Save id, it must be restored after the state copy
    int id = _id;

    // Snapshot the source object's state through a memory stream and
    // deserialize it back into 'this'. Members only — relations excluded.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive storeArchive(static_cast<std::ostream&>(ss));
        pDataModelDocObject->Serialize(storeArchive);
    }
    ss.seekg(0);
    {
        CbArchive loadArchive(static_cast<std::istream&>(ss));
        Serialize(loadArchive);
    }

    // Restore its own id
    _id = id;
}//@CODE_35089


/*@NOTE_5104
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void DataModelDocObject::Delete()
{//@CODE_5104
    GetDataModelDoc()->SetModifiedFlag();
    // Coalesce the whole delete cascade (this node + every UndoSubDelete'd
    // descendant, each of which would otherwise fire its own NotifyStructureChanged)
    // into a single view refresh, and show the wait cursor for its duration --
    // this is what was missing on the 1-2s relation delete. CbViewLock caches
    // the document pointer, so it survives RemoveReferences cutting this
    // object's doc backpointer inside UndoDelete; it nests safely when an outer
    // op (group delete, ...) already holds a lock.
    CbViewLock lock(GetDataModelDoc());

    // Derived refresh: kick each view type the deleted object touches (recorded
    // inside the lock, so they coalesce to one flush after the cascade).
    if (TouchesTree())
        GetDataModelDoc()->NotifyTreeViews();
    if (TouchesCd())
        GetDataModelDoc()->NotifyCdViews();
    if (TouchesSd())
        GetDataModelDoc()->NotifySdViews();

    (void)new UndoDelete(this);
}//@CODE_5104


/*@NOTE_22963
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void DataModelDocObject::Delete(DataModelDoc* pDataModelDoc)
{//@CODE_22963
    DataModelDoc::UndoBaseIterator iUndoBase(pDataModelDoc);
    while (--iUndoBase && !iUndoBase->GetLast())
    {
        // We are already on stack
        if (iUndoBase->GetDataModelDocObject() == this)
        {
            UndoNew* pUndoNew = dynamic_cast<UndoNew*>(iUndoBase.Get());
            if (pUndoNew)
            {
                delete pUndoNew;
                delete this;
                return;
            }
        }
    }
    
    pDataModelDoc->SetModifiedFlag();

    // Derived refresh: same kind rule as Delete().
    if (TouchesTree())
        pDataModelDoc->NotifyTreeViews();
    if (TouchesCd())
        pDataModelDoc->NotifyCdViews();
    if (TouchesSd())
        pDataModelDoc->NotifySdViews();

    (void)new UndoDelete(this);
}//@CODE_22963


int DataModelDocObject::FindStringInStr(CbString& str, const CbString& string,
                                        int offset)
{//@CODE_23225
    // Whole-identifier search from 'offset': reject a hit whose neighbouring
    // char is a C symbol (so "Color" doesn't match inside "ColorRef"). Search
    // in place via Find(start) -- the old str.Mid(offset) copied the whole
    // remainder on EVERY call, which dominated the cost of a type rename
    // (this runs 5x per name variant over every string in the model).
    const int strLen = str.GetLength();
    const int subLen = string.GetLength();
    while (1)
    {
        int index = str.Find(string, offset);
        if (index == -1)
            return -1;

        bool boundedBefore = (index == 0)           || !__iscsym(str[index - 1]);
        int  after         = index + subLen;
        bool boundedAfter  = (after >= strLen)       || !__iscsym(str[after]);

        if (boundedBefore && boundedAfter)
            return index;

        offset = index + 1;   // overlapping retry, same as before
    }
}//@CODE_23225


const CbString& DataModelDocObject::GetCaption()
{//@CODE_35159
    static CbString value("");

    return value;
}//@CODE_35159


CbString DataModelDocObject::GetContextList()
{//@CODE_27307
    CbString value;

    return value;
}//@CODE_27307


DataModelDocObject* DataModelDocObject::GetDataModelDocObject() const
{//@CODE_36729
    return 0;
}//@CODE_36729


/*@NOTE_4771
Convert note to Html format.
*/
CbString DataModelDocObject::GetHtmlNote()
{//@CODE_4771
    const CbString& note = GetNote();
    CbString html;

    for (int i = 0; i < note.GetLength(); i++)
    {
        if (note[i] == '&')
            html += "&amp;";
        else if (note[i] == '<')
            html += "&lt;";
        else if (note[i] == '>')
            html += "&gt;";
        else
            html += note[i];
    }

    int index = html.Find(NL NL);
    while (index != -1)
    {
        html = html.Left(index) + "<P>" + html.Mid(index+2);
        index = html.Find(NL NL);
    }

    return html;
}//@CODE_4771


/*@NOTE_7366
Convert the _id to a string and return it.
*/
CbString DataModelDocObject::GetIdAsString()
{//@CODE_7366
    CbString value;
    value.Format("%d", _id);

    return value;
}//@CODE_7366


/*@NOTE_3190
Return with indentSize * indentLevel spaces.
*/
CbString DataModelDocObject::GetIndent(int indentLevel)
{//@CODE_3190
    CbString value;

    DataModel* pDataModel = GetDataModelDoc()->GetDataModel();
    if (pDataModel)
    {
        int spaceCount = pDataModel->GetIndentSize() * indentLevel;
        for (int i = 0; i < spaceCount; i++)
            value += ' ';
    }
    
    return value;
}//@CODE_3190


/*@NOTE_4767
This version returns a empty string, but the overides, will do good.
*/
const CbString& DataModelDocObject::GetNote()
{//@CODE_4767
    static CbString value("");

    return value;
}//@CODE_4767


/*@NOTE_22885
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void DataModelDocObject::OnUndoRedoAdded()
{//@CODE_22885
}//@CODE_22885


/*@NOTE_22886
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void DataModelDocObject::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22886
    OnUndoRedoAdded();
}//@CODE_22886


/*@NOTE_22887
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called before the object changes state. This method calls
OnUndoRedoRemoving(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void DataModelDocObject::OnUndoRedoChanging(DataModelDocObject* pNewState)
{//@CODE_22887
    OnUndoRedoRemoving();
}//@CODE_22887


/*@NOTE_22932
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called after the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void DataModelDocObject::OnUndoRedoRemoved()
{//@CODE_22932
}//@CODE_22932


/*@NOTE_22888
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called before the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void DataModelDocObject::OnUndoRedoRemoving()
{//@CODE_22888
}//@CODE_22888


/*@NOTE_23219
Virtual method to replace strings at various places, called if a type name changes.
Returns a non zero value, if replacements have been made. Calls SaveState if valid
pointer 'saveState' is supplied.
*/
int DataModelDocObject::ReplaceInStr(CbString& str, const CbString& oldString,
                                     const CbString& newString, bool saveState)
{//@CODE_23219
    if (oldString.IsEmpty())
        return 0;

    int index = FindStringInStr(str, oldString, 0);
    if (index == -1)
        return 0;                       // no match -- leave str untouched, no alloc

    if (saveState)
        SaveState();

    // Build the result ONCE in a chunk-growing CbStringBuilder, appending the
    // gap-then-replacement for each hit. The old code rebuilt the WHOLE string
    // (Left + new + Mid, then reassign) on every match -- O(matches x length)
    // and a fresh allocation per hit. This is a single O(length) pass.
    const int oldLen = oldString.GetLength();
    CbStringBuilder result(str.GetLength() + 256);
    int copyFrom = 0;
    while (index != -1)
    {
        result += str.Mid(copyFrom, index - copyFrom);   // text before the hit
        result += newString;
        copyFrom = index + oldLen;
        index = FindStringInStr(str, oldString, copyFrom);
    }
    result += str.Mid(copyFrom);                         // trailing remainder

    str = result;
    return 1;                            // non-zero: replacements were made
}//@CODE_23219


/*@NOTE_23060
Virtual method to replace strings at various places, called if a type name changes.
*/
void DataModelDocObject::ReplaceInX(const CbString& oldString,
                                    const CbString& newString)
{//@CODE_23060
}//@CODE_23060


/*@NOTE_41187
Redirect this object's raw relation pointers from an old ConnectionSegment to its
replacement. Used while the old segment is raw-freed during a cap replacement, for
objects PARKED on the undo/redo stacks -- their _prev/_next/_first/_last are
severed from the live relation graph, so the normal relinking never reaches them.
See UndoBase::ChangeDataModelDocObject for the full reasoning and the sweep. Base
handles the DataModelDoc object-list links; ConnectionSegment and ConnectionShape
override to also fix their segment-list links.
*/
void DataModelDocObject::ReplaceReference(ConnectionSegment* pOld,
                                          ConnectionSegment* pNew)
{//@CODE_41187
    if (_prevDataModelDoc == pOld) _prevDataModelDoc = pNew;
    if (_nextDataModelDoc == pOld) _nextDataModelDoc = pNew;
}//@CODE_41187


/*@NOTE_5105
Save the state of the current object, it is checked if it isn't already on
stack in the last open undo session.
*/
void DataModelDocObject::SaveState(int always)
{//@CODE_5105
    if (!GetDataModelDoc()->GetIsUndoing() && !GetDataModelDoc()->GetIsRedoing())
    {
        if (!always)
        {
            DataModelDoc::UndoBaseIterator iUndoBase(GetDataModelDoc());
            while (--iUndoBase && !iUndoBase->GetLast())
            {
                // We are already on stack
                if (iUndoBase->GetDataModelDocObject() == this)
                    return;
            }
        }

        (void)new UndoChange(this);
        GetDataModelDoc()->SetModifiedFlag();

        // Derived refresh: recording the change IS the view notification --
        // kick each view type the changed object touches.
        if (TouchesTree())
            GetDataModelDoc()->NotifyTreeViews();
        if (TouchesCd())
            GetDataModelDoc()->NotifyCdViews();
        if (TouchesSd())
            GetDataModelDoc()->NotifySdViews();
    }
}//@CODE_5105


void DataModelDocObject::SetCaption(const CbString& rCaption)
{//@CODE_35160
}//@CODE_35160


/*@NOTE_4768
This is a dummy one, but the overrides, will make all up.
*/
void DataModelDocObject::SetNote(const CbString& rNote)
{//@CODE_4768
}//@CODE_4768


bool DataModelDocObject::TouchesCd() const
{//@CODE_41147
    // Does a change to this object need the CD canvases repainted? Yes for the
    // class-diagram shapes (ClassDiagramShape + descendants) and their helper
    // objects (connection segments + their Relation*StartSegment group, CD
    // note-attach points) -- AND for any tree object (Gti), because its CD
    // representation (a ClassShape, a relation line) draws from it and must
    // refresh. Conservative for Gti: e.g. a member edit need not really touch a
    // CD, but splitting those exceptions out is not worth it (JV).
    return (TouchesTree()
         || dynamic_cast<const ClassDiagram*>(this)
         || dynamic_cast<const ClassDiagramShape*>(this)
         || dynamic_cast<const ConnectionSegment*>(this)
         || dynamic_cast<const NoteShapePoint*>(this)) ? true : false;
}//@CODE_41147


bool DataModelDocObject::TouchesSd() const
{//@CODE_41148
    // Does a change to this object need the SD canvases repainted? Yes for the
    // sequence-diagram shapes (SequenceDiagramShape + descendants) and SD
    // note-attach points -- AND for any tree object (Gti), because its SD
    // representation (a lifeline, a signal's method) draws from it. Conservative
    // for Gti (a member edit never really touches an SD), exceptions not split.
    return (TouchesTree()
         || dynamic_cast<const SequenceDiagram*>(this)
         || dynamic_cast<const SequenceDiagramShape*>(this)
         || dynamic_cast<const SDNoteShapePoint*>(this)) ? true : false;
}//@CODE_41148


bool DataModelDocObject::TouchesTree() const
{//@CODE_41146
    return (dynamic_cast<const Gti*>(this) ? true : false);
}//@CODE_41146


unsigned int DataModelDocObject::GetId()
{//@CODE_1163
    return _id;
}//@CODE_1163


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5384
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void DataModelDocObject::CleanupReferences()
{
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_26
Method which must be called first in a constructor
*/
void DataModelDocObject::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_AVLTREE_OWNED_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_28
Method which must be called first in a destructor
*/
void DataModelDocObject::DestructorInclude()
{
    EXIT_AVLTREE_OWNED_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


Property* DataModelDocObject::FindEqualOrBiggerProperty(const CbString& name)
{
BODY_AVLTREE_FINDEQUALORBIGGER(GetName(), name, DataModelDocObject, DataModelDocObject, Property, Property)
}


Property* DataModelDocObject::FindEqualOrSmallerProperty(const CbString& name)
{
BODY_AVLTREE_FINDEQUALORSMALLER(GetName(), name, DataModelDocObject, DataModelDocObject, Property, Property)
}


Property* DataModelDocObject::FindProperty(const CbString& name)
{
BODY_AVLTREE_FIND(GetName(), name, DataModelDocObject, DataModelDocObject, Property, Property)
}


Property* DataModelDocObject::FindReverseProperty(const CbString& name)
{
BODY_AVLTREE_FINDREVERSE(GetName(), name, DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_41450
Method that returns true if it is actually a Property Object.
*/
bool DataModelDocObject::IsProperty() const
{
    return (dynamic_cast<const Property*>(this) != nullptr);
}


/*@NOTE_41451
Method that returns true if it is actually a PropertyInteger Object.
*/
bool DataModelDocObject::IsPropertyInteger() const
{
    return (dynamic_cast<const PropertyInteger*>(this) != nullptr);
}


/*@NOTE_41452
Method that returns true if it is actually a PropertyReal Object.
*/
bool DataModelDocObject::IsPropertyReal() const
{
    return (dynamic_cast<const PropertyReal*>(this) != nullptr);
}


/*@NOTE_41453
Method that returns true if it is actually a PropertyString Object.
*/
bool DataModelDocObject::IsPropertyString() const
{
    return (dynamic_cast<const PropertyString*>(this) != nullptr);
}


/*@NOTE_5385
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void DataModelDocObject::RemoveReferences()
{
    REMOVE_AVLTREE_OWNED_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_1554
Method which must be called first in a replace constructor
*/
void DataModelDocObject::ReplaceConstructorInclude(DataModelDocObject* pOld)
{
    REPLACE_AVLTREE_OWNED_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    REPLACE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_5386
Bring the current object relations into the same state as pDataModelDocObject.
*/
void DataModelDocObject::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_5388
Save the state of the current object relations to pDataModelDocObject.
*/
void DataModelDocObject::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_31
Serialize the members only to a CbObject object
*/
void DataModelDocObject::Serialize(CbArchive& archive)
{
    if (archive.IsStoring())
    {
        archive << _id;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _id;
        }
    }
}


/*@NOTE_30
Method which must be called first in a serialize constructor
*/
void DataModelDocObject::SerializeConstructorInclude()
{
    INIT_AVLTREE_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
}


/*@NOTE_33
Serialize the relations to a CbObject object
*/
void DataModelDocObject::SerializeRelations(CbArchive& archive,
                                            DataModelDocObject* pointerArray[])
{
    if (archive.IsStoring())
    {
        WRITE_AVLTREE_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_AVLTREE_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(DataModelDocObject)


// Methods for the relation(s) of the class
METHODS_AVLTREE_OWNED_ACTIVE(GetName(), DataModelDocObject, DataModelDocObject, Property, Property)
METHODS_ITERATOR_NOFILTER_MULTI_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
