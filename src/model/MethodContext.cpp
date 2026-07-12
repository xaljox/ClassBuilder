/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MethodContext.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MethodContext'
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


/*@NOTE_24036
Constructor needed for serialization, not meant to use for other purposes!
*/
MethodContext::MethodContext() //@INIT_24036
    : Context()
{//@CODE_24036
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_24036


/*@NOTE_25246
Constructor method.
*/
MethodContext::MethodContext(Method* pMethod,
                             ContextDeclaration* pContextDeclaration) //@INIT_25246
    : Context(pContextDeclaration)
{//@CODE_25246
    ConstructorInclude(pMethod);

    // Put in your own code
}//@CODE_25246


/*@NOTE_24034
Destructor method.
*/
MethodContext::~MethodContext()
{//@CODE_24034
    DestructorInclude();

    // Put in your own code
}//@CODE_24034


/*@NOTE_25950
Returns a pointer to the object to which this context applies.
*/
Gti* MethodContext::GetContextObject()
{//@CODE_25950
    return GetMethod();
}//@CODE_25950


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_24043
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MethodContext::CleanupReferences()
{
    Context::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)
}


/*@NOTE_24033
Method which must be called first in a constructor.
*/
void MethodContext::ConstructorInclude(Method* pMethod)
{
    INIT_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)
}


/*@NOTE_24035
Method which must be called first in a destructor.
*/
void MethodContext::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)
}


/*@NOTE_24044
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MethodContext::RemoveReferences()
{
    Context::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)
}


/*@NOTE_24045
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MethodContext::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MethodContext* pMethodContext = (MethodContext*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)
    Context::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_24047
Save the state of the current object relations to pDataModelDocObject.
*/
void MethodContext::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Context::SaveReferences(pDataModelDocObject);
    MethodContext* pMethodContext = (MethodContext*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)
}


/*@NOTE_24038
Serialize the members only to a CbObject object.
*/
void MethodContext::Serialize(CbArchive& archive)
{
    Context::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_24037
Method which must be called first in a serialize constructor.
*/
void MethodContext::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Method, Method, MethodContext, MethodContext)
}


/*@NOTE_24040
Serialize the relations to a CbObject object.
*/
void MethodContext::SerializeRelations(CbArchive& archive,
                                       DataModelDocObject* pointerArray[])
{
    Context::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(MethodContext)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
