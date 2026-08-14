/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MemberAndMethodGroupContext.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MemberAndMethodGroupContext'
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


/*@NOTE_25327
Constructor method.
*/
MemberAndMethodGroupContext::MemberAndMethodGroupContext(MemberAndMethodGroup* pMemberAndMethodGroup,
                                                         ContextDeclaration* pContextDeclaration) //@INIT_25327
    : Context(pContextDeclaration)
{//@CODE_25327
    ConstructorInclude(pMemberAndMethodGroup);

    // Put in your own code
}//@CODE_25327


/*@NOTE_24016
Constructor needed for serialization, not meant to use for other purposes!
*/
MemberAndMethodGroupContext::MemberAndMethodGroupContext() //@INIT_24016
    : Context()
{//@CODE_24016
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_24016


/*@NOTE_24014
Destructor method.
*/
MemberAndMethodGroupContext::~MemberAndMethodGroupContext()
{//@CODE_24014
    DestructorInclude();

    // Put in your own code
}//@CODE_24014


/*@NOTE_25938
Returns a pointer to the object to which this context applies.
*/
Gti* MemberAndMethodGroupContext::GetContextObject()
{//@CODE_25938
    return GetMemberAndMethodGroup();
}//@CODE_25938


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_24023
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MemberAndMethodGroupContext::CleanupReferences()
{
    Context::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
}


/*@NOTE_24013
Method which must be called first in a constructor.
*/
void MemberAndMethodGroupContext::ConstructorInclude(MemberAndMethodGroup* pMemberAndMethodGroup)
{
    INIT_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
}


/*@NOTE_24015
Method which must be called first in a destructor.
*/
void MemberAndMethodGroupContext::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
}


/*@NOTE_24024
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MemberAndMethodGroupContext::RemoveReferences()
{
    Context::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
}


/*@NOTE_24025
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MemberAndMethodGroupContext::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberAndMethodGroupContext* pMemberAndMethodGroupContext = (MemberAndMethodGroupContext*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    Context::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_24027
Save the state of the current object relations to pDataModelDocObject.
*/
void MemberAndMethodGroupContext::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Context::SaveReferences(pDataModelDocObject);
    MemberAndMethodGroupContext* pMemberAndMethodGroupContext = (MemberAndMethodGroupContext*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
}


/*@NOTE_24018
Serialize the members only to a CbObject object.
*/
void MemberAndMethodGroupContext::Serialize(CbArchive& archive)
{
    Context::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_24017
Method which must be called first in a serialize constructor.
*/
void MemberAndMethodGroupContext::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
}


/*@NOTE_24020
Serialize the relations to a CbObject object.
*/
void MemberAndMethodGroupContext::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(MemberAndMethodGroupContext)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
