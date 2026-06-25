/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindReverseAvlTreeMethod.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindReverseAvlTreeMethod'
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


/*@NOTE_35473
Constructor needed for serialization, not meant to use for other purposes!
*/
FindReverseAvlTreeMethod::FindReverseAvlTreeMethod() //@INIT_35473
    : FindMethod()
{//@CODE_35473
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_35473


/*@NOTE_35750
Constructor method.
*/
FindReverseAvlTreeMethod::FindReverseAvlTreeMethod(AvlTree* pAvlTree) //@INIT_35750
    : FindMethod(pAvlTree->GetRelation()->GetFromRelation(), true)
{//@CODE_35750
    ConstructorInclude(pAvlTree);

    // Put in your own code
    SetPhase(Complete_Phase);
    (void)new MemberArgument(this, pAvlTree->GetMember());
}//@CODE_35750


/*@NOTE_35470
Destructor method.
*/
FindReverseAvlTreeMethod::~FindReverseAvlTreeMethod()
{//@CODE_35470
    DestructorInclude();

    // Put in your own code
}//@CODE_35470


void FindReverseAvlTreeMethod::InitCode()
{//@CODE_35767
    _code.Empty();

    if (GetArgumentCount())
    {
        Relation* pRelation = GetAvlTree()->GetRelation();
        Member* pMember = GetAvlTree()->GetMember();
        _code += "BODY_";
        if (pRelation->GetCritical())
            _code += "CRITICAL_";
        _code += "AVLTREE_FINDREVERSE(";
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
}//@CODE_35767


int FindReverseAvlTreeMethod::IsFixed() const
{//@CODE_35763
    return 1;
}//@CODE_35763


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_35480
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindReverseAvlTreeMethod::CleanupReferences()
{
    FindMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
}


/*@NOTE_35471
Method which must be called first in a constructor.
*/
void FindReverseAvlTreeMethod::ConstructorInclude(AvlTree* pAvlTree)
{
    INIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
}


/*@NOTE_35472
Method which must be called first in a destructor.
*/
void FindReverseAvlTreeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
}


/*@NOTE_35481
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindReverseAvlTreeMethod::RemoveReferences()
{
    FindMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
}


/*@NOTE_35482
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindReverseAvlTreeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FindReverseAvlTreeMethod* pFindReverseAvlTreeMethod = (FindReverseAvlTreeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
    FindMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_35484
Save the state of the current object relations to pDataModelDocObject.
*/
void FindReverseAvlTreeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FindMethod::SaveReferences(pDataModelDocObject);
    FindReverseAvlTreeMethod* pFindReverseAvlTreeMethod = (FindReverseAvlTreeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
}


/*@NOTE_35475
Serialize the members only to a CbObject object.
*/
void FindReverseAvlTreeMethod::Serialize(CbArchive& archive)
{
    FindMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_35474
Method which must be called first in a serialize constructor.
*/
void FindReverseAvlTreeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
}


/*@NOTE_35477
Serialize the relations to a CbObject object.
*/
void FindReverseAvlTreeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FindReverseAvlTreeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
