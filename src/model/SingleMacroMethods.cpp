/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SingleMacroMethods.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SingleMacroMethods'
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


/*@NOTE_1918
Constructor needed for serialization, not meant to use for other purposes!
*/
SingleMacroMethods::SingleMacroMethods() //@INIT_1918
    : FromRelationMacroMethods()
{//@CODE_1918
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1918


SingleMacroMethods::SingleMacroMethods(FromRelationMacroMethods* pOld) //@INIT_2020
    : FromRelationMacroMethods(pOld)
{//@CODE_2020
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
    pMethod->SetAccess(PUBLIC);
    pMethod->SetName("Add" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void RemoveToName(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetName("Remove" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void ReplaceToName(ToClass* pToClassOld, ToClass* pToClassNew);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
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
    pMethod->SetConst(1);
    pMethod->SetPointer(1);
    pMethod->SetName("Get" + toName);
}//@CODE_2020


/*@NOTE_1916
Destructor method
*/
SingleMacroMethods::~SingleMacroMethods()
{//@CODE_1916
    DestructorInclude();

    // Put in your own code
}//@CODE_1916


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5654
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SingleMacroMethods::CleanupReferences()
{
    FromRelationMacroMethods::CleanupReferences();
}


/*@NOTE_1915
Method which must be called first in a constructor
*/
void SingleMacroMethods::ConstructorInclude()
{
}


/*@NOTE_1917
Method which must be called first in a destructor
*/
void SingleMacroMethods::DestructorInclude()
{
}


/*@NOTE_5655
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SingleMacroMethods::RemoveReferences()
{
    FromRelationMacroMethods::RemoveReferences();
}


/*@NOTE_5656
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SingleMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5658
Save the state of the current object relations to pDataModelDocObject.
*/
void SingleMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1920
Serialize the members only to a CbObject object
*/
void SingleMacroMethods::Serialize(CbArchive& archive)
{
    FromRelationMacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1919
Method which must be called first in a serialize constructor
*/
void SingleMacroMethods::SerializeConstructorInclude()
{
}


/*@NOTE_1922
Serialize the relations to a CbObject object
*/
void SingleMacroMethods::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(SingleMacroMethods)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
