/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberAndMethodGroup.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MemberAndMethodGroup'
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
#include "ClassBuilderDoc.h"
#include "qt/QtContextDialog.h"
#include "qt/QtIsClassMethodsDialog.h"
#include "qt/QtVirtualMethodsDialog.h"
//@END_USER2


// Static members


MemberAndMethodGroup::MemberAndMethodGroup(BaseClass* pBaseClass) //@INIT_1028
    : Group(pBaseClass->GetDataModelDoc())
{//@CODE_1028
    ConstructorInclude(pBaseClass);

    // Put in your own code
}//@CODE_1028


/*@NOTE_542
Constructor needed for serialization, not meant to use for other purposes!
*/
MemberAndMethodGroup::MemberAndMethodGroup() //@INIT_542
    : Group()
{//@CODE_542
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_542


/*@NOTE_540
Destructor method
*/
MemberAndMethodGroup::~MemberAndMethodGroup()
{//@CODE_540
    DestructorInclude();

    // Put in your own code
}//@CODE_540


void MemberAndMethodGroup::Add()
{//@CODE_1031
    if (!GetAdded())
    {
        SaveState(1);
        GetBaseClass()->AddChildLast(this);
        SetItemText(GetName());
        SetIcon();

        Gti::Add();
    }
}//@CODE_1031


int MemberAndMethodGroup::CompareName(MemberAndMethodGroup* pA,
                                      MemberAndMethodGroup* pB)
{//@CODE_1823
    int result = pA->GetName().CompareNoCase(pB->GetName());

    // House sort-comparator discipline (cf. Class::CompareName,
    // LifeLineShape::CompareOrderWeight): record undo ONLY on a swap. The bubble
    // SortMemberAndMethodGroup flips the adjacent pair exactly when result > 0, so
    // snapshot the right operand and close a level-2 sub-batch per swap -- the
    // sequence of single-pair sub-batches is what RESTORE_MULTI_PASSIVE can
    // reverse. (Previously an un-gated SaveState(1) on EVERY comparison: O(n^2)
    // undo entries and no sub-batch markers.) Do not pair this with a merge sort.
    if (result > 0)
    {
        pB->SaveState();
        pB->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_1823


int MemberAndMethodGroup::ComparePhase(MemberAndMethodGroup* pA,
                                       MemberAndMethodGroup* pB)
{//@CODE_23481
    int result = pA->GetPhase() - pB->GetPhase();

    // Same per-swap undo discipline as CompareName above.
    if (result > 0)
    {
        pB->SaveState();
        pB->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_23481


Context* MemberAndMethodGroup::CreateContext(ContextDeclaration* pContextDeclaration)
{//@CODE_25527
    return new MemberAndMethodGroupContext(this, pContextDeclaration);
}//@CODE_25527


bool MemberAndMethodGroup::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1488
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        BaseClass* pBaseClass = GetBaseClass();
        if (pBaseClass->GetPrevMemberAndMethodGroup(this))
            pGtiDropDefault = pBaseClass->GetPrevMemberAndMethodGroup(this);
        else
            pGtiDropDefault = pBaseClass;

        Remove();
        value = true;
    }

    return value;
}//@CODE_1488


void MemberAndMethodGroup::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1491
    if (ctrlKeyDown)
    {
    }
    else
    {
        BaseClass* pClass = GetBaseClass();
        BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
        MemberAndMethodGroup* pDropMemberAndMethodGroup = dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);
        if (pDropClass)
            pClass->MoveMemberAndMethodGroupFirst(this);
        else if (pDropMemberAndMethodGroup)
            pClass->MoveMemberAndMethodGroupAfter(this, pDropMemberAndMethodGroup);

        int i = 0;
        Class::MemberAndMethodGroupIterator memberAndMethodGroup(pClass);
        while (++memberAndMethodGroup)
        {
            memberAndMethodGroup->SaveState(1);
            memberAndMethodGroup->SetOrder(i++);
        }
        Add();
        MemberIterator member(this);
        while (++member)
            member->Add();
        MethodIterator method(this);
        while (++method)
            method->Add();
    }
}//@CODE_1491


bool MemberAndMethodGroup::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1494
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
    }
    else
    {
        BaseClass* pClass = GetBaseClass();
        BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
        MemberAndMethodGroup* pDropMemberAndMethodGroup = dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);
        if ((pDropClass && pDropClass == pClass) ||
            (pDropMemberAndMethodGroup && pDropMemberAndMethodGroup->GetBaseClass() == pClass))
        {
            value = true;
        }
    }

    return value;
}//@CODE_1494


Context* MemberAndMethodGroup::GetFirstContext()
{//@CODE_26269
    return GetFirstMemberAndMethodGroupContext();
}//@CODE_26269


Context* MemberAndMethodGroup::GetNextContext(Context* pContextPos)
{//@CODE_26270
    return GetNextMemberAndMethodGroupContext((MemberAndMethodGroupContext*)pContextPos);
}//@CODE_26270


