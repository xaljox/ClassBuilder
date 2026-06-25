/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ReplaceConstructor.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ReplaceConstructor'
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


/*@NOTE_386
Constructor needed for serialization, not meant to use for other purposes!
*/
ReplaceConstructor::ReplaceConstructor() //@INIT_386
    : Constructor()
{//@CODE_386
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_386


ReplaceConstructor::ReplaceConstructor(BaseClass* pBaseClass) //@INIT_976
    : Constructor(pBaseClass)
{//@CODE_976
    ConstructorInclude();

    // Put in your own code
    SetNote("Constructor needed for putting a new object in the old one's context.");

    Argument* argument = new Argument(this, GetBaseClass());
    argument->SetPointer(1);
    argument->SetName("pOld");

    if (pBaseClass->GetAdded())
        Add();

    (void)new ReplaceConstructorIncludeMethod(this);
}//@CODE_976


/*@NOTE_384
Destructor method
*/
ReplaceConstructor::~ReplaceConstructor()
{//@CODE_384
    DestructorInclude();

    // Put in your own code
}//@CODE_384


void ReplaceConstructor::InitCode()
{//@CODE_979
    _code.Empty();
    
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code += GetIndent() + "ReplaceConstructorInclude(pOld);" NL;
        
        Class::MemberIterator iMember(pClass, &Member::IsNonStatic);
        while (++iMember)
        {
            if (iMember.IsFirst())
                _code += NL;
            
            _code += GetIndent();
            _code += iMember->GetPrefixedName() + " = pOld->";
            _code += iMember->GetPrefixedName() + ";" NL;
            
            if (iMember->GetDelete())
            {
                _code += GetIndent();
                _code += "pOld->";
                _code += iMember->GetPrefixedName() + "= NULL;" NL;
            }
        }
        _code += NL + GetIndent() + "// Put in your own code" NL;
        }
}//@CODE_979


void ReplaceConstructor::InitInit()
{//@CODE_978
    _init.Empty();

    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        bool first = true;
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            if (first) 
            { 
                first = false; 
                _init += GetIndent() + ": "; 
            } 
            else  
                _init += GetIndent() + ", ";

            _init += inherit->GetBaseName() + "(pOld)" NL;
        }
    }
}//@CODE_978


void ReplaceConstructor::NotifyAddMember(Member* pMember)
{//@CODE_23016
}//@CODE_23016


void ReplaceConstructor::NotifyRemoveMember(Member* pMember)
{//@CODE_23018
}//@CODE_23018


int ReplaceConstructor::OnAddArgument(bool checkOnly)
{//@CODE_981
    return 0;
}//@CODE_981


int ReplaceConstructor::OnDelete(bool checkOnly)
{//@CODE_980
    return 0;
}//@CODE_980


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5516
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ReplaceConstructor::CleanupReferences()
{
    Constructor::CleanupReferences();
}


/*@NOTE_383
Method which must be called first in a constructor
*/
void ReplaceConstructor::ConstructorInclude()
{
    INIT_SINGLE_OWNED_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_385
Method which must be called first in a destructor
*/
void ReplaceConstructor::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_5517
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ReplaceConstructor::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
    Constructor::RemoveReferences();
}


/*@NOTE_5518
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ReplaceConstructor::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Constructor::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5520
Save the state of the current object relations to pDataModelDocObject.
*/
void ReplaceConstructor::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Constructor::SaveReferences(pDataModelDocObject);
}


/*@NOTE_388
Serialize the members only to a CbObject object
*/
void ReplaceConstructor::Serialize(CbArchive& archive)
{
    Constructor::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_387
Method which must be called first in a serialize constructor
*/
void ReplaceConstructor::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
}


/*@NOTE_390
Serialize the relations to a CbObject object
*/
void ReplaceConstructor::SerializeRelations(CbArchive& archive,
                                            DataModelDocObject* pointerArray[])
{
    Constructor::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ReplaceConstructor)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
