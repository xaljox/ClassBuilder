/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoSubDelete.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoSubDelete'
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


/*@NOTE_5222
Constructor for the making an undo object for the case an object is indirectly
deleted from the document. So the delete is a result from another delete.
*/
UndoSubDelete::UndoSubDelete(DataModelDocObject* pDataModelDocObject) //@INIT_5222
    : UndoBase(pDataModelDocObject)
{//@CODE_5222
    ConstructorInclude();

    // Notify object it is going to be removed
    _pDataModelDocObject->OnUndoRedoRemoving();

    // Make it a dead object by removing all references to it
    _pDataModelDocObject->RemoveReferences();
    
    GetDataModelDoc()->MoveUndoBaseLast(this);

    // Notify object it is removed
    _pDataModelDocObject->OnUndoRedoRemoved();

    // View update after the removal (mirrors UndoDelete).
    GetDataModelDoc()->NotifyStructureChanged();
}//@CODE_5222


UndoSubDelete::~UndoSubDelete()
{//@CODE_5212
    DestructorInclude();

    if (_pDataModelDocObject)
    {
        // This isn't in use, so get rid of it, but make it destructable first.
        _pDataModelDocObject->CleanupReferences();
        delete _pDataModelDocObject;
    }
}//@CODE_5212


/*@NOTE_5224
Undo the delete.
*/
void UndoSubDelete::Restore()
{//@CODE_5224
    // Restore the relations, with itself as example,
    // so it is placed back into its original context
    _pDataModelDocObject->RestoreReferences(_pDataModelDocObject);

    // Notify object it has added
    _pDataModelDocObject->OnUndoRedoAdded();

    _pDataModelDocObject = NULL;
    delete this;
}//@CODE_5224


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5211
Method which must be called first in a constructor.
*/
void UndoSubDelete::ConstructorInclude()
{
}


/*@NOTE_5213
Method which must be called first in a destructor.
*/
void UndoSubDelete::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
