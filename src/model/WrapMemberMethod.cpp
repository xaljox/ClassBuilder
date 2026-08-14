/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          WrapMemberMethod.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'WrapMemberMethod'
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


/*@NOTE_1294
Constructor needed for serialization, not meant to use for other purposes!
*/
WrapMemberMethod::WrapMemberMethod() //@INIT_1294
    : MemberMethod()
{//@CODE_1294
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1294


WrapMemberMethod::WrapMemberMethod(Member* pMember,
                                   Method* pMethod) //@INIT_1306
    : MemberMethod(pMember, pMethod->GetType())
{//@CODE_1306
    ConstructorInclude(pMethod);

    // Put in your own code
    SetName(pMethod->GetName());
    SetArray(pMethod->GetArray());
    Variable::SetConst(pMethod->Variable::GetConst());
    SetPointer(pMethod->GetPointer());
    SetReference(pMethod->GetReference());
    SetArraySize(pMethod->GetArraySize());
    SetArraySizeStr(pMethod->GetArraySizeStr());
    SetAccess(pMethod->GetAccess());
    SetConst(pMethod->GetConst());

	// Get the templates and strip '<', '>' and white spaces.
	CbString typeTemplate = pMember->GetType()->GetTemplate();
	CbString memberTemplate = pMember->GetTemplate();

	if (typeTemplate.GetAt(0) == '<')
	{
		typeTemplate = typeTemplate.Mid(1);
	}
	if (typeTemplate.GetAt(typeTemplate.GetLength()-1) == '>')
	{
		typeTemplate = typeTemplate.Left(typeTemplate.GetLength()-1);
	}
	typeTemplate.TrimLeft();
	typeTemplate.TrimRight();

	if (memberTemplate.GetAt(0) == '<')
	{
		memberTemplate = memberTemplate.Mid(1);
	}
	if (memberTemplate.GetAt(memberTemplate.GetLength()-1) == '>')
	{
		memberTemplate = memberTemplate.Left(memberTemplate.GetLength()-1);
	}
	memberTemplate.TrimLeft();
	memberTemplate.TrimRight();

    ArgumentIterator argument(pMethod);
    while (++argument)
	{
        Argument* pArgument = new Argument(this, argument);

		// See if another type is needed because of template
		Type* pType = 0;

		// The current type of argument can be found in the template of the type
		// of the member, but not in the template of the member itself
		if (typeTemplate.Find(pArgument->GetType()->GetName()) != -1 &&
			memberTemplate.Find(pArgument->GetType()->GetName()) == -1)
		{
			CbString typeTemplateTmp = typeTemplate;
			CbString memberTemplateTmp = memberTemplate;

			// Search corresponding type in the template of the member.
			int typeTemplateIndex = typeTemplateTmp.Find(',');
			int memberTemplateIndex = memberTemplateTmp.Find(',');
			while (!pType && typeTemplateIndex != -1 && memberTemplateIndex != -1)
			{
				CbString typeTemplateItem = typeTemplateTmp.Left(typeTemplateIndex);
				CbString memberTemplateItem = memberTemplateTmp.Left(memberTemplateIndex);

				typeTemplateTmp = typeTemplateTmp.Mid(typeTemplateIndex+1);
				memberTemplateTmp = memberTemplateTmp.Mid(memberTemplateIndex+1);

				typeTemplateItem.TrimRight();
				typeTemplateTmp.TrimLeft();
				memberTemplateItem.TrimRight();
				memberTemplateTmp.TrimLeft();

				if (pArgument->GetType()->GetName() == typeTemplateItem)
				{
					pType = GetDataModelDoc()->FindType(memberTemplateItem);
				}

				typeTemplateIndex = typeTemplateTmp.Find(',');
				memberTemplateIndex = memberTemplateTmp.Find(',');
			}

			if (!pType && pArgument->GetType()->GetName() == typeTemplateTmp)
			{
				pType = GetDataModelDoc()->FindType(memberTemplateTmp);
			}
		}

		// Another type is found because of template
		if (pType)
		{
			pType->MoveVariableLast(pArgument);
		}
	}
}//@CODE_1306


/*@NOTE_1292
Destructor method
*/
WrapMemberMethod::~WrapMemberMethod()
{//@CODE_1292
    DestructorInclude();

    // Put in your own code
}//@CODE_1292


void WrapMemberMethod::InitCode()
{//@CODE_1309
    _code = GetIndent();
    if (GetMethod()->GetType()->GetName() != "void" || GetMethod()->GetPointer())
        _code += "return ";
    
    _code += GetMember()->GetPrefixedName();

    if (GetMember()->GetPointer())
        _code += "->";
    else
        _code += ".";

    _code += GetMethod()->GetName() + "(";
    Method::ArgumentIterator argument(this);
    while (++argument)
    {
        _code += argument->GetName();
        if (!argument.IsLast())
            _code += ", ";
    }

    _code += ");" NL;

    SetPhaseUpwards(Complete_Phase);
}//@CODE_1309


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5576
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void WrapMemberMethod::CleanupReferences()
{
    MemberMethod::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
}


/*@NOTE_1291
Method which must be called first in a constructor
*/
void WrapMemberMethod::ConstructorInclude(Method* pMethod)
{
    INIT_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
}


/*@NOTE_1293
Method which must be called first in a destructor
*/
void WrapMemberMethod::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
}


/*@NOTE_5577
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void WrapMemberMethod::RemoveReferences()
{
    MemberMethod::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
}


/*@NOTE_5578
Bring the current object relations into the same state as pDataModelDocObject.
*/
void WrapMemberMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    WrapMemberMethod* pWrapMemberMethod = (WrapMemberMethod*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    MemberMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5580
Save the state of the current object relations to pDataModelDocObject.
*/
void WrapMemberMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberMethod::SaveReferences(pDataModelDocObject);
    WrapMemberMethod* pWrapMemberMethod = (WrapMemberMethod*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
}


/*@NOTE_1296
Serialize the members only to a CbObject object
*/
void WrapMemberMethod::Serialize(CbArchive& archive)
{
    MemberMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1295
Method which must be called first in a serialize constructor
*/
void WrapMemberMethod::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
}


/*@NOTE_1298
Serialize the relations to a CbObject object
*/
void WrapMemberMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(WrapMemberMethod)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
