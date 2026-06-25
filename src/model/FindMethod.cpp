/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindMethod.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FindMethod'
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
#include "qt/QtFindMethodDialog.h"
//@END_USER2


// Static members


/*@NOTE_295
Constructor needed for serialization, not meant to use for other purposes!
*/
FindMethod::FindMethod() //@INIT_295
    : FromRelationMethod()
    , _next(false)
    , _reverse(false)
{//@CODE_295
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_295


FindMethod::FindMethod(FromRelation* pFromRelation, bool reverse) //@INIT_914
    : FromRelationMethod(pFromRelation, pFromRelation->GetRelation()->GetToClass())
    , _next(false)
    , _reverse(reverse)
{//@CODE_914
    ConstructorInclude();

    // Put in your own code
    SetAccess(PUBLIC);
    SetPointer(1);
    if (reverse)
        SetName("FindReverse" + pFromRelation->GetRelation()->GetToName());
    else
        SetName("Find" + pFromRelation->GetRelation()->GetToName());
    SetPhase(Complete_Phase);
}//@CODE_914


/*@NOTE_293
Destructor method
*/
FindMethod::~FindMethod()
{//@CODE_293
    DestructorInclude();

    // Put in your own code
}//@CODE_293


void FindMethod::InitCode()
{//@CODE_916
    _code.Empty();

    if (GetArgumentCount())
    {
        Argument* pArgument = 0;
        Relation* pRelation = GetFromRelation()->GetRelation();
        if (pRelation->GetRelationMember())
        {
            Member* pMember = pRelation->GetRelationMember()->GetMember();
            Method::ArgumentIterator iArgument(this, &Argument::IsMemberArgument);
            while (++iArgument)
            {
                MemberArgument* pMemberArgument = (MemberArgument*)iArgument.Get();
                if (pMemberArgument->GetMember() == pMember)
                {
                    pArgument = iArgument;
                    break;
                }
            }
        }

        if (pArgument)
            _code = pRelation->GetRelationMember()->InitCodeFindMethod(this, pArgument);
        else
        {
            CbString pos;
            if (GetReverse())
                pos += "startBefore" + GetType()->GetName();
            else
                pos += "startAfter" + GetType()->GetName();

            CbString variable = pRelation->GetToClass()->GetBaseName();
            _code += GetIndent() + pRelation->GetToName() + "Iterator i" + variable;

            // Filtered iterators have a method-pointer parameter before the
            // ref (start position); NOFILTER iterators don't.
            CbString methodPlaceholder;
            if (pRelation->GetFilter())
                methodPlaceholder = "0, ";

            if (pRelation->GetStatic())
            {
                if (GetNext())
                    _code += "(" + methodPlaceholder + pos + ");" NL;
                else
                    _code += ";" NL;
            }
            else
            {
                if (GetNext())
                    _code += "(this, " + methodPlaceholder + pos + ");" NL;
                else
                    _code += "(this);" NL;
            }

            if (GetReverse())
                _code += GetIndent() + "while (--i";
            else
                _code += GetIndent() + "while (++i";
            _code += variable + ")" NL;
            _code += GetIndent() + "{" NL;

            _code += GetIndent(2) + "if (";
            bool first = true;
            Method::ArgumentIterator iArgument(this);
            while (++iArgument)
            {
                if (!iArgument->GetPath().IsEmpty())
                {
                    if (first)
                        first = false;
                    else
                        _code += " &&" NL + GetIndent(3);

                    _code += iArgument->GetName() + " == i" + variable + iArgument->GetPath();
                }
            }
            _code += ")" NL;
            _code += GetIndent(2) + "{" NL;
            _code += GetIndent(3) + "return i" + variable + ";" NL;
            _code += GetIndent(2) + "}" NL;
            _code += GetIndent() + "}" NL NL;

            _code += GetIndent() + "return 0;" NL;
        }
    }
    else
        _code += GetIndent() + "return 0;" NL;

    SetPhaseUpwards(Complete_Phase);
}//@CODE_916


int FindMethod::OnEditAttributes(bool checkOnly)
{//@CODE_917
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool fieldsChanged = false;

    if (Qt_ShowFindMethodDialog(this, ownerHwnd, fieldsChanged))
    {
        if (fieldsChanged)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_917


/*@NOTE_1543
Returns the value of member '_next'.
*/
bool FindMethod::GetNext()
{//@CODE_1543
    return _next;
}//@CODE_1543


/*@NOTE_1544
Set the value of member '_next' to 'next'.
*/
void FindMethod::SetNext(bool next)
{//@CODE_1544
    _next = next;
}//@CODE_1544


/*@NOTE_1783
Returns the value of member '_reverse'.
*/
bool FindMethod::GetReverse()
{//@CODE_1783
    return _reverse;
}//@CODE_1783


/*@NOTE_1784
Set the value of member '_reverse' to 'reverse'.
*/
void FindMethod::SetReverse(bool reverse)
{//@CODE_1784
    _reverse = reverse;
}//@CODE_1784


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5414
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FindMethod::CleanupReferences()
{
    FromRelationMethod::CleanupReferences();
}


/*@NOTE_292
Method which must be called first in a constructor
*/
void FindMethod::ConstructorInclude()
{
}


/*@NOTE_294
Method which must be called first in a destructor
*/
void FindMethod::DestructorInclude()
{
}


/*@NOTE_5415
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FindMethod::RemoveReferences()
{
    FromRelationMethod::RemoveReferences();
}


/*@NOTE_5416
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FindMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5418
Save the state of the current object relations to pDataModelDocObject.
*/
void FindMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelationMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_297
Serialize the members only to a CbObject object
*/
void FindMethod::Serialize(CbArchive& archive)
{
    FromRelationMethod::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _next;
        archive << _reverse;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _next;
            archive >> _reverse;
        }
    }
}


/*@NOTE_296
Method which must be called first in a serialize constructor
*/
void FindMethod::SerializeConstructorInclude()
{
}


/*@NOTE_299
Serialize the relations to a CbObject object
*/
void FindMethod::SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[])
{
    FromRelationMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(FindMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
