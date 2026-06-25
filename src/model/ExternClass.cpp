/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ExternClass.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ExternClass'
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
#include "qt/QtExternClassDialog.h"
#include "qt/QtVirtualMethodsDialog.h"
//@END_USER2


// Static members


ExternClass::ExternClass(DataModelDoc* pDataModelDoc) //@INIT_718
    : BaseClass(pDataModelDoc)
{//@CODE_718
    ConstructorInclude();

    // Put in your own code
    (void)new Destructor(this);
}//@CODE_718


/*@NOTE_3238
Constructor needed for putting a new object in the old one's context
*/
ExternClass::ExternClass(Class* pOld) //@INIT_3238
    : BaseClass(pOld)
{//@CODE_3238
    ReplaceConstructorInclude(pOld);

    SetPhase(None_Phase);

    // Deletes all private methods, FromRelationMethods and MacroMethods.
    // Transforms MemberMethods objects in Methods if needed
    BaseClass::MethodIterator iMethod(this);
    while (++iMethod)
    {
        iMethod->SetPhase(None_Phase);

        // Make method implementation empty
        iMethod->SetCode("");
        iMethod->SetDeclare(false);
        iMethod->SetImplement(false);
        if (iMethod->IsConstructor())
        {
            Constructor* pConstructor = (Constructor*)iMethod.Get();
            pConstructor->SetInit("");
        }

        if (iMethod->GetAccess() == PRIVATE || 
            iMethod->IsFromRelationMethod() || iMethod->IsMacroMethod())
        {
            iMethod->Delete();
        }
        else if (iMethod->IsMemberMethod())
        {
            MemberMethod* pMemberMethod = (MemberMethod*)iMethod.Get();

            if (pMemberMethod->GetMember()->GetAccess() == PRIVATE)
            {
                // Make copy of pointer to Method object, since the Method replace
                // constructor, also updates the Iterator!!
                (void)new Method(iMethod);
                delete pMemberMethod;
            }
        }
    }

    // Delete all private members
    Class::MemberIterator iMember(this);
    while (++iMember)
    {
        iMember->SetPhase(None_Phase);

        if (iMember->GetAccess() == PRIVATE)
        {
            delete iMember;
        }
    }
    
    // Delete empty groups
    MemberAndMethodGroupIterator iMemberAndMethodGroup(this);
    while (++iMemberAndMethodGroup)
    {
        iMemberAndMethodGroup->SetPhase(None_Phase);

        if (iMemberAndMethodGroup->GetMemberCount() == 0 &&
            iMemberAndMethodGroup->GetMethodCount() == 0)
        {
            delete iMemberAndMethodGroup;
        }
    }

    // We are converted now, so delete the old thing.
    delete pOld;
}//@CODE_3238


ExternClass::ExternClass(OtherType* pOld) //@INIT_7503
    : BaseClass(pOld)
{//@CODE_7503
    ConstructorInclude();

    // Put in your own code

    // We are converted now, so delete the old thing.
    delete pOld;
}//@CODE_7503


/*@NOTE_7547
Constructor method needed to copy ExternClass from one project to the other.
*/
ExternClass::ExternClass(DataModelDoc* pDataModelDoc,
                         ExternClass* pExternClass) //@INIT_7547
    : BaseClass(pDataModelDoc, pExternClass)
{//@CODE_7547
    ConstructorInclude();

    // Put in your own code
    InheritIterator iInherit(pExternClass);
    while (++iInherit)
    {
        (void)new Inherit(this, GetDataModelDoc()->FindOrDuplicateExternClass(iInherit->GetBaseClass()), iInherit);
    }
}//@CODE_7547


