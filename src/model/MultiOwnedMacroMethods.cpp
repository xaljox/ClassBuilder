/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MultiOwnedMacroMethods.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MultiOwnedMacroMethods'
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


/*@NOTE_1905
Constructor needed for serialization, not meant to use for other purposes!
*/
MultiOwnedMacroMethods::MultiOwnedMacroMethods() //@INIT_1905
    : FromRelationMacroMethods()
{//@CODE_1905
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1905


MultiOwnedMacroMethods::MultiOwnedMacroMethods(FromRelationMacroMethods* pOld) //@INIT_2021
    : FromRelationMacroMethods(pOld)
{//@CODE_2021
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

    if (pRelation->GetRelationMember())
    {
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

        // void DeleteAllToName();
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("DeleteAll" + toName);

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

        // ToClass* GetFirstToName();
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetFirst" + toName);

        // ToClass* GetLastToName();
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetLast" + toName);

        // ToClass* GetNextToName(ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetNext" + toName);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // ToClass* GetPrevToName(ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetPrev" + toName);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // int GetToNameCount();
        pMethod = new MacroMethod(this, pFromClass, pIntType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetConst(1);
        pMethod->SetName("Get" + toName + "Count");
    }
    else
    {
        // void AddToNameFirst(ToClass* pToClass);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PROTECTED);
        pMethod->SetName("Add" + toName + "First");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void AddToNameLast(ToClass* pToClass);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PROTECTED);
        pMethod->SetName("Add" + toName + "Last");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void AddToNameAfter(ToClass* pToClass, ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PROTECTED);
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
        pMethod->SetName("Remove" + toName);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void DeleteAllToName();
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("DeleteAll" + toName);

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

        // ToClass* GetFirstToName();
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetFirst" + toName);

        // ToClass* GetLastToName();
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetLast" + toName);

        // ToClass* GetNextToName(ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetNext" + toName);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // ToClass* GetPrevToName(ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pToClass);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetPointer(1);
        pMethod->SetConst(1);
        pMethod->SetName("GetPrev" + toName);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // int GetToNameCount();
        pMethod = new MacroMethod(this, pFromClass, pIntType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetConst(1);
        pMethod->SetName("Get" + toName + "Count");

        // void MoveToNameFirst(ToClass* pToClass);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Move" + toName + "First");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void MoveToNameLast(ToClass* pToClass);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Move" + toName + "Last");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void MoveToNameAfter(ToClass* pToClass, ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
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
        pMethod->SetName("Move" + toName + "Before");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // void SortToName(int (*Compare)(ToClass*, ToClass*));
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Sort" + toName);
        pArgument = new Argument(pMethod, pIntType);
        pArgument->SetName("(*Compare)(" + pToClass->GetName() + "*, " + pToClass->GetName() + "*)");

        // void MergeSortToName(int (*Compare)(ToClass*, ToClass*));
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("MergeSort" + toName);
        pArgument = new Argument(pMethod, pIntType);
        pArgument->SetName("(*Compare)(" + pToClass->GetName() + "*, " + pToClass->GetName() + "*)");
    }
}//@CODE_2021


/*@NOTE_1903
Destructor method
*/
MultiOwnedMacroMethods::~MultiOwnedMacroMethods()
{//@CODE_1903
    DestructorInclude();

    // Put in your own code
}//@CODE_1903


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5648
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MultiOwnedMacroMethods::CleanupReferences()
{
    FromRelationMacroMethods::CleanupReferences();
}


/*@NOTE_1902
Method which must be called first in a constructor
*/
void MultiOwnedMacroMethods::ConstructorInclude()
{
}


/*@NOTE_1904
Method which must be called first in a destructor
*/
void MultiOwnedMacroMethods::DestructorInclude()
{
}


/*@NOTE_5649
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MultiOwnedMacroMethods::RemoveReferences()
{
    FromRelationMacroMethods::RemoveReferences();
}


/*@NOTE_5650
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MultiOwnedMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5652
Save the state of the current object relations to pDataModelDocObject.
*/
void MultiOwnedMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1907
Serialize the members only to a CbObject object
*/
void MultiOwnedMacroMethods::Serialize(CbArchive& archive)
{
    FromRelationMacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1906
Method which must be called first in a serialize constructor
*/
void MultiOwnedMacroMethods::SerializeConstructorInclude()
{
}


/*@NOTE_1909
Serialize the relations to a CbObject object
*/
void MultiOwnedMacroMethods::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(MultiOwnedMacroMethods)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
