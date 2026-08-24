/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          CbViewLock.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'CbViewLock'
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


/*@NOTE_40970
Constructor method.
*/
CbViewLock::CbViewLock(DataModelDoc* pDataModelDoc,
                       bool busyCursor) //@INIT_40970
    : _pDataModelDoc(pDataModelDoc)
    , _busyCursor(busyCursor)
{//@CODE_40970
    ConstructorInclude();

    // Put in your own code
    // A null document is a deliberate no-op lock -- it lets a conditional
    // caller (e.g. the silent RollBack path, which wants no view update) use
    // the same guard rather than open-coding the lock primitives.
    //
    // busyCursor = false is the SINGLE-EDIT variant: same lock, same ONE
    // coalesced refresh at scope exit, just no cursor flip. ONE idiom for
    // every mutation -- the only choice a caller makes is whether the op is
    // long enough to want the wait cursor.
    if (_pDataModelDoc)
    {
        if (_busyCursor)
            Cb_BeginWaitCursor();
        _pDataModelDoc->LockAllViews();
    }
}//@CODE_40970


/*@NOTE_40909
Destructor method.
*/
CbViewLock::~CbViewLock()
{//@CODE_40909
    DestructorInclude();

    // Put in your own code
    if (_pDataModelDoc)
    {
        _pDataModelDoc->UnLockAndUpdateAllViews();
        if (_busyCursor)
            Cb_EndWaitCursor();
    }
}//@CODE_40909


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_40910
Method which must be called first in a constructor.
*/
void CbViewLock::ConstructorInclude()
{
}


/*@NOTE_40911
Method which must be called first in a destructor.
*/
void CbViewLock::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
