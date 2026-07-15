/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindReverseValueTreeMethod.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindReverseValueTreeMethod'
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


/*@NOTE_1799
Constructor needed for serialization, not meant to use for other purposes!
*/
FindReverseValueTreeMethod::FindReverseValueTreeMethod() //@INIT_1799
    : FindMethod()
{//@CODE_1799
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1799


FindReverseValueTreeMethod::FindReverseValueTreeMethod(ValueTree* pValueTree) //@INIT_1812
    : FindMethod(pValueTree->GetRelation()->GetFromRelation(), true)
{//@CODE_1812
    ConstructorInclude(pValueTree);

    // Put in your own code
    SetPhase(Complete_Phase);
    (void)new MemberArgument(this, pValueTree->GetMember());
}//@CODE_1812


/*@NOTE_1797
Destructor method
*/
FindReverseValueTreeMethod::~FindReverseValueTreeMethod()
{//@CODE_1797
    DestructorInclude();

    // Put in your own code
}//@CODE_1797


void FindReverseValueTreeMethod::InitCode()
{//@CODE_1814
    _code.Empty();

    if (GetArgumentCount())
    {
        Relation* pRelation = GetValueTree()->GetRelation();
        Member* pMember = GetValueTree()->GetMember();
        _code += "BODY_";
        if (pRelation->GetCritical())
            _code += "CRITICAL_";
        _code += "VALUETREE_FINDREVERSE(";
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
}//@CODE_1814


int FindReverseValueTreeMethod::IsFixed() const
{//@CODE_35422
    return 1;
}//@CODE_35422


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5612
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindReverseValueTreeMethod::CleanupReferences()
{
    FindMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_1796
Method which must be called first in a constructor
*/
void FindReverseValueTreeMethod::ConstructorInclude(ValueTree* pValueTree)
{
    INIT_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_1798
Method which must be called first in a destructor
*/
void FindReverseValueTreeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_5613
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindReverseValueTreeMethod::RemoveReferences()
{
    FindMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_5614
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindReverseValueTreeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FindReverseValueTreeMethod* pFindReverseValueTreeMethod = (FindReverseValueTreeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
    FindMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5616
Save the state of the current object relations to pDataModelDocObject.
*/
void FindReverseValueTreeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FindMethod::SaveReferences(pDataModelDocObject);
    FindReverseValueTreeMethod* pFindReverseValueTreeMethod = (FindReverseValueTreeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_1801
Serialize the members only to a CbObject object
*/
void FindReverseValueTreeMethod::Serialize(CbArchive& archive)
{
    FindMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1800
Method which must be called first in a serialize constructor
*/
void FindReverseValueTreeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_1803
Serialize the relations to a CbObject object
*/
void FindReverseValueTreeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FindReverseValueTreeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
