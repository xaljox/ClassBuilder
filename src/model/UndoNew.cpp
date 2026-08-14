/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          UndoNew.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoNew'
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


/*@NOTE_5158
Constructor for the making an undo object for the case a new object is added to
the  document.
*/
UndoNew::UndoNew(DataModelDocObject* pDataModelDocObject) //@INIT_5158
    : UndoBase(pDataModelDocObject)
{//@CODE_5158
    ConstructorInclude();

    // Put in your own code

    // Object creation dirties the document -- the Add/Delete half of the
    // two-place dirty rule at creation's single chokepoint (diagram shapes
    // have no Gti::Add). Guarded like SaveState: undo/redo replays recreate
    // objects through here and must not re-dirty.
    DataModelDoc* pDataModelDoc = pDataModelDocObject->GetDataModelDoc();
    if (pDataModelDoc && !pDataModelDoc->GetIsUndoing() &&
        !pDataModelDoc->GetIsRedoing())
    {
        pDataModelDoc->SetModifiedFlag();

        // Derived refresh: creation is the chokepoint that records the view
        // notification (diagram shapes have no Gti::Add) -- kick each view type
        // the new object touches.
        if (pDataModelDocObject->TouchesTree())
            pDataModelDoc->NotifyTreeViews();
        if (pDataModelDocObject->TouchesCd())
            pDataModelDoc->NotifyCdViews();
        if (pDataModelDocObject->TouchesSd())
            pDataModelDoc->NotifySdViews();
    }
}//@CODE_5158


/*@NOTE_5176
Constructor needed if a delete redo is performed and the corresponding undo has
to be popped on stack.
*/
UndoNew::UndoNew(RedoNew* pRedoNew) //@INIT_5176
    : UndoBase(pRedoNew)
{//@CODE_5176
    ConstructorInclude();

    // Put in your own code
}//@CODE_5176


/*@NOTE_5148
Destructor method.
*/
UndoNew::~UndoNew()
{//@CODE_5148
    DestructorInclude();

    // Put in your own code
}//@CODE_5148


/*@NOTE_5160
Undo the recorded new.
*/
void UndoNew::Restore()
{//@CODE_5160
    // Make a redoable new, the constructor take care of
    // all book keeping
    (void)new RedoNew(this);

    delete this;
}//@CODE_5160


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5147
Method which must be called first in a constructor.
*/
void UndoNew::ConstructorInclude()
{
}


/*@NOTE_5149
Method which must be called first in a destructor.
*/
void UndoNew::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
