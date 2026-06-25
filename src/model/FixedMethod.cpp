/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FixedMethod.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FixedMethod'
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


FixedMethod::FixedMethod(BaseClass* pBaseClass, Type* pType) //@INIT_988
    : Method(pBaseClass, pType)
{//@CODE_988
    ConstructorInclude();

    // Put in your own code
	_untouched = 0;
    SetPhase(Complete_Phase);
}//@CODE_988


/*@NOTE_412
Constructor needed for serialization, not meant to use for other purposes!
*/
FixedMethod::FixedMethod() //@INIT_412
    : Method()
{//@CODE_412
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_412


/*@NOTE_410
Destructor method
*/
FixedMethod::~FixedMethod()
{//@CODE_410
    DestructorInclude();

    // Put in your own code
}//@CODE_410


bool FixedMethod::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1371
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        value = Method::Drag(ctrlKeyDown, pGtiDropDefault);
    }

    return value;
}//@CODE_1371


int FixedMethod::IsFixed() const
{//@CODE_35419
    return 1;
}//@CODE_35419


int FixedMethod::OnAddArgument(bool checkOnly)
{//@CODE_992
    return 0;
}//@CODE_992


int FixedMethod::OnDelete(bool checkOnly)
{//@CODE_991
    return 0;
}//@CODE_991


int FixedMethod::OnEditExceptionSpecification(bool checkOnly)
{//@CODE_22712
    return 0;
}//@CODE_22712


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5420
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FixedMethod::CleanupReferences()
{
    Method::CleanupReferences();
}


/*@NOTE_409
Method which must be called first in a constructor
*/
void FixedMethod::ConstructorInclude()
{
}


/*@NOTE_411
Method which must be called first in a destructor
*/
void FixedMethod::DestructorInclude()
{
}


/*@NOTE_5421
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FixedMethod::RemoveReferences()
{
    Method::RemoveReferences();
}


/*@NOTE_5422
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FixedMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5424
Save the state of the current object relations to pDataModelDocObject.
*/
void FixedMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::SaveReferences(pDataModelDocObject);
}


/*@NOTE_414
Serialize the members only to a CbObject object
*/
void FixedMethod::Serialize(CbArchive& archive)
{
    Method::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_413
Method which must be called first in a serialize constructor
*/
void FixedMethod::SerializeConstructorInclude()
{
}


/*@NOTE_416
Serialize the relations to a CbObject object
*/
void FixedMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FixedMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
