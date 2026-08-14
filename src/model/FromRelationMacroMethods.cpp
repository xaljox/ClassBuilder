/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FromRelationMacroMethods.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FromRelationMacroMethods'
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


/*@NOTE_1866
Constructor needed for serialization, not meant to use for other purposes!
*/
FromRelationMacroMethods::FromRelationMacroMethods() //@INIT_1866
    : MacroMethods()
{//@CODE_1866
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1866


/*@NOTE_2007
Constructor needed for putting a new object in the old one's context
*/
FromRelationMacroMethods::FromRelationMacroMethods(FromRelationMacroMethods* pOld) //@INIT_2007
    : MacroMethods(pOld)
{//@CODE_2007
    ReplaceConstructorInclude(pOld);

    // Put in your own code
}//@CODE_2007


FromRelationMacroMethods::FromRelationMacroMethods(FromRelation* pFromRelation) //@INIT_2022
    : MacroMethods(pFromRelation->GetDataModelDoc())
{//@CODE_2022
    ConstructorInclude(pFromRelation);

    // Replace the current 'this' object with an object of the correct type
    Relation* pRelation = pFromRelation->GetRelation();
    if (pRelation->GetStatic())
    {
        if (pRelation->GetOwned())
            (void) new StaticMultiOwnedMacroMethods(this);
        else
            (void) new StaticMultiMacroMethods(this);
    }
    else if (pRelation->GetMulti())
    {
        if (pRelation->GetOwned())
            (void) new MultiOwnedMacroMethods(this);
        else
            (void) new MultiMacroMethods(this);
    }
    else
    {
        if (pRelation->GetOwned())
            (void) new SingleOwnedMacroMethods(this);
        else
            (void) new SingleMacroMethods(this);
    }

    // The current 'this' object is replaced, so delete it, 
	// but allow optimisation of Undo Stack.
    Delete(pFromRelation->GetDataModelDoc());
}//@CODE_2022


/*@NOTE_1864
Destructor method
*/
FromRelationMacroMethods::~FromRelationMacroMethods()
{//@CODE_1864
    DestructorInclude();

    // Put in your own code
}//@CODE_1864


Gti* FromRelationMacroMethods::GetGtiParent()
{//@CODE_1981
    return GetFromRelation();
}//@CODE_1981


Relation* FromRelationMacroMethods::GetRelation()
{//@CODE_1982
    return GetFromRelation()->GetRelation();
}//@CODE_1982


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5630
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FromRelationMacroMethods::CleanupReferences()
{
    MacroMethods::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_1863
Method which must be called first in a constructor
*/
void FromRelationMacroMethods::ConstructorInclude(FromRelation* pFromRelation)
{
    INIT_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_1865
Method which must be called first in a destructor
*/
void FromRelationMacroMethods::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_5631
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FromRelationMacroMethods::RemoveReferences()
{
    MacroMethods::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_2009
Method which must be called first in a replace constructor
*/
void FromRelationMacroMethods::ReplaceConstructorInclude(FromRelationMacroMethods* pOld)
{
    REPLACE_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_5632
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FromRelationMacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMacroMethods* pFromRelationMacroMethods = (FromRelationMacroMethods*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    MacroMethods::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5634
Save the state of the current object relations to pDataModelDocObject.
*/
void FromRelationMacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    MacroMethods::SaveReferences(pDataModelDocObject);
    FromRelationMacroMethods* pFromRelationMacroMethods = (FromRelationMacroMethods*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_1868
Serialize the members only to a CbObject object
*/
void FromRelationMacroMethods::Serialize(CbArchive& archive)
{
    MacroMethods::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1867
Method which must be called first in a serialize constructor
*/
void FromRelationMacroMethods::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
}


/*@NOTE_1870
Serialize the relations to a CbObject object
*/
void FromRelationMacroMethods::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FromRelationMacroMethods)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
