/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SetMemberMethod.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SetMemberMethod'
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


/*@NOTE_269
Constructor needed for serialization, not meant to use for other purposes!
*/
SetMemberMethod::SetMemberMethod() //@INIT_269
    : MemberMethod()
{//@CODE_269
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_269


SetMemberMethod::SetMemberMethod(Member* pMember) //@INIT_907
    : MemberMethod(pMember, pMember->GetDataModelDoc()->FindType("void"))
{//@CODE_907
    ConstructorInclude(pMember);

    // Put in your own code
    SetInline(pMember->GetRelationMemberCount() ? 0: 1);

    Argument* argument = new MemberArgument(this, pMember);
    //argument->SetPointer(pMember->GetPointer());
    if (pMember->GetType()->IsBaseClass())
    {
        if (!pMember->GetPointer())
        {
            argument->SetReference(1);
            argument->SetConst(1);
        }
    }

    CbString name = pMember->GetName();
    if (pMember->GetPointer() && name[0] == 'p')
    {
        name = name.Mid(1);
    } 
    if (pMember->GetPointerPointer() && name[0] == 'p')
    {
        name = name.Mid(1);
    } 
    if (pMember->GetReference() && name[0] == 'r')
    {
        name = name.Mid(1);
    }
    if (islower(name[0]))
    {
        name = pMember->GetName();
    }
    name.SetAt(0, toupper(name[0]));

    CbString argName;
    if (argument->GetPointer())
        argName += "p";
    if (argument->GetPointerPointer())
        argName += "p";
    if (argument->GetReference())
        argName += "r";
    if (argument->GetPointer() || argument->GetReference())
        argName += name;
    else
    {
        name.SetAt(0, tolower(name[0]));
        argName += name;
        name.SetAt(0, toupper(name[0]));
    }

    argument->SetName(argName);
    argument->SetTemplate(pMember->GetTemplate());

    SetName("Set" + name);

    CbString note;
    note.Format("Set the value of member '%s' to '%s'.", 
        pMember->GetPrefixedName(), argName);
    SetNote(note);
}//@CODE_907


/*@NOTE_267
Destructor method
*/
SetMemberMethod::~SetMemberMethod()
{//@CODE_267
    DestructorInclude();

    // Put in your own code
}//@CODE_267


void SetMemberMethod::InitCode()
{//@CODE_909
    if (GetArgumentCount())
    {
        _code.Empty();
        if (GetMember()->GetRelationMemberCount())
        {
            _code += GetIndent() + "if (" + GetMember()->GetPrefixedName() + " != " + 
                GetFirstArgument()->GetName() + ")" NL;
            _code += GetIndent() + "{" NL;
            Member::RelationMemberIterator iRelationMember(GetMember());
            while (++iRelationMember)
            {
                _code += iRelationMember->PrologueSetMemberMethod();
            }
            _code += NL + GetIndent(2) + GetMember()->GetPrefixedName() + " = " + 
                GetFirstArgument()->GetName() + ";" NL NL;
            iRelationMember.Reset();
            while (++iRelationMember)
            {
                _code += iRelationMember->EpilogSetMemberMethod();
            }
            _code += GetIndent() + "}" NL;
        }
        else // Normal case
        {
            if (GetMember()->GetDelete())
            {
                _code += GetIndent() + "delete " + GetMember()->GetPrefixedName() + ";" NL;
            }

            _code += GetIndent() + GetMember()->GetPrefixedName() + " = " + 
                GetFirstArgument()->GetName() + ";" NL;
        }
    }
}//@CODE_909


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5552
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SetMemberMethod::CleanupReferences()
{
    MemberMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
}


/*@NOTE_266
Method which must be called first in a constructor
*/
void SetMemberMethod::ConstructorInclude(Member* pMember)
{
    INIT_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
}


/*@NOTE_268
Method which must be called first in a destructor
*/
void SetMemberMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
}


/*@NOTE_5553
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SetMemberMethod::RemoveReferences()
{
    MemberMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
}


/*@NOTE_5554
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SetMemberMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SetMemberMethod* pSetMemberMethod = (SetMemberMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
    MemberMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5556
Save the state of the current object relations to pDataModelDocObject.
*/
void SetMemberMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberMethod::SaveReferences(pDataModelDocObject);
    SetMemberMethod* pSetMemberMethod = (SetMemberMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
}


/*@NOTE_271
Serialize the members only to a CbObject object
*/
void SetMemberMethod::Serialize(CbArchive& archive)
{
    MemberMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_270
Method which must be called first in a serialize constructor
*/
void SetMemberMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)
}


/*@NOTE_273
Serialize the relations to a CbObject object
*/
void SetMemberMethod::SerializeRelations(CbArchive& archive,
                                         DataModelDocObject* pointerArray[])
{
    MemberMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(SetMemberMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(Member, Member, SetMemberMethod, SetMemberMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
