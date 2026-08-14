/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          IsClassMethod.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'IsClassMethod'
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


/*@NOTE_503
Constructor needed for serialization, not meant to use for other purposes!
*/
IsClassMethod::IsClassMethod() //@INIT_503
    : FixedMethod()
{//@CODE_503
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_503


IsClassMethod::IsClassMethod(Class* pClass, Class* pRefClass) //@INIT_1012
    : FixedMethod(pClass, pClass->GetDataModelDoc()->FindType("bool"))
{//@CODE_1012
    ConstructorInclude(pRefClass);

    // Put in your own code
    SetName("Is" + pRefClass->GetBaseName());
    SetNote("Method that returns true if it is actually a " +
            pRefClass->GetName()+ " Object." NL);
    SetAccess(PUBLIC);
	SetConst(1);
}//@CODE_1012


/*@NOTE_501
Destructor method
*/
IsClassMethod::~IsClassMethod()
{//@CODE_501
    DestructorInclude();

    // Put in your own code
}//@CODE_501


void IsClassMethod::InitCode()
{//@CODE_1015
    _code.Empty();

    _code += GetIndent() + "return (dynamic_cast<";
	if (GetConst())
	{
		_code += "const ";
	}
	_code += GetRefClass()->GetName() + "*>(this) != nullptr);" NL;
}//@CODE_1015


int IsClassMethod::OnDelete(bool checkOnly)
{//@CODE_1016
    return Method::OnDelete(checkOnly); 
}//@CODE_1016


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5462
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void IsClassMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
}


/*@NOTE_500
Method which must be called first in a constructor
*/
void IsClassMethod::ConstructorInclude(Class* pRefClass)
{
    INIT_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
}


/*@NOTE_502
Method which must be called first in a destructor
*/
void IsClassMethod::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
}


/*@NOTE_5463
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void IsClassMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
}


/*@NOTE_5464
Bring the current object relations into the same state as pDataModelDocObject.
*/
void IsClassMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    IsClassMethod* pIsClassMethod = (IsClassMethod*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5466
Save the state of the current object relations to pDataModelDocObject.
*/
void IsClassMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
    IsClassMethod* pIsClassMethod = (IsClassMethod*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
}


/*@NOTE_505
Serialize the members only to a CbObject object
*/
void IsClassMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_504
Method which must be called first in a serialize constructor
*/
void IsClassMethod::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)
}


/*@NOTE_507
Serialize the relations to a CbObject object
*/
void IsClassMethod::SerializeRelations(CbArchive& archive,
                                       DataModelDocObject* pointerArray[])
{
    FixedMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(IsClassMethod)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
