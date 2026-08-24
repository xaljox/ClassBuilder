/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          UniqueValueTree.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UniqueValueTree'
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


/*@NOTE_1705
Constructor needed for serialization, not meant to use for other purposes!
*/
UniqueValueTree::UniqueValueTree() //@INIT_1705
    : RelationMember()
{//@CODE_1705
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1705


UniqueValueTree::UniqueValueTree(Relation* pRelation,
                                 Member* pMember) //@INIT_1714
    : RelationMember(pRelation, pMember)
{//@CODE_1714
    ConstructorInclude();

    // Add Find method with as argument the key member
    FindUniqueValueTreeMethod* pFindUniqueValueTreeMethod = new FindUniqueValueTreeMethod(this);
    pFindUniqueValueTreeMethod->GetBaseClass()->NotifyAddMethod(pFindUniqueValueTreeMethod);
    if (pRelation->GetFromRelation()->GetAdded())
    {
        pFindUniqueValueTreeMethod->Add();
    }

    // Delete all Find methods with as only argument the key member of this relation
    // Update all other.
    FromRelation::MethodIterator iMethod(pRelation->GetFromRelation(), &FromRelationMethod::IsFindMethod);
    while (++iMethod)
    {
        FindMethod* pFindMethod = (FindMethod*)iMethod.Get();
        Method::ArgumentIterator iArgument(pFindMethod, &Argument::IsMemberArgument);
        while (++iArgument)
        {
            MemberArgument* pMemberArgument = (MemberArgument*)iArgument.Get();
            if (pMemberArgument->GetMember() == pMember)
            {
                if (iMethod->GetArgumentCount() == 1 && 
                    iMethod.Get() != pFindUniqueValueTreeMethod)
                {
                    pFindMethod->Delete();
                }
                else
                {
                    CbString name;
                    if (pFindMethod->GetReverse())
                        name = "startBefore" + pFindMethod->GetType()->GetName();
                    else
                        name = "startAfter" + pFindMethod->GetType()->GetName();

                    Argument* pArgument = pFindMethod->FindArgument(name);
                    if (pArgument)
                        pArgument->Delete();

                    pFindMethod->SetNext(0);
                    pFindMethod->SetReverse(0);
                    pFindMethod->InitCode();
                }
                break;
            }
        }
    }
}//@CODE_1714


/*@NOTE_1703
Destructor method
*/
UniqueValueTree::~UniqueValueTree()
{//@CODE_1703
    DestructorInclude();

    // Put in your own code
}//@CODE_1703


int UniqueValueTree::GetImplementation()
{//@CODE_1721
    return 1;
}//@CODE_1721


CbString UniqueValueTree::InitCodeFindMethod(FindMethod* pFindMethod,
                                             Argument* pArgument)
{//@CODE_1789
    CbString code;
    FindMethod* pUsedFindMethod;

    pUsedFindMethod = GetFindUniqueValueTreeMethod();

    if (pUsedFindMethod)
    {
        code += GetIndent() + pFindMethod->GetType()->GetName() + "* result = " + 
                pUsedFindMethod->GetName() + "(" + pArgument->GetName() + ");" NL NL;

        Relation* pRelation = GetRelation();
        code += GetIndent() + "if (result)" NL;
        code += GetIndent() + "{" NL;
        code += GetIndent(2) + "if (";
        bool first = true;
        Method::ArgumentIterator iArgument(pFindMethod);
        while (++iArgument)
        {
            if (iArgument.Get() != pArgument && !iArgument->GetPath().IsEmpty())
            {
                if (first)
                    first = false;
                else
                    code += " &&" NL + GetIndent(3);

                code += iArgument->GetName() + " == result" + iArgument->GetPath();
            }
        }
        code += ")" NL;
        code += GetIndent(2) + "{" NL;
        code += GetIndent(3) + "return result;" NL;
        code += GetIndent(2) + "}" NL;
        code += GetIndent() + "}" NL NL;

        code += GetIndent() + "return 0;" NL;
    }

    return code;
}//@CODE_1789


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5588
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void UniqueValueTree::CleanupReferences()
{
    RelationMember::CleanupReferences();
}


/*@NOTE_1702
Method which must be called first in a constructor
*/
void UniqueValueTree::ConstructorInclude()
{
    INIT_SINGLE_OWNED_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)
}


/*@NOTE_1704
Method which must be called first in a destructor
*/
void UniqueValueTree::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)
}


/*@NOTE_5589
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void UniqueValueTree::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)
    RelationMember::RemoveReferences();
}


/*@NOTE_5590
Bring the current object relations into the same state as pDataModelDocObject.
*/
void UniqueValueTree::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5592
Save the state of the current object relations to pDataModelDocObject.
*/
void UniqueValueTree::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1707
Serialize the members only to a CbObject object
*/
void UniqueValueTree::Serialize(CbArchive& archive)
{
    RelationMember::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1706
Method which must be called first in a serialize constructor
*/
void UniqueValueTree::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)
}


/*@NOTE_1709
Serialize the relations to a CbObject object
*/
void UniqueValueTree::SerializeRelations(CbArchive& archive,
                                         DataModelDocObject* pointerArray[])
{
    RelationMember::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(UniqueValueTree)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
