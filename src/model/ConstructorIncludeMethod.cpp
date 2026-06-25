/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ConstructorIncludeMethod.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ConstructorIncludeMethod'
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


/*@NOTE_425
Constructor needed for serialization, not meant to use for other purposes!
*/
ConstructorIncludeMethod::ConstructorIncludeMethod() //@INIT_425
    : FixedMethod()
{//@CODE_425
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_425


ConstructorIncludeMethod::ConstructorIncludeMethod(Class* pClass) //@INIT_993
    : FixedMethod(pClass, pClass->GetDataModelDoc()->FindType("void"))
{//@CODE_993
    ConstructorInclude(pClass);

    // Put in your own code
    SetAccess(PRIVATE);
    SetName("ConstructorInclude");
    SetNote("Method which must be called first in a constructor.");
}//@CODE_993


/*@NOTE_423
Destructor method
*/
ConstructorIncludeMethod::~ConstructorIncludeMethod()
{//@CODE_423
    DestructorInclude();

    // Put in your own code
}//@CODE_423


void ConstructorIncludeMethod::InitCode()
{//@CODE_996
    _code.Empty();

    Class::FromRelationIterator fromRelation((Class*)GetBaseClass());
    while (++fromRelation)
        fromRelation->WriteFromMacro(_code, GetIndent() + "INIT_");

    Class::ToRelationIterator toRel((Class*)GetBaseClass());
    while (++toRel)
    {
        if (!(toRel->GetSingle() && 
              toRel->GetFromClass() == toRel->GetToClass() &&
              toRel->GetFromName() == toRel->GetToName()))
        {
            toRel->WriteToMacro(_code, GetIndent() + "INIT_");
        }
    }
}//@CODE_996


void ConstructorIncludeMethod::UpdateArguments()
{//@CODE_995
    SaveState();

	CbString oldString = "ConstructorInclude(";
	CbString newString = "ConstructorInclude(";
	bool first = true;

    ArgumentIterator iArgument(this);
    while (++iArgument)
	{
        if (!first)
            oldString += ", ";
        else
            first = false;

        oldString += iArgument->GetName();

        iArgument->Delete();
	}

	first = true;
    Class::ToRelationIterator iRelation(GetClass());
    while (++iRelation)
    {
        if (iRelation->GetOwned() && !iRelation->GetStatic() && 
            iRelation->GetFromClass()->GetDataModelDoc()) // Class -> ExternClass
        {
            Argument* pArgument = new Argument(this, iRelation->GetFromClass());
            pArgument->SetPointer(1);
            pArgument->SetName("p" + iRelation->GetFromName());
            pArgument->Add();

			if (!first)
				newString += ", ";
			else
				first = false;

			newString += pArgument->GetName();
        }
    }
	oldString += ")";
	newString += ")";

	if (oldString != newString)
	{
		// Adjust calls to ConstructorInclude in code
		BaseClass::MethodIterator iMethod(GetClass(), &Method::IsNormalConstructor);
		while (++iMethod)
		{
			iMethod->ReplaceInCode(oldString, newString);
		}

		// Find out the exact difference
		int fIndex = 0;
		int bIndex = 1;
		while (oldString.GetAt(fIndex) == newString.GetAt(fIndex))
		{
			fIndex++;
		}
		while (__iscsym(oldString.GetAt(fIndex)) || 
			   __iscsym(newString.GetAt(fIndex)))
		{
			fIndex--;
		}
		while (oldString.GetAt(oldString.GetLength()-bIndex) == 
			   newString.GetAt(newString.GetLength()-bIndex))
		{
			bIndex++;
		}
		while (__iscsym(oldString.GetAt(oldString.GetLength()-bIndex)) || 
			   __iscsym(newString.GetAt(newString.GetLength()-bIndex)))
		{
			bIndex--;
		}

		oldString = oldString.Mid(fIndex, oldString.GetLength()-fIndex-bIndex+1);
		oldString.Remove('(');
		oldString.Remove(')');
		oldString.Remove(',');
		oldString.Remove(' ');

		newString = newString.Mid(fIndex, newString.GetLength()-fIndex-bIndex+1);
		newString.Remove('(');
		newString.Remove(')');
		newString.Remove(',');
		newString.Remove(' ');

		if (!oldString.IsEmpty() && newString.IsEmpty())
		{
			// An argument has been removed, try to find argument on the 
			// constructor and delete it is found
			iMethod.Reset();
			while (++iMethod)
			{
				Argument* pArgument = iMethod->FindArgument(oldString);
				if (pArgument)
				{
					pArgument->Delete();
				}
			}
		}
		else if (oldString.IsEmpty() && !newString.IsEmpty())
		{
			// An argument has been added, find it on constructor include method
			// and create similar argument on constructor
			Argument* pArgument = FindArgument(newString);
			if (pArgument)
			{
				iMethod.Reset();
				while (++iMethod)
				{
					Argument* pNewArgument = new Argument(iMethod, pArgument);
					pNewArgument->Add();
				}
			}
		}
		else if (!oldString.IsEmpty() && !newString.IsEmpty())
		{
			// A rename of argument, try to find the same argument on constructor
			// and change name if found.
			iMethod.Reset();
			while (++iMethod)
			{
				Argument* pArgument = iMethod->FindArgument(oldString);
				if (pArgument)
				{
					pArgument->SaveState();
					pArgument->SetName(newString);
					pArgument->Update();
				}
			}
		}
	}

    Update();
}//@CODE_995


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5300
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ConstructorIncludeMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
}


/*@NOTE_422
Method which must be called first in a constructor
*/
void ConstructorIncludeMethod::ConstructorInclude(Class* pClass)
{
    INIT_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
}


/*@NOTE_424
Method which must be called first in a destructor
*/
void ConstructorIncludeMethod::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
}


/*@NOTE_5301
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ConstructorIncludeMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
}


/*@NOTE_5302
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ConstructorIncludeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConstructorIncludeMethod* pConstructorIncludeMethod = (ConstructorIncludeMethod*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5304
Save the state of the current object relations to pDataModelDocObject.
*/
void ConstructorIncludeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
    ConstructorIncludeMethod* pConstructorIncludeMethod = (ConstructorIncludeMethod*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
}


/*@NOTE_427
Serialize the members only to a CbObject object
*/
void ConstructorIncludeMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_426
Method which must be called first in a serialize constructor
*/
void ConstructorIncludeMethod::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
}


/*@NOTE_429
Serialize the relations to a CbObject object
*/
void ConstructorIncludeMethod::SerializeRelations(CbArchive& archive,
                                                  DataModelDocObject* pointerArray[])
{
    FixedMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ConstructorIncludeMethod)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
