/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberContext.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MemberContext'
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


/*@NOTE_25188
Constructor method.
*/
MemberContext::MemberContext(Member* pMember,
                             ContextDeclaration* pContextDeclaration) //@INIT_25188
    : Context(pContextDeclaration)
{//@CODE_25188
    ConstructorInclude(pMember);

    // Put in your own code
}//@CODE_25188


/*@NOTE_23996
Constructor needed for serialization, not meant to use for other purposes!
*/
MemberContext::MemberContext() //@INIT_23996
    : Context()
{//@CODE_23996
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23996


/*@NOTE_23994
Destructor method.
*/
MemberContext::~MemberContext()
{//@CODE_23994
    DestructorInclude();

    // Put in your own code
}//@CODE_23994


/*@NOTE_25916
Returns a pointer to the object to which this context applies.
*/
Gti* MemberContext::GetContextObject()
{//@CODE_25916
    return GetMember();
}//@CODE_25916


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_24003
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MemberContext::CleanupReferences()
{
    Context::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)
}


/*@NOTE_23993
Method which must be called first in a constructor.
*/
void MemberContext::ConstructorInclude(Member* pMember)
{
    INIT_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)
}


/*@NOTE_23995
Method which must be called first in a destructor.
*/
void MemberContext::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)
}


/*@NOTE_24004
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MemberContext::RemoveReferences()
{
    Context::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)
}


/*@NOTE_24005
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MemberContext::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberContext* pMemberContext = (MemberContext*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)
    Context::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_24007
Save the state of the current object relations to pDataModelDocObject.
*/
void MemberContext::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Context::SaveReferences(pDataModelDocObject);
    MemberContext* pMemberContext = (MemberContext*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)
}


/*@NOTE_23998
Serialize the members only to a CbObject object.
*/
void MemberContext::Serialize(CbArchive& archive)
{
    Context::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_23997
Method which must be called first in a serialize constructor.
*/
void MemberContext::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Member, Member, MemberContext, MemberContext)
}


/*@NOTE_24000
Serialize the relations to a CbObject object.
*/
void MemberContext::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(MemberContext)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