int MemberAndMethodGroup::OnAddConstructor(bool checkOnly)
{//@CODE_1036
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Constructor* pConstructor = new Constructor(GetBaseClass());
        pConstructor->CreateArguments();
        AddMethodLast(pConstructor);

        if (pConstructor->OnEditAttributes())
        {
            pConstructor->Add();
        }
        else
		{
            GetDataModelDoc()->RollBack();
		}
    }
    
    return 1;
}//@CODE_1036


int MemberAndMethodGroup::OnAddIsClassMethods(bool checkOnly)
{//@CODE_1038
    int value = 0;

    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        if (!checkOnly)
        {
            void* ownerHwnd = Cb_OwnerHwnd();
            Qt_ShowIsClassMethodsDialog(pClass, this, ownerHwnd);
        }
    
        value = 1;
    }

    return value;
}//@CODE_1038


int MemberAndMethodGroup::OnAddMember(bool checkOnly)
{//@CODE_1034
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Member* pMember = new Member(GetBaseClass(), GetDataModelDoc()->FindType(""));
        AddMemberLast(pMember);
        
        if (pMember->OnEditAttributes())
        {
            pMember->Add();
            GetBaseClass()->NotifyAddMember(pMember);
        }
        else
            GetDataModelDoc()->RollBack();
    }
    
    return 1;
}//@CODE_1034


int MemberAndMethodGroup::OnAddMethod(bool checkOnly)
{//@CODE_1035
    if (!checkOnly)
    {
        MethodIterator iMethod(this);
        while(--iMethod)
        {
            if (!iMethod->IsFixed() && 
                !iMethod->IsConstructor() &&
                !iMethod->IsDestructor() &&
                !iMethod->IsMemberMethod() &&
                !iMethod->IsFromRelationMethod())
            {
                break;
            }
        }
        GetDataModelDoc()->MarkLastUndo();
        Method* pMethod = new Method(GetBaseClass(), GetDataModelDoc()->FindType(""));

        // Copy settings from previous added method.
        if (iMethod)
        {
            pMethod->SetAccess(iMethod->GetAccess());
            pMethod->SetVirtual(iMethod->GetVirtual());
            pMethod->SetPure(iMethod->GetPure());
            pMethod->SetStatic(iMethod->GetStatic());
            pMethod->SetConst(iMethod->GetConst());
            pMethod->SetInline(iMethod->GetInline());
        }

        AddMethodLast(pMethod);

        if (pMethod->OnEditAttributes())
        {
            pMethod->Add();
            GetBaseClass()->NotifyAddMethod(pMethod);
        }
        else
            GetDataModelDoc()->RollBack();
    }
    
    return 1;
}//@CODE_1035


int MemberAndMethodGroup::OnAddVirtuals(bool checkOnly)
{//@CODE_1037
    int value = 0;

    ExternClass* pExternClass = dynamic_cast<ExternClass*>(GetBaseClass());
    if (pExternClass)
    {
        if (!checkOnly)
        {
            void* ownerHwnd = Cb_OwnerHwnd();
            Qt_ShowVirtualMethodsDialog(pExternClass, this, ownerHwnd);
        }
    
        value = 1;
    }

    return value;
}//@CODE_1037


int MemberAndMethodGroup::OnDelete(bool checkOnly)
{//@CODE_1033
    int value = 1;
    MemberIterator member(this);
    while (value && ++member)
    {
        value = member->OnDelete(1);
    }

    MethodIterator method(this);
    while (value && ++method)
    {
        value = method->OnDelete(1);
    }

    if (value && !checkOnly)
    {
        bool deleteGroup = false;

        int methodCount = GetMethodCount();
        int memberCount = GetMemberCount();

        CbString str;
        if (methodCount || memberCount)
            str.Format("Delete group '%s'?\n\nThis will also delete the %d method(s) "
                       "and %d member(s) it contains.",
                       GetName(), methodCount, memberCount);
        else
            str.Format("Are you sure you want to delete the empty group '%s'?",
                       GetName());

        // Destructive when the group has content -> default the box to 'No'.
        unsigned int flags = CBMB_ICONQUESTION | CBMB_YESNO;
        if (methodCount || memberCount)
            flags |= CBMB_DEFBUTTON2;

        if (CbMessageBox(str, flags) == CBMB_IDYES)
            deleteGroup = true;

        if (deleteGroup)
        {
            // Coalesce the cascade into a single view refresh. CbViewLock caches
            // the document, so its unlock survives this->Delete() below.
            CbViewLock lock(GetDataModelDoc());

            MemberIterator member(this);
            while (++member)
                member->Delete();

            MethodIterator method(this);
            while (++method)
                method->Delete();

            Delete();
        }
    }
    
    return value;
}//@CODE_1033


int MemberAndMethodGroup::OnEditContext(bool checkOnly)
{//@CODE_26177
    if (checkOnly)
        return true;

    UndoBase* pLastUndoBase = GetDataModelDoc()->MarkLastUndo();
    void* ownerHwnd = Cb_OwnerHwnd();

    if (Qt_ShowContextDialog(this, ownerHwnd))
    {
        // Close the dialog's undo transaction. No manual dirty set: any
        // change went through SaveState (two-place dirty rule).
        GetDataModelDoc()->MarkLastUndo();

        return true;
    }
    else
    {
        GetDataModelDoc()->RollBack(pLastUndoBase);
        return false;
    }
}//@CODE_26177


