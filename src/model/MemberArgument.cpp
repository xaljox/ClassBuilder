/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberArgument.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MemberArgument'
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


MemberArgument::MemberArgument(Method* pMethod, Member* pMember) //@INIT_918
    : Argument(pMethod, pMember->GetType())
{//@CODE_918
    ConstructorInclude(pMember);

    // Put in your own code
    SetPointer(pMember->GetPointer());
    //SetConstPointer(pMember->GetConstPointer());
    SetPointerPointer(pMember->GetPointerPointer());
    SetReference(pMember->GetReference());
    if (pMember->GetPointer())
    {
        SetConst(pMember->GetConst());
    }

    if (pMember->GetType()->IsBaseClass())
    {
        if (!pMember->GetPointer())
        {
            SetReference(1);
            SetConst(1);
        }
    }

    UpdateName();
}//@CODE_918


/*@NOTE_308
Constructor needed for serialization, not meant to use for other purposes!
*/
MemberArgument::MemberArgument() //@INIT_308
    : Argument()
{//@CODE_308
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_308


/*@NOTE_306
Destructor method
*/
MemberArgument::~MemberArgument()
{//@CODE_306
    DestructorInclude();

    // Put in your own code
}//@CODE_306


void MemberArgument::UpdateName()
{//@CODE_921
    if (GetMember()->GetPrefixedName() == GetMember()->GetName())
    {
        SetName(GetMember()->GetName() + "_");
    }
    else
    {
        SetName(GetMember()->GetName());
    }
}//@CODE_921


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5480
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MemberArgument::CleanupReferences()
{
    Argument::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)
}


/*@NOTE_305
Method which must be called first in a constructor
*/
void MemberArgument::ConstructorInclude(Member* pMember)
{
    INIT_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)
}


/*@NOTE_307
Method which must be called first in a destructor
*/
void MemberArgument::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)
}


/*@NOTE_5481
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MemberArgument::RemoveReferences()
{
    Argument::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)
}


/*@NOTE_5482
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MemberArgument::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberArgument* pMemberArgument = (MemberArgument*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)
    Argument::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5484
Save the state of the current object relations to pDataModelDocObject.
*/
void MemberArgument::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Argument::SaveReferences(pDataModelDocObject);
    MemberArgument* pMemberArgument = (MemberArgument*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)
}


/*@NOTE_310
Serialize the members only to a CbObject object
*/
void MemberArgument::Serialize(CbArchive& archive)
{
    Argument::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_309
Method which must be called first in a serialize constructor
*/
void MemberArgument::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Member, Member, MemberArgument, MemberArgument)
}


/*@NOTE_312
Serialize the relations to a CbObject object
*/
void MemberArgument::SerializeRelations(CbArchive& archive,
                                        DataModelDocObject* pointerArray[])
{
    Argument::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(MemberArgument)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
