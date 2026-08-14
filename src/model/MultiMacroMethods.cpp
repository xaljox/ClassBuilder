/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MultiMacroMethods.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MultiMacroMethods'
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


/*@NOTE_1892
Constructor needed for serialization, not meant to use for other purposes!
*/
MultiMacroMethods::MultiMacroMethods() //@INIT_1892
    : FromRelationMacroMethods()
{//@CODE_1892
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1892


MultiMacroMethods::MultiMacroMethods(FromRelationMacroMethods* pOld) //@INIT_2016
    : FromRelationMacroMethods(pOld)
{//@CODE_2016
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

        // void RemoveAllToName();
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("RemoveAll" + toName);

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
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Add" + toName + "First");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void AddToNameLast(ToClass* pToClass);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Add" + toName + "Last");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void AddToNameAfter(ToClass* pToClass, ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Add" + toName + "After");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // void AddToNameBefore(ToClass* pToClass, ToClass* pToClassPos);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Add" + toName + "Before");
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName() + "Pos");
        pArgument->SetPointer(1);

        // void RemoveToName(ToClass* pToClass);
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("Remove" + toName);
        pArgument = new Argument(pMethod, pToClass);
        pArgument->SetName("p" + pToClass->GetBaseName());
        pArgument->SetPointer(1);

        // void RemoveAllToName();
        pMethod = new MacroMethod(this, pFromClass, pVoidType);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetName("RemoveAll" + toName);

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
}//@CODE_2016


/*@NOTE_1890
Destructor method
*/
MultiMacroMethods::~MultiMacroMethods()
{//@CODE_1890
    DestructorInclude();

    // Put in your own code
}//@CODE_1890


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5642
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MultiMacroMethods::CleanupReferences()
{
    FromRelationMacroMethods::CleanupReferences();
}


/*@NOTE_1889
Method which must be called first in a constructor
*/
void MultiMacroMethods::ConstructorInclude()
{
}


/*@NOTE_1891
Method which must be called first in a destructor
*/
void MultiMacroMethods::DestructorInclude()
{
}


/*@NOTE_5643
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MultiMacroMethods::RemoveReferences()
{
    FromRelationMacroMethods::RemoveReferences();
}


/*@NOTE_5644
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MultiMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5646
Save the state of the current object relations to pDataModelDocObject.
*/
void MultiMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1894
Serialize the members only to a CbObject object
*/
void MultiMacroMethods::Serialize(CbArchive& archive)
{
    FromRelationMacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1893
Method which must be called first in a serialize constructor
*/
void MultiMacroMethods::SerializeConstructorInclude()
{
}


/*@NOTE_1896
Serialize the relations to a CbObject object
*/
void MultiMacroMethods::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(MultiMacroMethods)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
