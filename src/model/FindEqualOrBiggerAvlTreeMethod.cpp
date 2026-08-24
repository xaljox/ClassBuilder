/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FindEqualOrBiggerAvlTreeMethod.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindEqualOrBiggerAvlTreeMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
//@END_USER2


// Static members


/*@NOTE_35511
Constructor needed for serialization, not meant to use for other purposes!
*/
FindEqualOrBiggerAvlTreeMethod::FindEqualOrBiggerAvlTreeMethod() //@INIT_35511
    : FindMethod()
{//@CODE_35511
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_35511


/*@NOTE_35758
Constructor method.
*/
FindEqualOrBiggerAvlTreeMethod::FindEqualOrBiggerAvlTreeMethod(AvlTree* pAvlTree) //@INIT_35758
    : FindMethod(pAvlTree->GetRelation()->GetFromRelation(), false)
{//@CODE_35758
    ConstructorInclude(pAvlTree);

    // Put in your own code
    Variable::SetName("FindEqualOrBigger" + pAvlTree->GetRelation()->GetToName());

    SetPhase(Complete_Phase);
    (void)new MemberArgument(this, pAvlTree->GetMember());
}//@CODE_35758


/*@NOTE_35508
Destructor method.
*/
FindEqualOrBiggerAvlTreeMethod::~FindEqualOrBiggerAvlTreeMethod()
{//@CODE_35508
    DestructorInclude();

    // Put in your own code
}//@CODE_35508


void FindEqualOrBiggerAvlTreeMethod::InitCode()
{//@CODE_35769
    _code.Empty();

    if (GetArgumentCount())
    {
        Relation* pRelation = GetAvlTree()->GetRelation();
        Member* pMember = GetAvlTree()->GetMember();
        _code += "BODY_";
        if (pRelation->GetCritical())
            _code += "CRITICAL_";
        _code += "AVLTREE_FINDEQUALORBIGGER(";
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
}//@CODE_35769


int FindEqualOrBiggerAvlTreeMethod::IsFixed() const
{//@CODE_35765
    return 1;
}//@CODE_35765


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_35518
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindEqualOrBiggerAvlTreeMethod::CleanupReferences()
{
    FindMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35509
Method which must be called first in a constructor.
*/
void FindEqualOrBiggerAvlTreeMethod::ConstructorInclude(AvlTree* pAvlTree)
{
    INIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35510
Method which must be called first in a destructor.
*/
void FindEqualOrBiggerAvlTreeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35519
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindEqualOrBiggerAvlTreeMethod::RemoveReferences()
{
    FindMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35520
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindEqualOrBiggerAvlTreeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FindEqualOrBiggerAvlTreeMethod* pFindEqualOrBiggerAvlTreeMethod = (FindEqualOrBiggerAvlTreeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
    FindMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_35522
Save the state of the current object relations to pDataModelDocObject.
*/
void FindEqualOrBiggerAvlTreeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FindMethod::SaveReferences(pDataModelDocObject);
    FindEqualOrBiggerAvlTreeMethod* pFindEqualOrBiggerAvlTreeMethod = (FindEqualOrBiggerAvlTreeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35513
Serialize the members only to a CbObject object.
*/
void FindEqualOrBiggerAvlTreeMethod::Serialize(CbArchive& archive)
{
    FindMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_35512
Method which must be called first in a serialize constructor.
*/
void FindEqualOrBiggerAvlTreeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35515
Serialize the relations to a CbObject object.
*/
void FindEqualOrBiggerAvlTreeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(FindEqualOrBiggerAvlTreeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
