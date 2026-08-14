/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberMethod.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MemberMethod'
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


/*@NOTE_191
Constructor needed for serialization, not meant to use for other purposes!
*/
MemberMethod::MemberMethod() //@INIT_191
    : Method()
{//@CODE_191
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_191


MemberMethod::MemberMethod(Member* pMember, Type* pType) //@INIT_844
    : Method(pMember->GetBaseClass(), pType)
{//@CODE_844
    ConstructorInclude(pMember);

    // Put in your own code
	_untouched = 0;
    SetPhase(Complete_Phase);
}//@CODE_844


/*@NOTE_189
Destructor method
*/
MemberMethod::~MemberMethod()
{//@CODE_189
    DestructorInclude();

    // Put in your own code
}//@CODE_189


void MemberMethod::Add()
{//@CODE_847
    if (!GetAdded())
    {
        SaveState(1);
        GetMember()->AddChildLast(this);

        Method::Add();
    }
}//@CODE_847


bool MemberMethod::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1380
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = Method::Drag(ctrlKeyDown, pGtiDropDefault);
    }
    else
    {
    }

    return value;
}//@CODE_1380


bool MemberMethod::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1383
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        Member* pDropMember = dynamic_cast<Member*>(pGtiDrop);

        if (pDropMember && pDropMember->GetType() == GetBaseClass())
            value = true;
    }
    else
    {
        value = Method::DropTarget(ctrlKeyDown, pGtiDrop);
    }

    return value;
}//@CODE_1383


CbString MemberMethod::GetEndContextDeclaration()
{//@CODE_27315
    CbString str;
    
    MethodContextIterator iMethodContext(this);
    while (--iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetEndContextDeclaration();
    }
    
    str += GetMember()->GetEndContextDeclaration();
    
    return str;
}//@CODE_27315


CbString MemberMethod::GetEndContextImplementation()
{//@CODE_27316
    CbString str;
    
    MethodContextIterator iMethodContext(this);
    while (--iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetEndContextImplementation();
    }

    str += GetMember()->GetEndContextImplementation();
    
    return str;
}//@CODE_27316


CbString MemberMethod::GetStartContextDeclaration()
{//@CODE_27313
    CbString str = GetMember()->GetStartContextDeclaration();

    MethodContextIterator iMethodContext(this);
    while (++iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetStartContextDeclaration();
    }
    
    return str;
}//@CODE_27313


CbString MemberMethod::GetStartContextImplementation()
{//@CODE_27314
    CbString str = GetMember()->GetStartContextImplementation();
    
    MethodContextIterator iMethodContext(this);
    while (++iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetStartContextImplementation();
    }
    
    return str;
}//@CODE_27314


int MemberMethod::OnAddArgument(bool checkOnly)
{//@CODE_4735
    return 0;
}//@CODE_4735


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5486
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MemberMethod::CleanupReferences()
{
    Method::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)
}


/*@NOTE_188
Method which must be called first in a constructor
*/
void MemberMethod::ConstructorInclude(Member* pMember)
{
    INIT_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)
}


/*@NOTE_190
Method which must be called first in a destructor
*/
void MemberMethod::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)
}


/*@NOTE_5487
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MemberMethod::RemoveReferences()
{
    Method::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)
}


/*@NOTE_5488
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MemberMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberMethod* pMemberMethod = (MemberMethod*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)
    Method::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5490
Save the state of the current object relations to pDataModelDocObject.
*/
void MemberMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::SaveReferences(pDataModelDocObject);
    MemberMethod* pMemberMethod = (MemberMethod*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)
}


/*@NOTE_193
Serialize the members only to a CbObject object
*/
void MemberMethod::Serialize(CbArchive& archive)
{
    Method::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_192
Method which must be called first in a serialize constructor
*/
void MemberMethod::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Member, Member, MemberMethod, Method)
}


/*@NOTE_195
Serialize the relations to a CbObject object
*/
void MemberMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(MemberMethod)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
