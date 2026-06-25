/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          AvlTree.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'AvlTree'
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


/*@NOTE_35428
Constructor needed for serialization, not meant to use for other purposes!
*/
AvlTree::AvlTree() //@INIT_35428
    : RelationMember()
{//@CODE_35428
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_35428


/*@NOTE_35447
Constructor method.
*/
AvlTree::AvlTree(Relation* pRelation, Member* pMember) //@INIT_35447
    : RelationMember(pRelation, pMember)
{//@CODE_35447
    ConstructorInclude();

    // Put in your own code

    FindAvlTreeMethod* pFindAvlTreeMethod = new FindAvlTreeMethod(this);
    FindReverseAvlTreeMethod* pFindReverseAvlTreeMethod = new FindReverseAvlTreeMethod(this);
    FindEqualOrSmallerAvlTreeMethod* pFindEqualOrSmallerAvlTreeMethod = new FindEqualOrSmallerAvlTreeMethod(this);
    FindEqualOrBiggerAvlTreeMethod* pFindEqualOrBiggerAvlTreeMethod = new FindEqualOrBiggerAvlTreeMethod(this);
    pFindAvlTreeMethod->GetBaseClass()->NotifyAddMethod(pFindAvlTreeMethod);
    pFindReverseAvlTreeMethod->GetBaseClass()->NotifyAddMethod(pFindReverseAvlTreeMethod);
    pFindEqualOrSmallerAvlTreeMethod->GetBaseClass()->NotifyAddMethod(pFindEqualOrSmallerAvlTreeMethod);
    pFindEqualOrBiggerAvlTreeMethod->GetBaseClass()->NotifyAddMethod(pFindEqualOrBiggerAvlTreeMethod);
    if (pRelation->GetFromRelation()->GetAdded())
    {
        pFindAvlTreeMethod->Add();
        pFindReverseAvlTreeMethod->Add();
        pFindEqualOrSmallerAvlTreeMethod->Add();
        pFindEqualOrBiggerAvlTreeMethod->Add();
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
                    iMethod.Get() != pFindAvlTreeMethod &&
                    iMethod.Get() != pFindReverseAvlTreeMethod &&
                    iMethod.Get() != pFindEqualOrSmallerAvlTreeMethod &&
                    iMethod.Get() != pFindEqualOrBiggerAvlTreeMethod)
                {
                    iMethod->Delete();
                }
                else
                    iMethod->InitCode();
                break;
            }
        }
    }
}//@CODE_35447


/*@NOTE_35425
Destructor method.
*/
AvlTree::~AvlTree()
{//@CODE_35425
    DestructorInclude();

    // Put in your own code
}//@CODE_35425


int AvlTree::GetImplementation()
{//@CODE_35443
    return 2;
}//@CODE_35443


CbString AvlTree::InitCodeFindMethod(FindMethod* pFindMethod,
                                     Argument* pArgument)
{//@CODE_35444
    CbString code;
    FindMethod* pUsedFindMethod;

    CbString pos;
    if (pFindMethod->GetReverse())
    {
        pos += "startBefore" + pFindMethod->GetType()->GetName();
        pUsedFindMethod = GetFindReverseAvlTreeMethod();
    }
    else
    {
        pos += "startAfter" + pFindMethod->GetType()->GetName();
        pUsedFindMethod = GetFindAvlTreeMethod();
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
}//@CODE_35444


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_35435
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void AvlTree::CleanupReferences()
{
    RelationMember::CleanupReferences();
}


/*@NOTE_35426
Method which must be called first in a constructor.
*/
void AvlTree::ConstructorInclude()
{
    INIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
    INIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
    INIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
    INIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35427
Method which must be called first in a destructor.
*/
void AvlTree::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
    EXIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
    EXIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
    EXIT_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35436
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void AvlTree::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
    REMOVE_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
    REMOVE_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
    REMOVE_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
    RelationMember::RemoveReferences();
}


/*@NOTE_35437
Bring the current object relations into the same state as pDataModelDocObject.
*/
void AvlTree::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_35439
Save the state of the current object relations to pDataModelDocObject.
*/
void AvlTree::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember::SaveReferences(pDataModelDocObject);
}


/*@NOTE_35430
Serialize the members only to a CbObject object.
*/
void AvlTree::Serialize(CbArchive& archive)
{
    RelationMember::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_35429
Method which must be called first in a serialize constructor.
*/
void AvlTree::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
    INIT_SINGLE_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
    INIT_SINGLE_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
    INIT_SINGLE_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
}


/*@NOTE_35432
Serialize the relations to a CbObject object.
*/
void AvlTree::SerializeRelations(CbArchive& archive,
                                 DataModelDocObject* pointerArray[])
{
    RelationMember::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
        WRITE_SINGLE_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
        WRITE_SINGLE_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
        WRITE_SINGLE_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
            READ_SINGLE_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
            READ_SINGLE_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
            READ_SINGLE_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(AvlTree)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
METHODS_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
METHODS_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
METHODS_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
