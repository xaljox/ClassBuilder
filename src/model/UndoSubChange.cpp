/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoSubChange.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoSubChange'
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


UndoSubChange::UndoSubChange(DataModelDocObject* pDataModelDocObject) //@INIT_5840
    : UndoBase(pDataModelDocObject)
    , _pDataModelDocObjectSave(NULL)
{//@CODE_5840
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
}//@CODE_5840


/*@NOTE_5830
Destructor method.
*/
UndoSubChange::~UndoSubChange()
{//@CODE_5830
    DestructorInclude();

    // This isn't in use, so get rid of it, but make it destructable first.
    _pDataModelDocObjectSave->CleanupReferences();

    delete _pDataModelDocObjectSave;
}//@CODE_5830


/*@NOTE_5842
Undo the change.
*/
void UndoSubChange::Restore()
{//@CODE_5842
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

    // Notify object it has changed, this restore is only called, if a
    // passive relations disappears
    _pDataModelDocObject->OnUndoRedoChanged(0);

    delete this;
}//@CODE_5842


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5829
Method which must be called first in a constructor.
*/
void UndoSubChange::ConstructorInclude()
{
}


/*@NOTE_5831
Method which must be called first in a destructor.
*/
void UndoSubChange::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
