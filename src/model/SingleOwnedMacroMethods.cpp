/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SingleOwnedMacroMethods.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SingleOwnedMacroMethods'
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


/*@NOTE_1931
Constructor needed for serialization, not meant to use for other purposes!
*/
SingleOwnedMacroMethods::SingleOwnedMacroMethods() //@INIT_1931
    : FromRelationMacroMethods()
{//@CODE_1931
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1931


SingleOwnedMacroMethods::SingleOwnedMacroMethods(FromRelationMacroMethods* pOld) //@INIT_2019
    : FromRelationMacroMethods(pOld)
{//@CODE_2019
    ConstructorInclude();

    // Put in your own code
    Relation* pRelation = GetFromRelation()->GetRelation();
    Class* pFromClass = pRelation->GetFromClass();
    Class* pToClass = pRelation->GetToClass();
    CbString toName = pRelation->GetToName();
    Type* pVoidType = GetDataModelDoc()->FindType("void");

    MacroMethod* pMethod;
    Argument* pArgument;

    // void AddToName(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetName("Add" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void RemoveToName(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetName("Remove" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void ReplaceToName(ToClass* pToClassOld, ToClass* pToClassNew);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetName("Replace" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Old");
    pArgument->SetPointer(1);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "New");
    pArgument->SetPointer(1);

    // void MoveToName(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetName("Move" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // ToClass* GetToName();
    pMethod = new MacroMethod(this, pFromClass, pToClass);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetPointer(1);
    pMethod->SetConst(1);
    pMethod->SetName("Get" + toName);
}//@CODE_2019


/*@NOTE_1929
Destructor method
*/
SingleOwnedMacroMethods::~SingleOwnedMacroMethods()
{//@CODE_1929
    DestructorInclude();

    // Put in your own code
}//@CODE_1929


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5660
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SingleOwnedMacroMethods::CleanupReferences()
{
    FromRelationMacroMethods::CleanupReferences();
}


/*@NOTE_1928
Method which must be called first in a constructor
*/
void SingleOwnedMacroMethods::ConstructorInclude()
{
}


/*@NOTE_1930
Method which must be called first in a destructor
*/
void SingleOwnedMacroMethods::DestructorInclude()
{
}


/*@NOTE_5661
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SingleOwnedMacroMethods::RemoveReferences()
{
    FromRelationMacroMethods::RemoveReferences();
}


/*@NOTE_5662
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SingleOwnedMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5664
Save the state of the current object relations to pDataModelDocObject.
*/
void SingleOwnedMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1933
Serialize the members only to a CbObject object
*/
void SingleOwnedMacroMethods::Serialize(CbArchive& archive)
{
    FromRelationMacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1932
Method which must be called first in a serialize constructor
*/
void SingleOwnedMacroMethods::SerializeConstructorInclude()
{
}


/*@NOTE_1935
Serialize the relations to a CbObject object
*/
void SingleOwnedMacroMethods::SerializeRelations(CbArchive& archive,
                                                 DataModelDocObject* pointerArray[])
{
    FromRelationMacroMethods::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(SingleOwnedMacroMethods)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