/*@NOTE_113
Constructor needed for serialization, not meant to use for other purposes!
*/
ExternClass::ExternClass() //@INIT_113
    : BaseClass()
{//@CODE_113
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_113


/*@NOTE_111
Destructor method
*/
ExternClass::~ExternClass()
{//@CODE_111
    DestructorInclude();

    // Put in your own code
}//@CODE_111


void ExternClass::Add()
{//@CODE_3285
    if (!GetAdded())
    {
        SaveState(1);
        GetDataModelDoc()->GetExternClasses()->AddChildLast(this);

        SetIcon(ICON_EXTERNCLASS);
        SetItemText(GetName());

        Gti::Add();

        InheritIterator inherit(this);
        while (++inherit)
            inherit->Add();

        MemberAndMethodGroupIterator memberAndMethodGroup(this);
        while (++memberAndMethodGroup)
            memberAndMethodGroup->Add();

        MemberIterator member(this);
        while (++member)
            member->Add();

        Class::MethodIterator method(this, &Method::IsDirectMethod);
        while (++method)
            method->Add();
    }
}//@CODE_3285


int ExternClass::IsBaseClass(BaseClass* pBaseClass)
{//@CODE_3288
    InheritIterator inherit(this);
    while (++inherit)
    {
        if (inherit->GetBaseClass() == pBaseClass)
            return 1;

        ExternClass* pExternClass = dynamic_cast<ExternClass*>(inherit->GetBaseClass());
        if (pExternClass && pExternClass->IsBaseClass(pBaseClass))
            return 1;
    }

    return 0;
}//@CODE_3288


int ExternClass::OnAddInherit(bool checkOnly)
{//@CODE_3236
    if (!checkOnly)
    {
        Class* pClass = dynamic_cast<Class*>(this);

        if (pClass && pClass->GetSerialize() && pClass->GetFirstInherit())
            GetFirstInherit()->OnEditAttributes();
        else
        {
            GetDataModelDoc()->MarkLastUndo();
            Inherit* pInherit = new Inherit(this, this);

            if (pInherit->OnEditAttributes())
                pInherit->Add();
            else
                GetDataModelDoc()->RollBack();
        }
    }

    return 1;
}//@CODE_3236


int ExternClass::OnAddVirtuals(bool checkOnly)
{//@CODE_3336
    if (!checkOnly)
    {
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowVirtualMethodsDialog(this, nullptr, ownerHwnd);
    }

    return 1;
}//@CODE_3336


int ExternClass::OnDelete(bool checkOnly)
{//@CODE_721
    if (this->BaseClass::GetInheritCount())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete an inherited class", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete extern class '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }
    
    return 1;
}//@CODE_721


int ExternClass::OnEditAttributes(bool checkOnly)
{//@CODE_720
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;

    if (Qt_ShowExternClassDialog(this, changed, ownerHwnd))
    {
        if (changed)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_720


int ExternClass::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_7598
    int result = 0;
    
    Method* pMethod = dynamic_cast<Method*>(pGti);
    if (pMethod && (IsClass() || !pMethod->IsPrivateMethod()))
    {
		Method* pNewMethod = 0;
		Constructor* pConstructor = dynamic_cast<Constructor*>(pMethod);
		Destructor* pDestructor = dynamic_cast<Destructor*>(pMethod);
		if(pConstructor && !IsClass() && 
			GetName() == pMethod->GetBaseClass()->GetName())
		{
			result = 1;
			if (!checkOnly)
			{
                pNewMethod = new Constructor(this, pConstructor);
			}
		}
		else if (pDestructor && !IsClass() && 
			         GetName() == pMethod->GetBaseClass()->GetName())
		{
			result = 1;
			if (!checkOnly)
			{
                pNewMethod = new Destructor(this, pDestructor);
			}
		}
		else
		{
			result = 1;
			if (!checkOnly)
			{
                pNewMethod = new Method(this, GetDataModelDoc()->FindOrDuplicateType(pMethod->GetType()), pMethod);
			}
		}
		
		if (pNewMethod)
		{
			if (!IsClass())
			{
				pNewMethod->SetCode("");
			}
			pNewMethod->Add();
            NotifyAddMethod(pNewMethod);
		}
	}
	
	Member* pMember = dynamic_cast<Member*>(pGti);
	if (pMember && (IsClass() || !pMember->IsPrivate()))
	{
		if (!checkOnly)
		{
			Member* pNewMember = new Member(this, GetDataModelDoc()->FindOrDuplicateType(pMember->GetType()), pMember);
			pNewMember->Add();
            NotifyAddMember(pNewMember);
		}
		result = 1;
	}
	
	if (result)
	{
		if (!checkOnly)
		{
			DataModelDoc::TypeIterator iType(GetDataModelDoc());
			while (++iType)
			{
				if (!iType->GetAdded())
				{
					iType->Add();
				}
			}
		}
	}
	else
	{
		result = Gti::OnPaste(pGti, checkOnly);
	}
	
	return result;
}//@CODE_7598


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5402
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ExternClass::CleanupReferences()
{
    BaseClass::CleanupReferences();
}


/*@NOTE_110
Method which must be called first in a constructor
*/
void ExternClass::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_112
Method which must be called first in a destructor
*/
void ExternClass::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_5403
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ExternClass::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
    BaseClass::RemoveReferences();
}


/*@NOTE_3240
Method which must be called first in a replace constructor
*/
void ExternClass::ReplaceConstructorInclude(ExternClass* pOld)
{
    REPLACE_MULTI_OWNED_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_5404
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ExternClass::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    BaseClass::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5406
Save the state of the current object relations to pDataModelDocObject.
*/
void ExternClass::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    BaseClass::SaveReferences(pDataModelDocObject);
}


/*@NOTE_115
Serialize the members only to a CbObject object
*/
void ExternClass::Serialize(CbArchive& archive)
{
    BaseClass::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_114
Method which must be called first in a serialize constructor
*/
void ExternClass::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_117
Serialize the relations to a CbObject object
*/
void ExternClass::SerializeRelations(CbArchive& archive,
                                     DataModelDocObject* pointerArray[])
{
    BaseClass::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ExternClass)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)
METHODS_ITERATOR_MULTI_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
