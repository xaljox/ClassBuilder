/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FromRelationMethod.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FromRelationMethod'
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


/*@NOTE_282
Constructor needed for serialization, not meant to use for other purposes!
*/
FromRelationMethod::FromRelationMethod() //@INIT_282
    : Method()
{//@CODE_282
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_282


FromRelationMethod::FromRelationMethod(FromRelation* pFromRelation,
                                       Type* pType) //@INIT_910
    : Method(pFromRelation->GetRelation()->GetFromClass(), pType)
{//@CODE_910
    ConstructorInclude(pFromRelation);

    // Put in your own code
    SetStatic(pFromRelation->GetRelation()->GetStatic());
	_untouched = 0;
}//@CODE_910


/*@NOTE_280
Destructor method
*/
FromRelationMethod::~FromRelationMethod()
{//@CODE_280
    DestructorInclude();

    // Put in your own code
}//@CODE_280


void FromRelationMethod::Add()
{//@CODE_913
    if (!GetAdded())
    {
        SaveState(1);
        GetFromRelation()->AddChildLast(this);

        Method::Add();
    }
}//@CODE_913


bool FromRelationMethod::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1374
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = Method::Drag(ctrlKeyDown, pGtiDropDefault);
    }
    else
    {
    }

    return value;
}//@CODE_1374


bool FromRelationMethod::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1377
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        Member* pDropMember = dynamic_cast<Member*>(pGtiDrop);

        if (pDropMember && pDropMember->GetType() == GetBaseClass())
            value = true;
    }
    else
    {
        value = Method::DropTarget(ctrlKeyDown, pGtiDrop);
    }

    return value;
}//@CODE_1377


int FromRelationMethod::OnAddArgument(bool checkOnly)
{//@CODE_35785
    return 0;
}//@CODE_35785


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5432
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FromRelationMethod::CleanupReferences()
{
    Method::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
}


/*@NOTE_279
Method which must be called first in a constructor
*/
void FromRelationMethod::ConstructorInclude(FromRelation* pFromRelation)
{
    INIT_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
}


/*@NOTE_281
Method which must be called first in a destructor
*/
void FromRelationMethod::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
}


/*@NOTE_5433
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FromRelationMethod::RemoveReferences()
{
    Method::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
}


/*@NOTE_5434
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FromRelationMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMethod* pFromRelationMethod = (FromRelationMethod*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
    Method::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5436
Save the state of the current object relations to pDataModelDocObject.
*/
void FromRelationMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::SaveReferences(pDataModelDocObject);
    FromRelationMethod* pFromRelationMethod = (FromRelationMethod*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
}


/*@NOTE_284
Serialize the members only to a CbObject object
*/
void FromRelationMethod::Serialize(CbArchive& archive)
{
    Method::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_283
Method which must be called first in a serialize constructor
*/
void FromRelationMethod::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)
}


/*@NOTE_286
Serialize the relations to a CbObject object
*/
void FromRelationMethod::SerializeRelations(CbArchive& archive,
                                            DataModelDocObject* pointerArray[])
{
    Method::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(FromRelationMethod)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
