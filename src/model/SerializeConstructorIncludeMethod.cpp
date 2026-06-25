/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SerializeConstructorIncludeMethod.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SerializeConstructorIncludeMethod'
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


/*@NOTE_438
Constructor needed for serialization, not meant to use for other purposes!
*/
SerializeConstructorIncludeMethod::SerializeConstructorIncludeMethod() //@INIT_438
    : FixedMethod()
{//@CODE_438
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_438


SerializeConstructorIncludeMethod::SerializeConstructorIncludeMethod(SerializeConstructor* pSerializeConstructor) //@INIT_997
    : FixedMethod(pSerializeConstructor->GetBaseClass(), 
                  pSerializeConstructor->GetDataModelDoc()->FindType("void"))
{//@CODE_997
    ConstructorInclude(pSerializeConstructor);

    // Put in your own code
    SetName("SerializeConstructorInclude");
    SetNote("Method which must be called first in a serialize constructor.");
}//@CODE_997


/*@NOTE_436
Destructor method
*/
SerializeConstructorIncludeMethod::~SerializeConstructorIncludeMethod()
{//@CODE_436
    DestructorInclude();

    // Put in your own code
}//@CODE_436


void SerializeConstructorIncludeMethod::InitCode()
{//@CODE_999
    _code.Empty();

    Class::FromRelationIterator fromRelation((Class*)GetBaseClass());
    while (++fromRelation)
        fromRelation->WriteFromMacro(_code, GetIndent() + "INIT_", 0);

    Class::ToRelationIterator toRel((Class*)GetBaseClass());
    while (++toRel)
    {
        if (!(toRel->GetSingle() && 
              toRel->GetFromClass() == toRel->GetToClass() &&
              toRel->GetFromName() == toRel->GetToName()))
        {
            toRel->WriteToMacro(_code, GetIndent() + "INIT_", 0);
        }
    }
}//@CODE_999


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5534
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SerializeConstructorIncludeMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_435
Method which must be called first in a constructor
*/
void SerializeConstructorIncludeMethod::ConstructorInclude(SerializeConstructor* pSerializeConstructor)
{
    INIT_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_437
Method which must be called first in a destructor
*/
void SerializeConstructorIncludeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_5535
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SerializeConstructorIncludeMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_5536
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SerializeConstructorIncludeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SerializeConstructorIncludeMethod* pSerializeConstructorIncludeMethod = (SerializeConstructorIncludeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5538
Save the state of the current object relations to pDataModelDocObject.
*/
void SerializeConstructorIncludeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
    SerializeConstructorIncludeMethod* pSerializeConstructorIncludeMethod = (SerializeConstructorIncludeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_440
Serialize the members only to a CbObject object
*/
void SerializeConstructorIncludeMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_439
Method which must be called first in a serialize constructor
*/
void SerializeConstructorIncludeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_442
Serialize the relations to a CbObject object
*/
void SerializeConstructorIncludeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(SerializeConstructorIncludeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
