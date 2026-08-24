/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ReplaceConstructorIncludeMethod.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ReplaceConstructorIncludeMethod'
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


/*@NOTE_451
Constructor needed for serialization, not meant to use for other purposes!
*/
ReplaceConstructorIncludeMethod::ReplaceConstructorIncludeMethod() //@INIT_451
    : FixedMethod()
{//@CODE_451
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_451


ReplaceConstructorIncludeMethod::ReplaceConstructorIncludeMethod(ReplaceConstructor* pReplaceConstructor) //@INIT_1000
    : FixedMethod(pReplaceConstructor->GetBaseClass(), 
                  pReplaceConstructor->GetDataModelDoc()->FindType("void"))
{//@CODE_1000
    ConstructorInclude(pReplaceConstructor);

    // Put in your own code
    SetName("ReplaceConstructorInclude");
    SetNote("Method which must be called first in a replace constructor.");

    Argument* argument = new Argument(this, GetBaseClass());
    argument->SetPointer(1);
    argument->SetName("pOld");

    BaseClass::MethodIterator method(GetBaseClass(), &Method::IsFixedMethod);
    while (++method)
    {
        if (method->GetName() == "ConstructorInclude")
        {
            if (method->GetMemberAndMethodGroup())
            {
                method->GetMemberAndMethodGroup()->AddMethodLast(this);
                break;
            }
        }
    }

    if (GetBaseClass()->GetAdded())
        Add();
}//@CODE_1000


/*@NOTE_449
Destructor method
*/
ReplaceConstructorIncludeMethod::~ReplaceConstructorIncludeMethod()
{//@CODE_449
    DestructorInclude();

    // Put in your own code
}//@CODE_449


void ReplaceConstructorIncludeMethod::InitCode()
{//@CODE_1002
    _code.Empty();

    Class::FromRelationIterator fromRelation((Class*)GetBaseClass());
    while (++fromRelation)
        fromRelation->WriteFromMacro(_code, GetIndent() + "REPLACE_");

    Class::ToRelationIterator toRel((Class*)GetBaseClass());
    while (++toRel)
    {
        if (!(toRel->GetSingle() && 
              toRel->GetFromClass() == toRel->GetToClass() &&
              toRel->GetFromName() == toRel->GetToName()))
        {
            toRel->WriteToMacro(_code, GetIndent() + "REPLACE_");
        }
    }
}//@CODE_1002


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5522
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ReplaceConstructorIncludeMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_448
Method which must be called first in a constructor
*/
void ReplaceConstructorIncludeMethod::ConstructorInclude(ReplaceConstructor* pReplaceConstructor)
{
    INIT_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_450
Method which must be called first in a destructor
*/
void ReplaceConstructorIncludeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_5523
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ReplaceConstructorIncludeMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_5524
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ReplaceConstructorIncludeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ReplaceConstructorIncludeMethod* pReplaceConstructorIncludeMethod = (ReplaceConstructorIncludeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5526
Save the state of the current object relations to pDataModelDocObject.
*/
void ReplaceConstructorIncludeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
    ReplaceConstructorIncludeMethod* pReplaceConstructorIncludeMethod = (ReplaceConstructorIncludeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_453
Serialize the members only to a CbObject object
*/
void ReplaceConstructorIncludeMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_452
Method which must be called first in a serialize constructor
*/
void ReplaceConstructorIncludeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_455
Serialize the relations to a CbObject object
*/
void ReplaceConstructorIncludeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(ReplaceConstructorIncludeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
