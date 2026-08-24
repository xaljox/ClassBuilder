/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RedoDelete.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RedoDelete'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
//@END_USER2


// Static members


/*@NOTE_5205
Constructor needed if a delete undo is performed and the corresponding redo has
to be popped on stack.
*/
RedoDelete::RedoDelete(UndoDelete* pUndoDelete) //@INIT_5205
    : RedoBase(pUndoDelete)
{//@CODE_5205
    ConstructorInclude();

    // Put in your own code
}//@CODE_5205


/*@NOTE_5195
Destructor method.
*/
RedoDelete::~RedoDelete()
{//@CODE_5195
    DestructorInclude();

    // Put in your own code
}//@CODE_5195


/*@NOTE_5207
Redo the delete.
*/
void RedoDelete::Restore()
{//@CODE_5207
    // Make a undoable delete, the constructor take care of
    // all book keeping
    (void)new UndoDelete(this);

    delete this;
}//@CODE_5207


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5194
Method which must be called first in a constructor.
*/
void RedoDelete::ConstructorInclude()
{
}


/*@NOTE_5196
Method which must be called first in a destructor.
*/
void RedoDelete::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
