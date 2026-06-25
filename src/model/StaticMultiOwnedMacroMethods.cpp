/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          StaticMultiOwnedMacroMethods.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'StaticMultiOwnedMacroMethods'
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


/*@NOTE_1957
Constructor needed for serialization, not meant to use for other purposes!
*/
StaticMultiOwnedMacroMethods::StaticMultiOwnedMacroMethods() //@INIT_1957
    : FromRelationMacroMethods()
{//@CODE_1957
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1957


StaticMultiOwnedMacroMethods::StaticMultiOwnedMacroMethods(FromRelationMacroMethods* pOld) //@INIT_2017
    : FromRelationMacroMethods(pOld)
{//@CODE_2017
    ConstructorInclude();

    // Put in your own code
    Relation* pRelation = GetFromRelation()->GetRelation();
    Class* pFromClass = pRelation->GetFromClass();
    Class* pToClass = pRelation->GetToClass();
    CbString toName = pRelation->GetToName();
    Type* pVoidType = GetDataModelDoc()->FindType("void");
    Type* pIntType = GetDataModelDoc()->FindType("int");

    MacroMethod* pMethod;
    Argument* pArgument;

    // void AddToNameFirst(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetStatic(1);
    pMethod->SetName("Add" + toName + "First");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void AddToNameLast(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetStatic(1);
    pMethod->SetName("Add" + toName + "Last");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void AddToNameAfter(ToClass* pToClass, ToClass* pToClassPos);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetStatic(1);
    pMethod->SetName("Add" + toName + "After");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
    pArgument->SetPointer(1);

    // void AddToNameBefore(ToClass* pToClass, ToClass* pToClassPos);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetStatic(1);
    pMethod->SetName("Add" + toName + "Before");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
    pArgument->SetPointer(1);

    // void RemoveToName(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetStatic(1);
    pMethod->SetName("Remove" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void DeleteAllToName();
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetStatic(1);
    pMethod->SetName("DeleteAll" + toName);

    // void ReplaceToName(ToClass* pToClassOld, ToClass* pToClassNew);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PROTECTED);
    pMethod->SetStatic(1);
    pMethod->SetName("Replace" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Old");
    pArgument->SetPointer(1);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "New");
    pArgument->SetPointer(1);

    // ToClass* GetFirstToName();
    pMethod = new MacroMethod(this, pFromClass, pToClass);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetPointer(1);
    pMethod->SetStatic(1);
    pMethod->SetConst(1);
    pMethod->SetName("GetFirst" + toName);

    // ToClass* GetLastToName();
    pMethod = new MacroMethod(this, pFromClass, pToClass);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetPointer(1);
    pMethod->SetStatic(1);
    pMethod->SetConst(1);
    pMethod->SetName("GetLast" + toName);

    // ToClass* GetNextToName(ToClass* pToClassPos);
    pMethod = new MacroMethod(this, pFromClass, pToClass);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetPointer(1);
    pMethod->SetStatic(1);
    pMethod->SetConst(1);
    pMethod->SetName("GetNext" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
    pArgument->SetPointer(1);

    // ToClass* GetPrevToName(ToClass* pToClassPos);
    pMethod = new MacroMethod(this, pFromClass, pToClass);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetPointer(1);
    pMethod->SetStatic(1);
    pMethod->SetConst(1);
    pMethod->SetName("GetPrev" + toName);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
    pArgument->SetPointer(1);

    // int GetToNameCount();
    pMethod = new MacroMethod(this, pFromClass, pIntType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetStatic(1);
    pMethod->SetConst(1);
    pMethod->SetName("Get" + toName + "Count");

    // void MoveToNameFirst(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetStatic(1);
    pMethod->SetName("Move" + toName + "First");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void MoveToNameLast(ToClass* pToClass);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetStatic(1);
    pMethod->SetName("Move" + toName + "Last");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);

    // void MoveToNameAfter(ToClass* pToClass, ToClass* pToClassPos);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetStatic(1);
    pMethod->SetName("Move" + toName + "After");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
    pArgument->SetPointer(1);

    // void MoveToNameBefore(ToClass* pToClass, ToClass* pToClassPos);
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetStatic(1);
    pMethod->SetName("Move" + toName + "Before");
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName());
    pArgument->SetPointer(1);
    pArgument = new Argument(pMethod, pToClass);
    pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
    pArgument->SetPointer(1);

    // static void SortToName(int (*Compare)(ToClass*, ToClass*));
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetStatic(1);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetName("Sort" + toName);
    pArgument = new Argument(pMethod, pIntType);
    pArgument->SetName("(*Compare)(" + pToClass->GetName() + "*, " + pToClass->GetName() + "*)");

    // static void MergeSortToName(int (*Compare)(ToClass*, ToClass*));
    pMethod = new MacroMethod(this, pFromClass, pVoidType);
    pMethod->SetStatic(1);
    pMethod->SetAccess(PUBLIC);
    pMethod->SetName("MergeSort" + toName);
    pArgument = new Argument(pMethod, pIntType);
    pArgument->SetName("(*Compare)(" + pToClass->GetName() + "*, " + pToClass->GetName() + "*)");
}//@CODE_2017


/*@NOTE_1955
Destructor method
*/
StaticMultiOwnedMacroMethods::~StaticMultiOwnedMacroMethods()
{//@CODE_1955
    DestructorInclude();

    // Put in your own code
}//@CODE_1955


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5672
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void StaticMultiOwnedMacroMethods::CleanupReferences()
{
    FromRelationMacroMethods::CleanupReferences();
}


/*@NOTE_1954
Method which must be called first in a constructor
*/
void StaticMultiOwnedMacroMethods::ConstructorInclude()
{
}


/*@NOTE_1956
Method which must be called first in a destructor
*/
void StaticMultiOwnedMacroMethods::DestructorInclude()
{
}


/*@NOTE_5673
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void StaticMultiOwnedMacroMethods::RemoveReferences()
{
    FromRelationMacroMethods::RemoveReferences();
}


/*@NOTE_5674
Bring the current object relations into the same state as pDataModelDocObject.
*/
void StaticMultiOwnedMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5676
Save the state of the current object relations to pDataModelDocObject.
*/
void StaticMultiOwnedMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1959
Serialize the members only to a CbObject object
*/
void StaticMultiOwnedMacroMethods::Serialize(CbArchive& archive)
{
    FromRelationMacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1958
Method which must be called first in a serialize constructor
*/
void StaticMultiOwnedMacroMethods::SerializeConstructorInclude()
{
}


/*@NOTE_1961
Serialize the relations to a CbObject object
*/
void StaticMultiOwnedMacroMethods::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(StaticMultiOwnedMacroMethods)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