int MemberAndMethodGroup::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_5826
    int result = 0;
    
    Method* pMethod = dynamic_cast<Method*>(pGti);
    if (pMethod && (GetBaseClass()->IsClass() || !pMethod->IsPrivateMethod()))
    {
		Method* pNewMethod = 0;
		Constructor* pConstructor = dynamic_cast<Constructor*>(pMethod);
		Destructor* pDestructor = dynamic_cast<Destructor*>(pMethod);
		if(pConstructor && !GetBaseClass()->IsClass() && 
		   GetBaseClass()->GetName() == pMethod->GetBaseClass()->GetName())
		{
			result = 1;
			if (!checkOnly)
			{
                pNewMethod = new Constructor(GetBaseClass(), pConstructor);
			}
		}
		else if (pDestructor && !GetBaseClass()->IsClass() && 
			     GetBaseClass()->GetName() == pMethod->GetBaseClass()->GetName())
		{
			result = 1;
			if (!checkOnly)
			{
                pNewMethod = new Destructor(GetBaseClass(), pDestructor);
			}
		}
		else
		{
			result = 1;
			if (!checkOnly)
			{
                pNewMethod = new Method(GetBaseClass(), GetDataModelDoc()->FindOrDuplicateType(pMethod->GetType()), pMethod);
			}
		}
		
		if (pNewMethod)
		{
			if (!GetBaseClass()->IsClass())
			{
				pNewMethod->SetCode("");
			}
            AddMethodLast(pNewMethod);
			pNewMethod->Add();
            GetBaseClass()->NotifyAddMethod(pNewMethod);
		}
    }
    
    Member* pMember = dynamic_cast<Member*>(pGti);
    if (pMember && (GetBaseClass()->IsClass() || !pMember->IsPrivate()))
    {
        if (!checkOnly)
        {
            Member* pNewMember = new Member(GetBaseClass(), GetDataModelDoc()->FindOrDuplicateType(pMember->GetType()), pMember);
            AddMemberLast(pNewMember);
            pNewMember->Add();
            GetBaseClass()->NotifyAddMember(pNewMember);
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
}//@CODE_5826


void MemberAndMethodGroup::SetIcon()
{//@CODE_1030
    if (GetMemberCount() && GetMethodCount())
            Gti::SetIcon(ICON_MEMBERMETHODGROUP);
        else if (GetMemberCount())
            Gti::SetIcon(ICON_MEMBERGROUP);
        else if (GetMethodCount())
            Gti::SetIcon(ICON_METHODGROUP);
        else
            Gti::SetIcon(ICON_FILE);
}//@CODE_1030


void MemberAndMethodGroup::Update()
{//@CODE_1032
    if (GetAdded())
    {
        SetItemText(GetName());
        SetIcon();

        Gti::Update();
    }
}//@CODE_1032


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5474
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MemberAndMethodGroup::CleanupReferences()
{
    Group::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
}


/*@NOTE_539
Method which must be called first in a constructor
*/
void MemberAndMethodGroup::ConstructorInclude(BaseClass* pBaseClass)
{
    INIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    INIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    INIT_MULTI_OWNED_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    INIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
}


/*@NOTE_541
Method which must be called first in a destructor
*/
void MemberAndMethodGroup::DestructorInclude()
{
    EXIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    EXIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    EXIT_MULTI_OWNED_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    EXIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
}


/*@NOTE_5475
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MemberAndMethodGroup::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    REMOVE_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    REMOVE_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    Group::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
}


/*@NOTE_5476
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MemberAndMethodGroup::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberAndMethodGroup* pMemberAndMethodGroup = (MemberAndMethodGroup*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    Group::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5478
Save the state of the current object relations to pDataModelDocObject.
*/
void MemberAndMethodGroup::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Group::SaveReferences(pDataModelDocObject);
    MemberAndMethodGroup* pMemberAndMethodGroup = (MemberAndMethodGroup*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
}


/*@NOTE_544
Serialize the members only to a CbObject object
*/
void MemberAndMethodGroup::Serialize(CbArchive& archive)
{
    Group::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_543
Method which must be called first in a serialize constructor
*/
void MemberAndMethodGroup::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    INIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    INIT_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    INIT_MULTI_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
}


/*@NOTE_546
Serialize the relations to a CbObject object
*/
void MemberAndMethodGroup::SerializeRelations(CbArchive& archive,
                                              DataModelDocObject* pointerArray[])
{
    Group::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
        WRITE_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
        WRITE_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
            READ_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
            READ_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(MemberAndMethodGroup)


// Methods for the relation(s) of the class
METHODS_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
METHODS_ITERATOR_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
METHODS_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
METHODS_ITERATOR_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
METHODS_MULTI_OWNED_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
METHODS_ITERATOR_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
METHODS_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
