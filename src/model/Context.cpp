/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Context.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Context'
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


/*@NOTE_25109
Constructor method.
*/
Context::Context(ContextDeclaration* pContextDeclaration) //@INIT_25109
    : DataModelDocObject(pContextDeclaration->GetDataModelDoc())
{//@CODE_25109
    ConstructorInclude(pContextDeclaration);

    // Put in your own code
}//@CODE_25109


/*@NOTE_23862
Constructor needed for serialization, not meant to use for other purposes!
*/
Context::Context() //@INIT_23862
    : DataModelDocObject()
{//@CODE_23862
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23862


/*@NOTE_23860
Destructor method.
*/
Context::~Context()
{//@CODE_23860
    DestructorInclude();

    // Put in your own code
}//@CODE_23860


/*@NOTE_25889
Returns a pointer to the object to which this context applies.
*/
Gti* Context::GetContextObject()
{//@CODE_25889
    return 0;
}//@CODE_25889


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_23869
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Context::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
}


/*@NOTE_23859
Method which must be called first in a constructor.
*/
void Context::ConstructorInclude(ContextDeclaration* pContextDeclaration)
{
    INIT_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
}


/*@NOTE_23861
Method which must be called first in a destructor.
*/
void Context::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
}


/*@NOTE_23870
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Context::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
}


/*@NOTE_23871
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Context::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Context* pContext = (Context*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_23873
Save the state of the current object relations to pDataModelDocObject.
*/
void Context::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    Context* pContext = (Context*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
}


/*@NOTE_23864
Serialize the members only to a CbObject object.
*/
void Context::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_23863
Method which must be called first in a serialize constructor.
*/
void Context::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)
}


/*@NOTE_23866
Serialize the relations to a CbObject object.
*/
void Context::SerializeRelations(CbArchive& archive,
                                 DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Context)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(ContextDeclaration, ContextDeclaration, Context, Context)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
