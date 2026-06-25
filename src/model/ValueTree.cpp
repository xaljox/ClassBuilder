/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ValueTree.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ValueTree'
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


/*@NOTE_1746
Constructor needed for serialization, not meant to use for other purposes!
*/
ValueTree::ValueTree() //@INIT_1746
    : RelationMember()
{//@CODE_1746
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1746


ValueTree::ValueTree(Relation* pRelation, Member* pMember) //@INIT_1775
    : RelationMember(pRelation, pMember)
{//@CODE_1775
    ConstructorInclude();

    FindValueTreeMethod* pFindValueTreeMethod = new FindValueTreeMethod(this);
    FindReverseValueTreeMethod* pFindReverseValueTreeMethod = new FindReverseValueTreeMethod(this);
    pFindValueTreeMethod->GetBaseClass()->NotifyAddMethod(pFindValueTreeMethod);
    pFindReverseValueTreeMethod->GetBaseClass()->NotifyAddMethod(pFindReverseValueTreeMethod);
    if (pRelation->GetFromRelation()->GetAdded())
    {
        pFindValueTreeMethod->Add();
        pFindReverseValueTreeMethod->Add();
    }

    // Delete all Find methods with as only argument the key member of this relation
    // Update all other.
    FromRelation::MethodIterator iMethod(pRelation->GetFromRelation(), &FromRelationMethod::IsFindMethod);
    while (++iMethod)
    {
        Method::ArgumentIterator iArgument(iMethod, &Argument::IsMemberArgument);
        while (++iArgument)
        {
            MemberArgument* pMemberArgument = (MemberArgument*)iArgument.Get();
            if (pMemberArgument->GetMember() == pMember)
            {
                if (iMethod->GetArgumentCount() == 1 && 
                    iMethod.Get() != pFindValueTreeMethod &&
                    iMethod.Get() != pFindReverseValueTreeMethod)
                {
                    iMethod->Delete();
                }
                else
                    iMethod->InitCode();
                break;
            }
        }
    }
}//@CODE_1775


/*@NOTE_1744
Destructor method
*/
ValueTree::~ValueTree()
{//@CODE_1744
    DestructorInclude();

    // Put in your own code
}//@CODE_1744


int ValueTree::GetImplementation()
{//@CODE_1774
    return 1;
}//@CODE_1774


CbString ValueTree::InitCodeFindMethod(FindMethod* pFindMethod,
                                       Argument* pArgument)
{//@CODE_1791
    CbString code;
    FindMethod* pUsedFindMethod;

    CbString pos;
    if (pFindMethod->GetReverse())
    {
        pos += "startBefore" + pFindMethod->GetType()->GetName();
        pUsedFindMethod = GetFindReverseValueTreeMethod();
    }
    else
    {
        pos += "startAfter" + pFindMethod->GetType()->GetName();
        pUsedFindMethod = GetFindValueTreeMethod();
    }

    if (pUsedFindMethod)
    {
        if (pFindMethod->GetNext())
        {
            code += GetIndent() + "if (!" + pos + ")" NL;
            code += GetIndent(2) + pos + " = " + pUsedFindMethod->GetName() + "(" + 
                    pArgument->GetName() + ");" NL NL;
        }
        else
        {
            code += GetIndent() + pFindMethod->GetType()->GetName() + "* " + pos + " = " + 
                    pUsedFindMethod->GetName() + "(" + pArgument->GetName() + ");" NL NL;
        }

        Relation* pRelation = GetRelation();
        CbString variable = pRelation->GetToClass()->GetBaseName();
        code += GetIndent() + pRelation->GetToName() + "Iterator i" + variable;
        if (pRelation->GetStatic())
                code += "(0, " + pos + ");" NL;
        else
                code += "(this, 0, " + pos + ");" NL;

        if (pFindMethod->GetReverse())
            code += GetIndent() + "while (--i" + variable + ")" NL;
        else
            code += GetIndent() + "while (++i" + variable + ")" NL;
        code += GetIndent() + "{" NL;

        code += GetIndent(2) + "if (" + pArgument->GetName() + " == i" + variable + pArgument->GetPath() + ")" NL;
        code += GetIndent(2) + "{" NL;
        bool ifNeeded = false;
        Method::ArgumentIterator iArgument(pFindMethod);
        while (++iArgument)
        {
            if (iArgument.Get() != pArgument && !iArgument->GetPath().IsEmpty())
            {
                ifNeeded = true;
                break;
            }
        }
        if (ifNeeded)
        {
            code += GetIndent(3) + "if (";
            bool first = true;
            iArgument.Reset();
            while (++iArgument)
            {
                if (iArgument.Get() != pArgument && !iArgument->GetPath().IsEmpty())
                {
                    if (first)
                        first = false;
                    else
                        code += " &&" NL + GetIndent(4);

                    code += iArgument->GetName() + " == i" + variable + iArgument->GetPath();
                }
            }
            code += ")" NL;
            code += GetIndent(3) + "{" NL;
            code += GetIndent(4) + "return i" + variable + ";" NL;
            code += GetIndent(3) + "}" NL;
        }
        else
        {
            code += GetIndent(3) + "return i" + variable + ";" NL;
        }
        code += GetIndent(2) + "}" NL;
        code += GetIndent(2) + "else" NL;
        code += GetIndent(3) + "break; // Past correct keys" NL;
        code += GetIndent() + "}" NL NL;

        code += GetIndent() + "return 0;" NL;
    }

    return code;
}//@CODE_1791


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5600
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ValueTree::CleanupReferences()
{
    RelationMember::CleanupReferences();
}


/*@NOTE_1743
Method which must be called first in a constructor
*/
void ValueTree::ConstructorInclude()
{
    INIT_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
    INIT_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_1745
Method which must be called first in a destructor
*/
void ValueTree::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
    EXIT_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_5601
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ValueTree::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
    REMOVE_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
    RelationMember::RemoveReferences();
}


/*@NOTE_5602
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ValueTree::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5604
Save the state of the current object relations to pDataModelDocObject.
*/
void ValueTree::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1748
Serialize the members only to a CbObject object
*/
void ValueTree::Serialize(CbArchive& archive)
{
    RelationMember::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1747
Method which must be called first in a serialize constructor
*/
void ValueTree::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
    INIT_SINGLE_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
}


/*@NOTE_1750
Serialize the relations to a CbObject object
*/
void ValueTree::SerializeRelations(CbArchive& archive,
                                   DataModelDocObject* pointerArray[])
{
    RelationMember::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
        WRITE_SINGLE_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
            READ_SINGLE_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ValueTree)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)
METHODS_SINGLE_OWNED_ACTIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
