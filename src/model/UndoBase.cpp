/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoBase.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoBase'
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
//@END_USER2


// Static members


/*@NOTE_5123
Creates an undo object, with as argument the object involved in the undo.
*/
UndoBase::UndoBase(DataModelDocObject* pDataModelDocObject) //@INIT_5123
    : _pDataModelDocObject(pDataModelDocObject)
    , _last(0)
{//@CODE_5123
    ConstructorInclude(pDataModelDocObject->GetDataModelDoc());

    // If we add something new to the undo stack, then we have
    // to clean the redo stack
    GetDataModelDoc()->CleanRedo();
}//@CODE_5123


/*@NOTE_5144
Constructor needed if a redo is performed and the corresponding undo has to be
popped on stack.
*/
UndoBase::UndoBase(RedoBase* pRedoBase) //@INIT_5144
    : _pDataModelDocObject(pRedoBase->GetDataModelDocObject())
    , _last(pRedoBase->GetLast())
{//@CODE_5144
    ConstructorInclude(pRedoBase->GetDataModelDoc());
}//@CODE_5144


/*@NOTE_34868
Constructor method.
*/
UndoBase::UndoBase(DataModelDoc* pDataModelDoc) //@INIT_34868
    : _pDataModelDocObject((DataModelDocObject*)pDataModelDoc)
    , _last(0)
{//@CODE_34868
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
    GetDataModelDoc()->CleanRedo();
}//@CODE_34868


/*@NOTE_5108
Destructor method.
*/
UndoBase::~UndoBase()
{//@CODE_5108
    DestructorInclude();

    // Put in your own code
}//@CODE_5108


void UndoBase::AccumulateTouches(bool& tree, bool& cd, bool& sd)
{//@CODE_41376
    tree = tree || TouchesTree();
    cd   = cd   || TouchesCd();
    sd   = sd   || TouchesSd();
}//@CODE_41376


/*@NOTE_41170
A ConnectionSegment is being replaced in place by a styled cap (ConvertRouting):
pConnectionSegment is the cap, the current _pDataModelDocObject (pOld) is about
to be freed. The live relation graph is relinked elsewhere (ReplaceConstructor-
Include), but objects PARKED on the undo stack keep raw _prev/_next/_first/_last
pointers to pOld, severed from the live graph -- so nothing else redirects them,
and freeing pOld would leave them dangling. Walk the undo stack and let each
parked object redirect its own raw pointers from pOld to the cap via the virtual
ReplaceReference. A parked DataModelDoc (only ever on the stack via UndoChangeDoc,
stored into _pDataModelDocObject by a cast) is NOT a DataModelDocObject, so it is
detected by pointer identity and routed to DataModelDoc::ReplaceReference instead
of the segment virtual (which would read a bogus vtable). The redo stack is empty
here -- minting the cap's UndoNew already called CleanRedo -- but it is swept too,
a harmless no-op that keeps the replace safe if any future path ever reaches it
with redo entries present.
*/
void UndoBase::ChangeDataModelDocObject(ConnectionSegment* pConnectionSegment)
{//@CODE_41170
    ConnectionSegment* pOld = (ConnectionSegment*)_pDataModelDocObject;
    DataModelDoc* pDoc = GetDataModelDoc();
    DataModelDocObject* pDocAsObj = (DataModelDocObject*)pDoc;

    DataModelDoc::UndoBaseIterator iUndoBase(pDoc);
    while (++iUndoBase)
    {
        DataModelDocObject* p = iUndoBase->GetDataModelDocObject();
        if (p == pDocAsObj)
            pDoc->ReplaceReference(pOld, pConnectionSegment);
        else if (p)
            p->ReplaceReference(pOld, pConnectionSegment);
    }

    DataModelDoc::RedoBaseIterator iRedoBase(pDoc);
    while (++iRedoBase)
    {
        DataModelDocObject* p = iRedoBase->GetDataModelDocObject();
        if (p == pDocAsObj)
            pDoc->ReplaceReference(pOld, pConnectionSegment);
        else if (p)
            p->ReplaceReference(pOld, pConnectionSegment);
    }

    _pDataModelDocObject = pConnectionSegment;
}//@CODE_41170


bool UndoBase::TouchesCd()
{//@CODE_41150
    DataModelDocObject* pObject = GetDataModelDocObject();
    if (!pObject || dynamic_cast<DataModelDoc*>(pObject))   // null or a parked DataModelDoc (stored by cast)
        return true;
    return pObject->TouchesCd();
}//@CODE_41150


bool UndoBase::TouchesSd()
{//@CODE_41151
    DataModelDocObject* pObject = GetDataModelDocObject();
    if (!pObject || dynamic_cast<DataModelDoc*>(pObject))   // null or a parked DataModelDoc (stored by cast)
        return true;
    return pObject->TouchesSd();
}//@CODE_41151


bool UndoBase::TouchesTree()
{//@CODE_41149
    DataModelDocObject* pObject = GetDataModelDocObject();
    if (!pObject || dynamic_cast<DataModelDoc*>(pObject))   // null or a parked DataModelDoc (stored by cast)
        return true;
    return pObject->TouchesTree();
}//@CODE_41149


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5107
Method which must be called first in a constructor.
*/
void UndoBase::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
}


/*@NOTE_5109
Method which must be called first in a destructor.
*/
void UndoBase::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
}


/*@NOTE_41456
Method that returns true if it is actually a UndoChangeDoc Object.
*/
bool UndoBase::IsUndoChangeDoc() const
{
    return (dynamic_cast<const UndoChangeDoc*>(this) != nullptr);
}


/*@NOTE_41454
Method that returns true if it is actually a UndoDelete Object.
*/
bool UndoBase::IsUndoDelete() const
{
    return (dynamic_cast<const UndoDelete*>(this) != nullptr);
}


/*@NOTE_41455
Method that returns true if it is actually a UndoSubDelete Object.
*/
bool UndoBase::IsUndoSubDelete() const
{
    return (dynamic_cast<const UndoSubDelete*>(this) != nullptr);
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
