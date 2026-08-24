/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ToRelationMacroMethods.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ToRelationMacroMethods'
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


/*@NOTE_1879
Constructor needed for serialization, not meant to use for other purposes!
*/
ToRelationMacroMethods::ToRelationMacroMethods() //@INIT_1879
    : MacroMethods()
{//@CODE_1879
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1879


ToRelationMacroMethods::ToRelationMacroMethods(ToRelation* pToRelation) //@INIT_1990
    : MacroMethods(pToRelation->GetDataModelDoc())
{//@CODE_1990
    ConstructorInclude(pToRelation);

    // Put in your own code
    if (!pToRelation->GetRelation()->GetStatic())
    {
        Relation* pRelation = pToRelation->GetRelation();
        Class* pFromClass = pRelation->GetFromClass();
        Class* pToClass = pRelation->GetToClass();
        CbString fromName = pRelation->GetFromName();

        MacroMethod* pMethod = new MacroMethod(this, pToClass, pFromClass);
        pMethod->SetPointer(1);
        pMethod->SetAccess(PUBLIC);
        pMethod->SetConst(1);
        pMethod->SetName("Get" + fromName);
    }
}//@CODE_1990


/*@NOTE_1877
Destructor method
*/
ToRelationMacroMethods::~ToRelationMacroMethods()
{//@CODE_1877
    DestructorInclude();

    // Put in your own code
}//@CODE_1877


Gti* ToRelationMacroMethods::GetGtiParent()
{//@CODE_1992
    return GetToRelation();
}//@CODE_1992


Relation* ToRelationMacroMethods::GetRelation()
{//@CODE_1993
    return GetToRelation()->GetRelation();
}//@CODE_1993


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5636
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ToRelationMacroMethods::CleanupReferences()
{
    MacroMethods::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
}


/*@NOTE_1876
Method which must be called first in a constructor
*/
void ToRelationMacroMethods::ConstructorInclude(ToRelation* pToRelation)
{
    INIT_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
}


/*@NOTE_1878
Method which must be called first in a destructor
*/
void ToRelationMacroMethods::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
}


/*@NOTE_5637
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ToRelationMacroMethods::RemoveReferences()
{
    MacroMethods::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
}


/*@NOTE_5638
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ToRelationMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ToRelationMacroMethods* pToRelationMacroMethods = (ToRelationMacroMethods*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    MacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5640
Save the state of the current object relations to pDataModelDocObject.
*/
void ToRelationMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    MacroMethods::SaveReferences(pDataModelDocObject);
    ToRelationMacroMethods* pToRelationMacroMethods = (ToRelationMacroMethods*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
}


/*@NOTE_1881
Serialize the members only to a CbObject object
*/
void ToRelationMacroMethods::Serialize(CbArchive& archive)
{
    MacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1880
Method which must be called first in a serialize constructor
*/
void ToRelationMacroMethods::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
}


/*@NOTE_1883
Serialize the relations to a CbObject object
*/
void ToRelationMacroMethods::SerializeRelations(CbArchive& archive,
                                                DataModelDocObject* pointerArray[])
{
    MacroMethods::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ToRelationMacroMethods)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
