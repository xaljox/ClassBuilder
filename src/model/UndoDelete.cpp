/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoDelete.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoDelete'
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


/*@NOTE_5190
Constructor for the making an undo object for the case an object is deleted
from the  document
*/
UndoDelete::UndoDelete(DataModelDocObject* pDataModelDocObject) //@INIT_5190
    : UndoBase(pDataModelDocObject)
{//@CODE_5190
    ConstructorInclude();

    // Notify object it is going to be removed
    _pDataModelDocObject->OnUndoRedoRemoving();

    // Make it a dead object by removing all references to it
    _pDataModelDocObject->RemoveReferences();
    
    GetDataModelDoc()->MoveUndoBaseLast(this);

    // Notify object it is removed
    _pDataModelDocObject->OnUndoRedoRemoved();

    // View update AFTER the object left the data structure (the Qt trees
    // rebuild from the model on this call; the removed object can't report
    // it itself -- RemoveReferences cut its doc backpointer).
    GetDataModelDoc()->NotifyStructureChanged();
}//@CODE_5190


/*@NOTE_5208
Constructor needed if a new redo is performed and the corresponding undo has to
be popped on stack.
*/
UndoDelete::UndoDelete(RedoDelete* pRedoDelete) //@INIT_5208
    : UndoBase(pRedoDelete)
{//@CODE_5208
    ConstructorInclude();

    // Notify object it is going to be removed
    _pDataModelDocObject->OnUndoRedoRemoving();

    // Make it a dead object by removing all references to it
    _pDataModelDocObject->RemoveReferences();
    
    GetDataModelDoc()->MoveUndoBaseLast(this);

    // Notify object it is removed
    _pDataModelDocObject->OnUndoRedoRemoved();

    // View update after the removal (see the other ctor).
    GetDataModelDoc()->NotifyStructureChanged();
}//@CODE_5208


UndoDelete::~UndoDelete()
{//@CODE_5180
    DestructorInclude();

    if (_pDataModelDocObject)
    {
        // This isn't in use, so get rid of it, but make it destructable first.
        _pDataModelDocObject->CleanupReferences();
        delete _pDataModelDocObject;
    }
}//@CODE_5180


/*@NOTE_5192
Undo the delete.
*/
void UndoDelete::Restore()
{//@CODE_5192
    // Restore the relations, with itself as example,
    // so it is placed back into its original context
    _pDataModelDocObject->RestoreReferences(_pDataModelDocObject);

    // Notify object it has added
    _pDataModelDocObject->OnUndoRedoAdded();

    // Make it redoable
    (void)new RedoDelete(this);

    _pDataModelDocObject = NULL;
    delete this;
}//@CODE_5192


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5179
Method which must be called first in a constructor.
*/
void UndoDelete::ConstructorInclude()
{
}


/*@NOTE_5181
Method which must be called first in a destructor.
*/
void UndoDelete::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
