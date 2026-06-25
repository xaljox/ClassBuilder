/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          GetMemberMethod.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'GetMemberMethod'
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


/*@NOTE_256
Constructor needed for serialization, not meant to use for other purposes!
*/
GetMemberMethod::GetMemberMethod() //@INIT_256
    : MemberMethod()
{//@CODE_256
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_256


GetMemberMethod::GetMemberMethod(Member* pMember) //@INIT_904
    : MemberMethod(pMember, pMember->GetType())
{//@CODE_904
    ConstructorInclude(pMember);

    // Put in your own code
    SetInline(1);
    SetConst(1);

    SetPointer(pMember->GetPointer());
    SetConstPointer(pMember->GetConstPointer());
    SetPointerPointer(pMember->GetPointerPointer());
    SetReference(pMember->GetReference());
    Variable::SetConst(pMember->GetConst());
    if (pMember->GetType()->IsBaseClass())
    {
        if (!pMember->GetPointer())
        {
            SetReference(1);
            Variable::SetConst(1);
        }
    }

    CbString name = pMember->GetName();
    if (pMember->GetPointer() && name[0] == 'p')
    {
        name = name.Mid(1);
    } 
    if (pMember->GetPointerPointer() && name[0] == 'p')
    {
        name = name.Mid(1);
    } 
    if (pMember->GetReference() && name[0] == 'r')
    {
        name = name.Mid(1);
    }
    if (islower(name[0]))
    {
        name = pMember->GetName();
    }
    name.SetAt(0, toupper(name[0]));

    SetName("Get" + name);

    CbString note;
    note.Format("Returns the value of member '%s'.", pMember->GetPrefixedName());
    SetNote(note);

    SetTemplate(pMember->GetTemplate());
}//@CODE_904


/*@NOTE_254
Destructor method
*/
GetMemberMethod::~GetMemberMethod()
{//@CODE_254
    DestructorInclude();

    // Put in your own code
}//@CODE_254


void GetMemberMethod::InitCode()
{//@CODE_906
    _code = GetIndent() + "return ";
    
    _code += GetMember()->GetPrefixedName() + ";" NL;

    SetPhaseUpwards(Complete_Phase);
}//@CODE_906


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5438
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void GetMemberMethod::CleanupReferences()
{
    MemberMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
}


/*@NOTE_253
Method which must be called first in a constructor
*/
void GetMemberMethod::ConstructorInclude(Member* pMember)
{
    INIT_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
}


/*@NOTE_255
Method which must be called first in a destructor
*/
void GetMemberMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
}


/*@NOTE_5439
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void GetMemberMethod::RemoveReferences()
{
    MemberMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
}


/*@NOTE_5440
Bring the current object relations into the same state as pDataModelDocObject.
*/
void GetMemberMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    GetMemberMethod* pGetMemberMethod = (GetMemberMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
    MemberMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5442
Save the state of the current object relations to pDataModelDocObject.
*/
void GetMemberMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberMethod::SaveReferences(pDataModelDocObject);
    GetMemberMethod* pGetMemberMethod = (GetMemberMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
}


/*@NOTE_258
Serialize the members only to a CbObject object
*/
void GetMemberMethod::Serialize(CbArchive& archive)
{
    MemberMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_257
Method which must be called first in a serialize constructor
*/
void GetMemberMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)
}


/*@NOTE_260
Serialize the relations to a CbObject object
*/
void GetMemberMethod::SerializeRelations(CbArchive& archive,
                                         DataModelDocObject* pointerArray[])
{
    MemberMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(GetMemberMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
