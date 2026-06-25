/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindEqualOrSmallerAvlTreeMethod.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindEqualOrSmallerAvlTreeMethod'
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


/*@NOTE_35492
Constructor needed for serialization, not meant to use for other purposes!
*/
FindEqualOrSmallerAvlTreeMethod::FindEqualOrSmallerAvlTreeMethod() //@INIT_35492
    : FindMethod()
{//@CODE_35492
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_35492


/*@NOTE_35754
Constructor method.
*/
FindEqualOrSmallerAvlTreeMethod::FindEqualOrSmallerAvlTreeMethod(AvlTree* pAvlTree) //@INIT_35754
    : FindMethod(pAvlTree->GetRelation()->GetFromRelation(), false)
{//@CODE_35754
    ConstructorInclude(pAvlTree);

    // Put in your own code
    Variable::SetName("FindEqualOrSmaller" + pAvlTree->GetRelation()->GetToName());

    SetPhase(Complete_Phase);
    (void)new MemberArgument(this, pAvlTree->GetMember());
}//@CODE_35754


/*@NOTE_35489
Destructor method.
*/
FindEqualOrSmallerAvlTreeMethod::~FindEqualOrSmallerAvlTreeMethod()
{//@CODE_35489
    DestructorInclude();

    // Put in your own code
}//@CODE_35489


void FindEqualOrSmallerAvlTreeMethod::InitCode()
{//@CODE_35768
    _code.Empty();

    if (GetArgumentCount())
    {
        Relation* pRelation = GetAvlTree()->GetRelation();
        Member* pMember = GetAvlTree()->GetMember();
        _code += "BODY_";
        if (pRelation->GetCritical())
            _code += "CRITICAL_";
        _code += "AVLTREE_FINDEQUALORSMALLER(";
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
}//@CODE_35768


int FindEqualOrSmallerAvlTreeMethod::IsFixed() const
{//@CODE_35764
    return 1;
}//@CODE_35764


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_35499
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindEqualOrSmallerAvlTreeMethod::CleanupReferences()
{
    FindMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
}


/*@NOTE_35490
Method which must be called first in a constructor.
*/
void FindEqualOrSmallerAvlTreeMethod::ConstructorInclude(AvlTree* pAvlTree)
{
    INIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
}


/*@NOTE_35491
Method which must be called first in a destructor.
*/
void FindEqualOrSmallerAvlTreeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
}


/*@NOTE_35500
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindEqualOrSmallerAvlTreeMethod::RemoveReferences()
{
    FindMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
}


/*@NOTE_35501
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindEqualOrSmallerAvlTreeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FindEqualOrSmallerAvlTreeMethod* pFindEqualOrSmallerAvlTreeMethod = (FindEqualOrSmallerAvlTreeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
    FindMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_35503
Save the state of the current object relations to pDataModelDocObject.
*/
void FindEqualOrSmallerAvlTreeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FindMethod::SaveReferences(pDataModelDocObject);
    FindEqualOrSmallerAvlTreeMethod* pFindEqualOrSmallerAvlTreeMethod = (FindEqualOrSmallerAvlTreeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
}


/*@NOTE_35494
Serialize the members only to a CbObject object.
*/
void FindEqualOrSmallerAvlTreeMethod::Serialize(CbArchive& archive)
{
    FindMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_35493
Method which must be called first in a serialize constructor.
*/
void FindEqualOrSmallerAvlTreeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
}


/*@NOTE_35496
Serialize the relations to a CbObject object.
*/
void FindEqualOrSmallerAvlTreeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FindEqualOrSmallerAvlTreeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
