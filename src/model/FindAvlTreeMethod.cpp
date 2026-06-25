/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindAvlTreeMethod.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindAvlTreeMethod'
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


/*@NOTE_35454
Constructor needed for serialization, not meant to use for other purposes!
*/
FindAvlTreeMethod::FindAvlTreeMethod() //@INIT_35454
    : FindMethod()
{//@CODE_35454
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_35454


/*@NOTE_35746
Constructor method.
*/
FindAvlTreeMethod::FindAvlTreeMethod(AvlTree* pAvlTree) //@INIT_35746
    : FindMethod(pAvlTree->GetRelation()->GetFromRelation(), false)
{//@CODE_35746
    ConstructorInclude(pAvlTree);

    // Put in your own code
    SetPhase(Complete_Phase);
    (void)new MemberArgument(this, pAvlTree->GetMember());
}//@CODE_35746


/*@NOTE_35451
Destructor method.
*/
FindAvlTreeMethod::~FindAvlTreeMethod()
{//@CODE_35451
    DestructorInclude();

    // Put in your own code
}//@CODE_35451


void FindAvlTreeMethod::InitCode()
{//@CODE_35766
    _code.Empty();

    if (GetArgumentCount())
    {
        Relation* pRelation = GetAvlTree()->GetRelation();
        Member* pMember = GetAvlTree()->GetMember();
        _code += "BODY_";
        if (pRelation->GetCritical())
            _code += "CRITICAL_";
        _code += "AVLTREE_FIND(";
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
}//@CODE_35766


int FindAvlTreeMethod::IsFixed() const
{//@CODE_35762
    return 1;
}//@CODE_35762


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_35461
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindAvlTreeMethod::CleanupReferences()
{
    FindMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
}


/*@NOTE_35452
Method which must be called first in a constructor.
*/
void FindAvlTreeMethod::ConstructorInclude(AvlTree* pAvlTree)
{
    INIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
}


/*@NOTE_35453
Method which must be called first in a destructor.
*/
void FindAvlTreeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
}


/*@NOTE_35462
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindAvlTreeMethod::RemoveReferences()
{
    FindMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
}


/*@NOTE_35463
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindAvlTreeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FindAvlTreeMethod* pFindAvlTreeMethod = (FindAvlTreeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
    FindMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_35465
Save the state of the current object relations to pDataModelDocObject.
*/
void FindAvlTreeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FindMethod::SaveReferences(pDataModelDocObject);
    FindAvlTreeMethod* pFindAvlTreeMethod = (FindAvlTreeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
}


/*@NOTE_35456
Serialize the members only to a CbObject object.
*/
void FindAvlTreeMethod::Serialize(CbArchive& archive)
{
    FindMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_35455
Method which must be called first in a serialize constructor.
*/
void FindAvlTreeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
}


/*@NOTE_35458
Serialize the relations to a CbObject object.
*/
void FindAvlTreeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FindAvlTreeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
