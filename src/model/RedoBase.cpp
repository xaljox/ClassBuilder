/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoBase.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RedoBase'
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


/*@NOTE_5137
All different kind of re-doable mutations to the data structure are derived from this class.
*/
RedoBase::RedoBase(UndoBase* pUndoBase) //@INIT_5137
    : _pDataModelDocObject(pUndoBase->GetDataModelDocObject())
    , _last(pUndoBase->GetLast())
{//@CODE_5137
    ConstructorInclude(pUndoBase->GetDataModelDoc());
}//@CODE_5137


/*@NOTE_5128
Destructor method.
*/
RedoBase::~RedoBase()
{//@CODE_5128
    DestructorInclude();

    // Put in your own code
}//@CODE_5128


void RedoBase::AccumulateTouches(bool& tree, bool& cd, bool& sd)
{//@CODE_41380
    tree = tree || TouchesTree();
    cd   = cd   || TouchesCd();
    sd   = sd   || TouchesSd();
}//@CODE_41380


bool RedoBase::TouchesCd()
{//@CODE_41153
    DataModelDocObject* pObject = GetDataModelDocObject();
    if (!pObject || dynamic_cast<DataModelDoc*>(pObject))   // null or a parked DataModelDoc (stored by cast)
        return true;
    return pObject->TouchesCd();
}//@CODE_41153


bool RedoBase::TouchesSd()
{//@CODE_41154
    DataModelDocObject* pObject = GetDataModelDocObject();
    if (!pObject || dynamic_cast<DataModelDoc*>(pObject))   // null or a parked DataModelDoc (stored by cast)
        return true;
    return pObject->TouchesSd();
}//@CODE_41154


bool RedoBase::TouchesTree()
{//@CODE_41152
    DataModelDocObject* pObject = GetDataModelDocObject();
    if (!pObject || dynamic_cast<DataModelDoc*>(pObject))   // null or a parked DataModelDoc (stored by cast)
        return true;
    return pObject->TouchesTree();
}//@CODE_41152


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5127
Method which must be called first in a constructor.
*/
void RedoBase::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
}


/*@NOTE_5129
Method which must be called first in a destructor.
*/
void RedoBase::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
}


/*@NOTE_41457
Method that returns true if it is actually a RedoChangeDoc Object.
*/
bool RedoBase::IsRedoChangeDoc() const
{
    return (dynamic_cast<const RedoChangeDoc*>(this) != nullptr);
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
