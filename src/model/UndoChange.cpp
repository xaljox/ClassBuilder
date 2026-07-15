/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoChange.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoChange'
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
#include <sstream>
//@END_USER2


// Static members


/*@NOTE_5239
Constructor for the making an undo object for the case an object is changed in
the  document
*/
UndoChange::UndoChange(DataModelDocObject* pDataModelDocObject) //@INIT_5239
    : UndoBase(pDataModelDocObject)
    , _pDataModelDocObjectSave(NULL)
{//@CODE_5239
    ConstructorInclude();

    // Snapshot the object's state into a sibling instance via CObject on a
    // memory stream. Serialize bytes are identical to the file path; the
    // polymorphic operator instantiates the right subclass via CbClassRegistration.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive store(static_cast<std::ostream&>(ss));
        store << _pDataModelDocObject;
    }
    ss.seekg(0);
    {
        CbArchive load(static_cast<std::istream&>(ss));
        CbObject* tmp = NULL;
        load >> tmp;
        _pDataModelDocObjectSave = static_cast<DataModelDocObject*>(tmp);
    }

    // Save the state of the relations
    _pDataModelDocObject->SaveReferences(_pDataModelDocObjectSave);
}//@CODE_5239


/*@NOTE_5259
Undo constructor used if a redo is performed.
*/
UndoChange::UndoChange(RedoChange* pRedoChange) //@INIT_5259
    : UndoBase(pRedoChange)
    , _pDataModelDocObjectSave(NULL)
{//@CODE_5259
    ConstructorInclude();

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive store(static_cast<std::ostream&>(ss));
        store << _pDataModelDocObject;
    }
    ss.seekg(0);
    {
        CbArchive load(static_cast<std::istream&>(ss));
        CbObject* tmp = NULL;
        load >> tmp;
        _pDataModelDocObjectSave = static_cast<DataModelDocObject*>(tmp);
    }

    // Save the state of the relations
    _pDataModelDocObject->SaveReferences(_pDataModelDocObjectSave);
}//@CODE_5259


UndoChange::~UndoChange()
{//@CODE_5227
    DestructorInclude();

    // This isn't in use, so get rid of it, but make it destructable first.
    _pDataModelDocObjectSave->CleanupReferences();

    delete _pDataModelDocObjectSave;
}//@CODE_5227


/*@NOTE_5241
Undo the change.
*/
void UndoChange::Restore()
{//@CODE_5241
    // Save the current state first
    RedoChange* pRedoChange = new RedoChange(this);

    // Notify object it is going to change
    _pDataModelDocObject->OnUndoRedoChanging(_pDataModelDocObjectSave);

    // Restore member state via CObject on a memory stream: serialize the
    // saved snapshot out, then deserialize back into the live object.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive store(static_cast<std::ostream&>(ss));
        _pDataModelDocObjectSave->Serialize(store);
    }
    ss.seekg(0);
    {
        CbArchive load(static_cast<std::istream&>(ss));
        _pDataModelDocObject->Serialize(load);
    }

    // Restore the state of the relations
    _pDataModelDocObject->RestoreReferences(_pDataModelDocObjectSave);

    // Notify object it has changed
    _pDataModelDocObject->OnUndoRedoChanged(pRedoChange->GetDataModelDocObjectSave());

    delete this;
}//@CODE_5241


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5226
Method which must be called first in a constructor.
*/
void UndoChange::ConstructorInclude()
{
}


/*@NOTE_5228
Method which must be called first in a destructor.
*/
void UndoChange::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
