/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoChange.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RedoChange'
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


/*@NOTE_5256
Constructor needed if a change undo is performed and the corresponding redo has
to be popped on stack.
*/
RedoChange::RedoChange(UndoChange* pUndoChange) //@INIT_5256
    : RedoBase(pUndoChange)
    , _pDataModelDocObjectSave(NULL)
{//@CODE_5256
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
}//@CODE_5256


RedoChange::~RedoChange()
{//@CODE_5244
    DestructorInclude();

    // This isn't in use, so get rid of it, but make it destructable first.
    _pDataModelDocObjectSave->CleanupReferences();
    
    delete _pDataModelDocObjectSave;
}//@CODE_5244


/*@NOTE_5258
Redo the change.
*/
void RedoChange::Restore()
{//@CODE_5258
    // Save the current state first
    UndoChange* pUndoChange = new UndoChange(this);

    // Notify object it is going to change
    _pDataModelDocObject->OnUndoRedoChanging(_pDataModelDocObjectSave);

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
    _pDataModelDocObject->OnUndoRedoChanged(pUndoChange->GetDataModelDocObjectSave());

    delete this;
}//@CODE_5258


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5243
Method which must be called first in a constructor.
*/
void RedoChange::ConstructorInclude()
{
}


/*@NOTE_5245
Method which must be called first in a destructor.
*/
void RedoChange::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
