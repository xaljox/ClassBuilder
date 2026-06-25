/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoNew.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RedoNew'
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


/*@NOTE_5173
Constructor needed if a new undo is performed and the corresponding redo has
to be popped on stack.
*/
RedoNew::RedoNew(UndoNew* pUndoNew) //@INIT_5173
    : RedoBase(pUndoNew)
{//@CODE_5173
    ConstructorInclude();

    // Notify object it is going to be removed
    _pDataModelDocObject->OnUndoRedoRemoving();
    
    // Make it a dead object by removing all references to it
    _pDataModelDocObject->RemoveReferences();

    // Notify object it is removed
    _pDataModelDocObject->OnUndoRedoRemoved();

    // View update after the removal (mirrors UndoDelete).
    GetDataModelDoc()->NotifyStructureChanged();
}//@CODE_5173


RedoNew::~RedoNew()
{//@CODE_5163
    DestructorInclude();

    if (_pDataModelDocObject)
    {
        // This isn't in use, so get rid of it, but make it destructable first.
        _pDataModelDocObject->CleanupReferences();
        delete _pDataModelDocObject;
    }
}//@CODE_5163


/*@NOTE_5175
Redo the recorded new.
*/
void RedoNew::Restore()
{//@CODE_5175
    // Restore the relations, with itself as example,
    // so it is placed back into its original context
    _pDataModelDocObject->RestoreReferences(_pDataModelDocObject);

    // Notify object it has added
    _pDataModelDocObject->OnUndoRedoAdded();

    // Make it undoable
    (void)new UndoNew(this);

    _pDataModelDocObject = NULL;
    delete this;
}//@CODE_5175


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5162
Method which must be called first in a constructor.
*/
void RedoNew::ConstructorInclude()
{
}


/*@NOTE_5164
Method which must be called first in a destructor.
*/
void RedoNew::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
