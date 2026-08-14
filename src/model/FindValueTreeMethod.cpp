/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindValueTreeMethod.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindValueTreeMethod'
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


/*@NOTE_1759
Constructor needed for serialization, not meant to use for other purposes!
*/
FindValueTreeMethod::FindValueTreeMethod() //@INIT_1759
    : FindMethod()
{//@CODE_1759
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1759


FindValueTreeMethod::FindValueTreeMethod(ValueTree* pValueTree) //@INIT_1768
    : FindMethod(pValueTree->GetRelation()->GetFromRelation(), false)
{//@CODE_1768
    ConstructorInclude(pValueTree);

    // Put in your own code
    SetPhase(Complete_Phase);
    (void)new MemberArgument(this, pValueTree->GetMember());
}//@CODE_1768


/*@NOTE_1757
Destructor method
*/
FindValueTreeMethod::~FindValueTreeMethod()
{//@CODE_1757
    DestructorInclude();

    // Put in your own code
}//@CODE_1757


void FindValueTreeMethod::InitCode()
{//@CODE_1773
    _code.Empty();

    if (GetArgumentCount())
    {
        Relation* pRelation = GetValueTree()->GetRelation();
        Member* pMember = GetValueTree()->GetMember();
        _code += "BODY_";
        if (pRelation->GetCritical())
            _code += "CRITICAL_";
        _code += "VALUETREE_FIND(";
        if (pMember->GetGetMemberMethod())
            _code += pMember->GetGetMemberMethod()->GetName() + "(), ";
        else
            _code += pMember->GetPrefixedName() + ", ";
        _code += GetFirstArgument()->GetName() + ", " +
                 pRelation->GetFromClass()->Type::GetName() + pRelation->GetFromClass()->GetTemplateDefine() + ", " +
                 pRelation->GetFromName() + ", " +
                 pRelation->GetToClass()->Type::GetName() + pRelation->GetToClass()->GetTemplateDefine() + ", " +
                 pRelation->GetToName() + ")";
    }

        else
    {
        _code = GetIndent() + _code;
    }

    _code += NL;

    SetPhaseUpwards(Complete_Phase);
}//@CODE_1773


int FindValueTreeMethod::IsFixed() const
{//@CODE_35421
    return 1;
}//@CODE_35421


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5606
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindValueTreeMethod::CleanupReferences()
{
    FindMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
}


/*@NOTE_1756
Method which must be called first in a constructor
*/
void FindValueTreeMethod::ConstructorInclude(ValueTree* pValueTree)
{
    INIT_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
}


/*@NOTE_1758
Method which must be called first in a destructor
*/
void FindValueTreeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
}


/*@NOTE_5607
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindValueTreeMethod::RemoveReferences()
{
    FindMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
}


/*@NOTE_5608
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindValueTreeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FindValueTreeMethod* pFindValueTreeMethod = (FindValueTreeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
    FindMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5610
Save the state of the current object relations to pDataModelDocObject.
*/
void FindValueTreeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FindMethod::SaveReferences(pDataModelDocObject);
    FindValueTreeMethod* pFindValueTreeMethod = (FindValueTreeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
}


/*@NOTE_1761
Serialize the members only to a CbObject object
*/
void FindValueTreeMethod::Serialize(CbArchive& archive)
{
    FindMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1760
Method which must be called first in a serialize constructor
*/
void FindValueTreeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
}


/*@NOTE_1763
Serialize the relations to a CbObject object
*/
void FindValueTreeMethod::SerializeRelations(CbArchive& archive,
                                             DataModelDocObject* pointerArray[])
{
    FindMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(FindValueTreeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
