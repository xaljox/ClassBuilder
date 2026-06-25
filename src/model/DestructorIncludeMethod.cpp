/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DestructorIncludeMethod.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DestructorIncludeMethod'
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


/*@NOTE_464
Constructor needed for serialization, not meant to use for other purposes!
*/
DestructorIncludeMethod::DestructorIncludeMethod() //@INIT_464
    : FixedMethod()
{//@CODE_464
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_464


DestructorIncludeMethod::DestructorIncludeMethod(Destructor* refDestructor) //@INIT_1003
    : FixedMethod(refDestructor->GetBaseClass(), refDestructor->GetDataModelDoc()->FindType("void"))
{//@CODE_1003
    ConstructorInclude(refDestructor);

    // Put in your own code
    SetAccess(PRIVATE);
    SetName("DestructorInclude");
    SetNote("Method which must be called first in a destructor." NL);
}//@CODE_1003


/*@NOTE_462
Destructor method
*/
DestructorIncludeMethod::~DestructorIncludeMethod()
{//@CODE_462
    DestructorInclude();

    // Put in your own code
}//@CODE_462


void DestructorIncludeMethod::InitCode()
{//@CODE_1005
    _code.Empty();

    Class::FromRelationIterator fromRelation((Class*)GetBaseClass());
    while (++fromRelation)
        fromRelation->WriteFromMacro(_code, GetIndent() + "EXIT_");

    Class::ToRelationIterator toRel((Class*)GetBaseClass());
    while (++toRel)
    {
        if (!(toRel->GetSingle() && 
              toRel->GetFromClass() == toRel->GetToClass() &&
              toRel->GetFromName() == toRel->GetToName()))
        {
            toRel->WriteToMacro(_code, GetIndent() + "EXIT_");
        }
    }
}//@CODE_1005


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5396
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void DestructorIncludeMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_461
Method which must be called first in a constructor
*/
void DestructorIncludeMethod::ConstructorInclude(Destructor* pDestructor)
{
    INIT_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_463
Method which must be called first in a destructor
*/
void DestructorIncludeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_5397
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void DestructorIncludeMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_5398
Bring the current object relations into the same state as pDataModelDocObject.
*/
void DestructorIncludeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    DestructorIncludeMethod* pDestructorIncludeMethod = (DestructorIncludeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5400
Save the state of the current object relations to pDataModelDocObject.
*/
void DestructorIncludeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
    DestructorIncludeMethod* pDestructorIncludeMethod = (DestructorIncludeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_466
Serialize the members only to a CbObject object
*/
void DestructorIncludeMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_465
Method which must be called first in a serialize constructor
*/
void DestructorIncludeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_468
Serialize the relations to a CbObject object
*/
void DestructorIncludeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(DestructorIncludeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
