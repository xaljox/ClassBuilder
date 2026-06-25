/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Member.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Member'
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
// (ClassBuilderView.h include removed -- the MFC tree view is retired)
#include "qt/QtMemberDialog.h"
#include "qt/QtMemberMethodsDialog.h"
#include "qt/QtContextDialog.h"
//@END_USER2


// Static members


Member::Member(BaseClass* pBaseClass, Type* pType) //@INIT_831
    : Variable(pType)
    , _access(PRIVATE)
    , _initialization("")
    , _delete(0)
    , _serialize(0)
    , _static(0)
    , _bitField(false)
    , _bitFieldSize(0)
    , _mutable(0)
{//@CODE_831
    ConstructorInclude(pBaseClass);

    // Put in your own code
    Class* pClass = dynamic_cast<Class*>(pBaseClass);
    if (!pClass)
    {
        // if extern class, no private members allowed, they arren't visable
        _access = PROTECTED;
    }
    else if (pClass->GetSerialize())
    {
        _serialize = 1;
    }

    InitPhase();
}//@CODE_831


Member::Member(BaseClass* pBaseClass, Member* pMember) //@INIT_1386
    : Variable(*pMember)
    , _access(pMember->_access)
    , _initialization(pMember->_initialization)
    , _delete(pMember->_delete)
    , _serialize(pMember->_serialize)
    , _static(pMember->_static)
    , _bitField(pMember->_bitField)
    , _bitFieldSize(pMember->_bitFieldSize)
    , _mutable(pMember->_mutable)
{//@CODE_1386
    // Rename while the name already exists
    while (pBaseClass->FindMember(Variable::GetName()))
    {
        Variable::SetName(Variable::GetName() + "X");
    }

    ConstructorInclude(pBaseClass);

    // Put in your own code
    
    if (_serialize)
    {
        Class* pClass = dynamic_cast<Class*>(GetBaseClass());
        if (!pClass || !pClass->GetSerialize())
        {
            _serialize = 0;
        }
    }

    InitPhase();
}//@CODE_1386


/*@NOTE_7562
Constructor method needed to copy member from one project to another.
*/
Member::Member(BaseClass* pBaseClass, Type* pType, Member* pMember) //@INIT_7562
    : Variable(pType, pMember)
    , _access(pMember->_access)
    , _initialization(pMember->_initialization)
    , _delete(pMember->_delete)
    , _serialize(pMember->_serialize)
    , _static(pMember->_static)
    , _bitField(pMember->_bitField)
    , _bitFieldSize(pMember->_bitFieldSize)
    , _mutable(pMember->_mutable)
{//@CODE_7562
    // Rename while the name already exists
    while (pBaseClass->FindMember(Variable::GetName()))
    {
        Variable::SetName(Variable::GetName() + "X");
    }

    ConstructorInclude(pBaseClass);

    // Put in your own code
    
    if (_serialize)
    {
        Class* pClass = dynamic_cast<Class*>(GetBaseClass());
        if (!pClass || !pClass->GetSerialize())
        {
            _serialize = 0;
        }
    }

    InitPhase();
}//@CODE_7562


/*@NOTE_178
Constructor needed for serialization, not meant to use for other purposes!
*/
Member::Member() //@INIT_178
    : Variable()
    , _bitField(false)
    , _bitFieldSize(0)
    , _mutable(0)
{//@CODE_178
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_178


/*@NOTE_176
Destructor method
*/
Member::~Member()
{//@CODE_176
    DestructorInclude();

    // Put in your own code
}//@CODE_176


void Member::Add()
{//@CODE_840
    if (!GetAdded())
    {
        SaveState(1);
        if (GetMemberAndMethodGroup())
        {
            GetMemberAndMethodGroup()->AddChildLast(this);
            GetMemberAndMethodGroup()->Update();
        }
        else
            GetBaseClass()->AddChildLast(this);

        SetItemText();
        SetIcon(ICON_PUBLIC_MEMBER + _access);

        Gti::Add();

        MethodIterator method(this);
        while (++method)
            method->Add();
    }
}//@CODE_840


int Member::CompareTree(Member* pMember1, Member* pMember2)
{//@CODE_4959
    if (pMember1->GetStatic() == pMember2->GetStatic())
    {
        if (pMember1->GetAccess() == pMember2->GetAccess())
            return pMember1->GetName().CompareNoCase(pMember2->GetName());
        else
            return int(pMember1->GetAccess())-int(pMember2->GetAccess());
    }
    else
        return int(pMember1->GetStatic())-int(pMember2->GetStatic());
}//@CODE_4959


Member& Member::CopyValuesFrom(Member& rMember)
{//@CODE_1598
    Variable::CopyValuesFrom(rMember);
    
    _access = rMember._access;
    _initialization = rMember._initialization;
    _delete = rMember._delete;
    _serialize = rMember._serialize;
    _static = rMember._static;
    _bitField = rMember._bitField;
    _bitFieldSize = rMember._bitFieldSize;
    _mutable = rMember._mutable;
    
    if (_serialize)
    {
        Class* pClass = dynamic_cast<Class*>(GetBaseClass());
        if (!pClass || !pClass->GetSerialize())
        {
            _serialize = 0;
        }
    }
    
    return *this;
}//@CODE_1598


Context* Member::CreateContext(ContextDeclaration* pContextDeclaration)
{//@CODE_25515
    return new MemberContext(this, pContextDeclaration);
}//@CODE_25515


/*@NOTE_5821
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void Member::Delete()
{//@CODE_5821
    int version = GetDataModelDoc()->GetVersion();
    if (GetVersion() <= version)
    {
        Class* pClass = dynamic_cast<Class*>(GetBaseClass());
        if (pClass)
        {
            pClass->SaveState(1);
            pClass->SetVersion(version + 1);

            CbString str;
            str.Format("@Deleted member '%s'", GetPrefixedName().c_str());
    
            pClass->AddModified(str);
        }
    }
    GetBaseClass()->NotifyRemoveMember(this);

    DataModelDocObject::Delete();
}//@CODE_5821


bool Member::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1341
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = true;
    }
    else
    {
        MemberAndMethodGroup* pMemberAndMethodGroup = GetMemberAndMethodGroup();
        if (pMemberAndMethodGroup)
        {
            SaveState(1);
            pGtiDropDefault = pMemberAndMethodGroup;
            pMemberAndMethodGroup->RemoveMember(this);
            pMemberAndMethodGroup->Update();
        }
        else
            pGtiDropDefault = GetBaseClass();

        Remove();
        value = true;
    }

    return value;
}//@CODE_1341


void Member::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1347
    DropOnClass(ctrlKeyDown, pGtiDrop);
    DropOnMethod(ctrlKeyDown, pGtiDrop);
}//@CODE_1347


void Member::DropOnClass(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1653
    BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
    MemberAndMethodGroup* pDropMemberAndMethodGroup = 
        dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);

    if (ctrlKeyDown)
    {
        if (pDropClass || pDropMemberAndMethodGroup)
        {
            if (pDropMemberAndMethodGroup)
                pDropClass = pDropMemberAndMethodGroup->GetBaseClass();

            Member* pNewMember = new Member(pDropClass, this);

            if (pDropMemberAndMethodGroup)
                pDropMemberAndMethodGroup->AddMemberLast(pNewMember);

            pNewMember->Add();
            pDropClass->NotifyAddMember(pNewMember);
        }
    }
    else
    {
        if (pDropClass || pDropMemberAndMethodGroup)
        {
            if (pDropMemberAndMethodGroup)
                pDropClass = pDropMemberAndMethodGroup->GetBaseClass();

            if (pDropClass != GetBaseClass())
                pDropClass->MoveMember(this);

            if (pDropMemberAndMethodGroup)
                pDropMemberAndMethodGroup->AddMemberLast(this);

            Add();
        }
    }
}//@CODE_1653


void Member::DropOnMethod(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1656
    Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
    
    if (ctrlKeyDown)
    {
        if (pDropMethod)
        {
            int argumentCount = 0;
            Argument** pArgumentArray = 0;

            // In case of virtual function, adjust virtual overides also.
            if (pDropMethod->GetVirtual())
            {
                pArgumentArray = 
                    new Argument*[GetDataModelDoc()->GetTypeCount()];

                DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsExternClass);
                while (++iType)
                {
                    ExternClass* pDerivedClass = (ExternClass*)iType.Get();

                    if (pDerivedClass->IsBaseClass(pDropMethod->GetBaseClass()))
                    {
                        Method* pMethod = pDerivedClass->FindSimilarMethod(pDropMethod);
                        if (pMethod)
                        {
                            pArgumentArray[argumentCount] = new Argument(pMethod, GetType());
                            pArgumentArray[argumentCount++]->Variable::CopyValuesFrom (*this);
                        }
                    }
                }
            }

            Argument* pNewArgument = new Argument(pDropMethod, GetType());
            pNewArgument->Variable::CopyValuesFrom (*this);

            if (pNewArgument->OnEditAttributes())
            {
                pNewArgument->Add();
            
                for (int i = 0; i < argumentCount; i++)
                    pArgumentArray[i]->Add();
            }
            else
            {
                GetDataModelDoc()->RollBack();
            }

            delete pArgumentArray;
        }
    }
    else
    {
    }
}//@CODE_1656


bool Member::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1344
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
        if (pDropMethod)
        {
            if (!pDropMethod->IsFixed() && 
                !pDropMethod->IsFromRelationMethod() &&
                !pDropMethod->IsMemberMethod() &&
                !pDropMethod->IsDestructor() &&
                !pDropMethod->IsSerializeConstructor() &&
                !pDropMethod->IsReplaceConstructor())
            {
                value = true;
            }
        }
        else
        {
            BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
            MemberAndMethodGroup* pDropMemberAndMethodGroup = 
                dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);

            if (pDropMemberAndMethodGroup)
                pDropClass = pDropMemberAndMethodGroup->GetBaseClass();

            if (pDropClass && !pDropClass->FindMember(GetName()))
                value = true;
        }
    }
    else
    {
        if (dynamic_cast<ExternClass*>(pGtiDrop) ||
            dynamic_cast<MemberAndMethodGroup*>(pGtiDrop))
        {
            value = true;
        }
    }

    return value;
}//@CODE_1344


MemberArgument* Member::FindMemberArgument(Method* pMethod)
{//@CODE_19544
    MemberArgumentIterator iMemberArgument(this);
    while (++iMemberArgument)
    {
        if (pMethod == iMemberArgument->GetMethod())
        {
            return iMemberArgument;
        }
    }

    return 0;
}//@CODE_19544


MemberShape* Member::FindMemberShape(ClassShape* pClassShape)
{//@CODE_3958
    MemberShapeIterator iMemberShape(this);
    while (++iMemberShape)
    {
        if (pClassShape == iMemberShape->GetClassShape())
        {
            return iMemberShape;
        }
    }

    return 0;
}//@CODE_3958


MemberMethod* Member::FindMethod(const CbString& rName)
{//@CODE_1394
    MethodIterator iMemberMethod(this);
    while (++iMemberMethod)
    {
        if (rName == iMemberMethod->GetName())
        {
            return iMemberMethod;
        }
    }

    return 0;
}//@CODE_1394


MemberMethod* Member::FindSimilarMethod(Method* pMethod)
{//@CODE_23499
    MethodIterator iMemberMethod(this);
    while (++iMemberMethod)
    {
        if (iMemberMethod->IsSimilar(pMethod))
        {
            return iMemberMethod;
        }
    }

    return 0;
}//@CODE_23499


CbString Member::GetContextList()
{//@CODE_27309
    CbString str;
   
    if (GetMemberAndMethodGroup())
    {
        MemberAndMethodGroup::MemberAndMethodGroupContextIterator 
            iMemberAndMethodGroupContext(GetMemberAndMethodGroup());
        while (++iMemberAndMethodGroupContext)
        {
            if (!str.IsEmpty())
                str += ", ";
            str += iMemberAndMethodGroupContext->GetContextDeclaration()->GetName();
        }
    }
    
    MemberContextIterator iMemberContext(this);
    while (++iMemberContext)
    {
        if (!str.IsEmpty())
            str += ", ";
        str += iMemberContext->GetContextDeclaration()->GetName();
    }

    return str;
}//@CODE_27309


CbString Member::GetEndContextDeclaration()
{//@CODE_26336
    CbString str;
    
    MemberContextIterator iMemberContext(this);
    while (--iMemberContext)
    {
        str += iMemberContext->GetContextDeclaration()->GetEndContextDeclaration();
    }

    if (GetMemberAndMethodGroup())
    {
        MemberAndMethodGroup::MemberAndMethodGroupContextIterator 
            iMemberAndMethodGroupContext(GetMemberAndMethodGroup());
        while (--iMemberAndMethodGroupContext)
        {
            str += iMemberAndMethodGroupContext->GetContextDeclaration()->GetEndContextDeclaration();
        }
    }
    
    return str;
}//@CODE_26336


CbString Member::GetEndContextImplementation()
{//@CODE_26338
    CbString str;
    
    MemberContextIterator iMemberContext(this);
    while (--iMemberContext)
    {
        str += iMemberContext->GetContextDeclaration()->GetEndContextImplementation();
    }

    if (GetMemberAndMethodGroup())
    {
        MemberAndMethodGroup::MemberAndMethodGroupContextIterator 
            iMemberAndMethodGroupContext(GetMemberAndMethodGroup());
        while (--iMemberAndMethodGroupContext)
        {
            str += iMemberAndMethodGroupContext->GetContextDeclaration()->GetEndContextImplementation();
        }
    }
    
    return str;
}//@CODE_26338


Context* Member::GetFirstContext()
{//@CODE_26244
    return GetFirstMemberContext();
}//@CODE_26244


Context* Member::GetNextContext(Context* pContextPos)
{//@CODE_26245
    return GetNextMemberContext((MemberContext*)pContextPos);
}//@CODE_26245


/*@NOTE_1532
Returns the memberprefix, strored at DataModel concatenated with its name,
*/
CbString Member::GetPrefixedName()
{//@CODE_1532
    if (!GetName().IsEmpty())
        return GetBaseClass()->GetMemberPrefix() + GetName();
    else
        return GetName();
}//@CODE_1532


/*@NOTE_4747
Give the text as it should appear in the class diagram, it is a method of this class in 
instead of MemberShape, because of the Select dialog.
*/
CbString Member::GetShapeText(VerbosityType verbosity)
{//@CODE_4747
    CbString text;
    switch (GetAccess())
    {
    case PRIVATE:
        text = "- ";
        break;
    case PROTECTED:
        text = "# ";
        break;
    case PUBLIC:
        text = "+ ";
        break;
    }

    if (GetMutable())
    {
        text += "<<Mutable>> ";
    }

    text += GetBaseClass()->GetMemberPrefix() + GetVariableName();
    text += " : ";
    if (GetStatic() && (verbosity&VERBOSITY_STATIC))
        text += "static ";
    text += GetTypeName();
    text.TrimRight();
    if (GetBitField())
    {
        CbString bitField;
        bitField.Format(":%d", GetBitFieldSize());
        text += bitField;
    }

    if (!GetInitialization().IsEmpty())
	text += " = " + GetInitialization();

    return text;
}//@CODE_4747


CbString Member::GetStartContextDeclaration()
{//@CODE_26335
    CbString str;

    if (GetMemberAndMethodGroup())
    {
        MemberAndMethodGroup::MemberAndMethodGroupContextIterator 
            iMemberAndMethodGroupContext(GetMemberAndMethodGroup());
        while (++iMemberAndMethodGroupContext)
        {
            str += iMemberAndMethodGroupContext->GetContextDeclaration()->GetStartContextDeclaration();
        }
    }
    
    MemberContextIterator iMemberContext(this);
    while (++iMemberContext)
    {
        str += iMemberContext->GetContextDeclaration()->GetStartContextDeclaration();
    }
    
    return str;
}//@CODE_26335


CbString Member::GetStartContextImplementation()
{//@CODE_26337
    CbString str;

    if (GetMemberAndMethodGroup())
    {
        MemberAndMethodGroup::MemberAndMethodGroupContextIterator 
            iMemberAndMethodGroupContext(GetMemberAndMethodGroup());
        while (++iMemberAndMethodGroupContext)
        {
            str += iMemberAndMethodGroupContext->GetContextDeclaration()->GetStartContextImplementation();
        }
    }
    
    MemberContextIterator iMemberContext(this);
    while (++iMemberContext)
    {
        str += iMemberContext->GetContextDeclaration()->GetStartContextImplementation();
    }
    
    return str;
}//@CODE_26337


void Member::InitPhase()
{//@CODE_23461
    if (GetBaseClass()->IsClass())
    {
        if (GetPhase() == None_Phase)
        {
            if (GetBaseClass()->GetPhase() > Design_Phase)
            {
                SetPhase(Implementation_Phase);
            }
            else
            {
                SetPhase(GetBaseClass()->GetPhase());
            }
        }
    }
    else
    {
        SetPhase(None_Phase);
    }
}//@CODE_23461


bool Member::IsNonStatic() const
{//@CODE_839
 return (_static? 0: 1); 
}//@CODE_839


bool Member::IsPrivate() const
{//@CODE_838
 return (_access == PRIVATE? 1: 0); 
}//@CODE_838


bool Member::IsProtected() const
{//@CODE_837
 return (_access == PROTECTED? 1: 0); 
}//@CODE_837


bool Member::IsPublic() const
{//@CODE_836
 return (_access == PUBLIC? 1: 0); 
}//@CODE_836


int Member::OnAddMethod(bool checkOnly)
{//@CODE_1393
    if (!GetType()->IsBaseClass())
    {
        return Gti::OnAddMethod(checkOnly);
    }

    if (!checkOnly)
    {
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowMemberMethodsDialog(this, ownerHwnd);
    }
    
    return 1;
}//@CODE_1393


int Member::OnDelete(bool checkOnly)
{//@CODE_843
    if (GetName() == "membersOnly" &&
        GetBaseClass() == GetDataModelDoc()->GetDataModel()->GetDocument())
    {
        if (!checkOnly)
            CbMessageBox(
                "The static '_membersOnly' flag on the document is referenced"
                " by the generated Serialize body (and used by SerializeMembersOnly"
                " when present) and cannot be deleted.",
                CBMB_ICONEXCLAMATION);
        return 0;
    }

    if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete member '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }

    return 1;
}//@CODE_843


int Member::OnEditAttributes(bool checkOnly)
{//@CODE_842
	if (checkOnly)
		return 1;

    if (GetName() == "membersOnly" &&
        GetBaseClass() == GetDataModelDoc()->GetDataModel()->GetDocument())
    {
        CbMessageBox(
            "The static '_membersOnly' flag on the document is referenced"
            " by the generated Serialize body (and used by SerializeMembersOnly"
            " when present) and cannot be edited.",
            CBMB_ICONEXCLAMATION);
        return 0;
    }

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;
    if (Qt_ShowMemberDialog(this, changed, ownerHwnd))
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
}//@CODE_842


int Member::OnEditContext(bool checkOnly)
{//@CODE_25759
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
}//@CODE_25759


/*@NOTE_22945
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Member::OnUndoRedoAdded()
{//@CODE_22945
    Gti::OnUndoRedoAdded();
    
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }
}//@CODE_22945


/*@NOTE_22951
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Member::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22951
    Gti::OnUndoRedoChanged(pOldState);
    
    Member* pMember = (Member*)pOldState;
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }
    if (pMember && pMember->GetMemberAndMethodGroup())
    {
        pMember->GetMemberAndMethodGroup()->Update();
    }
}//@CODE_22951


/*@NOTE_22935
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called after the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Member::OnUndoRedoRemoved()
{//@CODE_22935
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }
}//@CODE_22935


void Member::SetItemText()
{//@CODE_830
    CbString itemText;
    if (GetStatic())
        itemText += "static ";

    if (GetMutable())
        itemText += "mutable ";

    if (!GetName().IsEmpty())
        itemText += GetTypeName() + GetBaseClass()->GetMemberPrefix() + GetVariableName();
    else
        itemText += GetTypeName();

    if (GetBitField())
    {
        CbString bitField;
        bitField.Format(":%d", GetBitFieldSize());
        itemText += bitField;
    }

    Gti::SetItemText(itemText);
}//@CODE_830


void Member::SetName(const CbString& rName)
{//@CODE_834
    SaveState();

    if (GetName().IsEmpty())
    {
        Variable::SetName(rName);
        Member::MemberArgumentIterator argument(this);
        while (++argument)
            argument->UpdateName();
    }
    else
    {
        if (rName != GetName())
        {
            CbString oldString = GetPrefixedName();
            CbString newString = GetBaseClass()->GetMemberPrefix() + rName;

            // Do modification on code of methods first
            BaseClass::MethodIterator iMethod(GetBaseClass(), &Method::IsNonMacroMethod);
            while (++iMethod)
            {
                iMethod->ReplaceInCode(oldString, newString);
            }

            if (GetAccess() != PRIVATE)
            {
                DataModel::ClassIterator iClass(GetDataModelDoc()->GetDataModel());
                while (++iClass)
                {
                    if (iClass->IsBaseClass(GetBaseClass()))
                    {
                        BaseClass::MethodIterator iMethod(iClass, &Method::IsNonMacroMethod);
                        while (++iMethod)
                        {
                            iMethod->ReplaceInCode(oldString, newString);
                        }
                    }
                }
            }

            oldString = GetName();
            if (GetPointer() && oldString[0] == 'p')
            {
                oldString = oldString.Mid(1);
            } 
            if (GetPointerPointer() && oldString[0] == 'p')
            {
                oldString = oldString.Mid(1);
            } 
            if (GetReference() && oldString[0] == 'r')
            {
                oldString = oldString.Mid(1);
            }
			if (islower(oldString[0]))
			{
				oldString = GetName();
			}

            oldString.SetAt(0, toupper(oldString[0]));

            newString = rName;
            if (GetPointer() && newString[0] == 'p')
            {
                newString = newString.Mid(1);
            } 
            if (GetPointerPointer() && newString[0] == 'p')
            {
                newString = newString.Mid(1);
            } 
            if (GetReference() && newString[0] == 'r')
            {
                newString = newString.Mid(1);
            }
			if (islower(newString[0]))
			{
				newString = rName;
			}
            newString.SetAt(0, toupper(newString[0]));

            Variable::SetName(rName);

            if (oldString != newString)
            {
                if (GetGetMemberMethod() && 
                    GetGetMemberMethod()->GetName() == "Get" + oldString)
                {
                    GetGetMemberMethod()->SaveState();
                    GetGetMemberMethod()->SetName("Get" + newString);
                }

                if (GetSetMemberMethod() && 
                    GetSetMemberMethod()->GetName() == "Set" + oldString)
                {
                    GetSetMemberMethod()->SaveState();
                    GetSetMemberMethod()->SetName("Set" + newString);
                }
            }

            Member::MemberArgumentIterator argument(this);
            while (++argument)
                argument->UpdateName();
        }
    }
}//@CODE_834


bool Member::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40778
    AccessType access = GetAccess();
    bool show = (access == PUBLIC    && pTreeViewModel->GetShowPublicMembers())
             || (access == PROTECTED && pTreeViewModel->GetShowProtectedMembers())
             || (access == PRIVATE   && pTreeViewModel->GetShowPrivateMembers());
    if (!show)
    {
        return false;
    }

    if (!(GetStatic() ? pTreeViewModel->GetShowStaticMembers()
                      : pTreeViewModel->GetShowNonStaticMembers()))
    {
        return false;
    }

    return Gti::ShownByFilter(pTreeViewModel);
}//@CODE_40778


void Member::Update()
{//@CODE_841
    if (GetAdded())
    {
        SetItemText();
        SetIcon(ICON_PUBLIC_MEMBER + _access);

        /*
        ChildIterator gti(this);
        while (++gti)
            gti->Remove();
            */

        Member::MemberArgumentIterator argument(this);
        while (++argument)
            argument->Update();

        Gti::Update();

        MethodIterator method(this);
        while (++method)
        {
            if (method->GetAdded())
                method->Update();
            else
                method->Add();
        }
    }
}//@CODE_841


AccessType Member::GetAccess()
{//@CODE_1195
    return _access;
}//@CODE_1195


void Member::SetAccess(AccessType access)
{//@CODE_1196
    _access = access;
}//@CODE_1196


bool Member::GetDelete() const
{//@CODE_1201
    return _delete;
}//@CODE_1201


void Member::SetDelete(bool val)
{//@CODE_1202
    _delete = val;
}//@CODE_1202


const CbString& Member::GetInitialization()
{//@CODE_1198
    return _initialization;
}//@CODE_1198


void Member::SetInitialization(const CbString& rInitialization)
{//@CODE_1199
    if (_initialization != rInitialization)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

		if (!rInitialization.IsEmpty() || !GetType()->IsOtherType())
		{
			CbString oldString = GetPrefixedName() + "(" + _initialization + ")";
			CbString newString = GetPrefixedName() + "(" + rInitialization + ")";

			BaseClass::MethodIterator iMethod(GetBaseClass(), &Method::IsConstructor);
			while (++iMethod)
			{
				Constructor* pConstructor = (Constructor*)iMethod.Get();
				pConstructor->ReplaceInInit(oldString, newString);
			}
		}
        _initialization = rInitialization;
    }
}//@CODE_1199


/*@NOTE_19517
Returns the value of member '_mutable'.
*/
bool Member::GetMutable() const
{//@CODE_19517
    return _mutable;
}//@CODE_19517


/*@NOTE_19518
Set the value of member '_mutable' to 'val'.
*/
void Member::SetMutable(bool val)
{//@CODE_19518
    _mutable = val;
}//@CODE_19518


bool Member::GetSerialize() const
{//@CODE_1204
    return _serialize;
}//@CODE_1204


void Member::SetSerialize(bool serialize)
{//@CODE_1205
    _serialize = serialize;
}//@CODE_1205


bool Member::GetStatic() const
{//@CODE_1207
    return _static;
}//@CODE_1207


void Member::SetStatic(bool val)
{//@CODE_1208
    _static = val;
}//@CODE_1208


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5468
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Member::CleanupReferences()
{
    Variable::CleanupReferences();
    CLEANUP_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    CLEANUP_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)
}


/*@NOTE_175
Method which must be called first in a constructor
*/
void Member::ConstructorInclude(BaseClass* pBaseClass)
{
    INIT_MULTI_OWNED_ACTIVE(Member, Member, MemberMethod, Method)
    INIT_MULTI_OWNED_ACTIVE(Member, Member, MemberArgument, MemberArgument)
    INIT_SINGLE_OWNED_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
    INIT_SINGLE_OWNED_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
    INIT_MULTI_OWNED_ACTIVE(Member, Member, RelationMember, RelationMember)
    INIT_MULTI_OWNED_ACTIVE(Member, Member, MemberShape, MemberShape)
    INIT_MULTI_OWNED_ACTIVE(Member, Member, MemberContext, MemberContext)
    INIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    INIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)
}


/*@NOTE_177
Method which must be called first in a destructor
*/
void Member::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Member, Member, MemberMethod, Method)
    EXIT_MULTI_OWNED_ACTIVE(Member, Member, MemberArgument, MemberArgument)
    EXIT_SINGLE_OWNED_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
    EXIT_SINGLE_OWNED_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
    EXIT_MULTI_OWNED_ACTIVE(Member, Member, RelationMember, RelationMember)
    EXIT_MULTI_OWNED_ACTIVE(Member, Member, MemberShape, MemberShape)
    EXIT_MULTI_OWNED_ACTIVE(Member, Member, MemberContext, MemberContext)
    EXIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    EXIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)
}


/*@NOTE_5469
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Member::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(Member, Member, MemberContext, MemberContext)
    REMOVE_MULTI_OWNED_ACTIVE(Member, Member, MemberShape, MemberShape)
    REMOVE_MULTI_OWNED_ACTIVE(Member, Member, RelationMember, RelationMember)
    REMOVE_SINGLE_OWNED_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
    REMOVE_SINGLE_OWNED_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
    REMOVE_MULTI_OWNED_ACTIVE(Member, Member, MemberArgument, MemberArgument)
    REMOVE_MULTI_OWNED_ACTIVE(Member, Member, MemberMethod, Method)
    Variable::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)
    REMOVE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
}


/*@NOTE_5470
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Member::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Member* pMember = (Member*)pDataModelDocObject;
    RESTORE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    RESTORE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)
    Variable::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5472
Save the state of the current object relations to pDataModelDocObject.
*/
void Member::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Variable::SaveReferences(pDataModelDocObject);
    Member* pMember = (Member*)pDataModelDocObject;
    SAVE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    SAVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)
}


/*@NOTE_180
Serialize the members only to a CbObject object
*/
void Member::Serialize(CbArchive& archive)
{
    Variable::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _access;
        archive << _initialization;
        archive << _delete;
        archive << _serialize;
        archive << _static;
        archive << _bitField;
        archive << _bitFieldSize;
        archive << _mutable;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _access;
            archive >> _initialization;
            archive >> _delete;
            archive >> _serialize;
            archive >> _static;
            archive >> _bitField;
            archive >> _bitFieldSize;
            archive >> _mutable;
        }
    }
}


/*@NOTE_179
Method which must be called first in a serialize constructor
*/
void Member::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Member, Member, MemberMethod, Method)
    INIT_MULTI_ACTIVE(Member, Member, MemberArgument, MemberArgument)
    INIT_SINGLE_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
    INIT_SINGLE_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
    INIT_MULTI_ACTIVE(Member, Member, RelationMember, RelationMember)
    INIT_MULTI_ACTIVE(Member, Member, MemberShape, MemberShape)
    INIT_MULTI_ACTIVE(Member, Member, MemberContext, MemberContext)
    INIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    INIT_MULTI_PASSIVE(BaseClass, BaseClass, Member, Member)
}


/*@NOTE_182
Serialize the relations to a CbObject object
*/
void Member::SerializeRelations(CbArchive& archive,
                                DataModelDocObject* pointerArray[])
{
    Variable::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Member, Member, MemberMethod, Method)
        WRITE_MULTI_ACTIVE(Member, Member, MemberArgument, MemberArgument)
        WRITE_SINGLE_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
        WRITE_SINGLE_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
        WRITE_MULTI_ACTIVE(Member, Member, RelationMember, RelationMember)
        WRITE_MULTI_ACTIVE(Member, Member, MemberShape, MemberShape)
        WRITE_MULTI_ACTIVE(Member, Member, MemberContext, MemberContext)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Member, Member, MemberMethod, Method)
            READ_MULTI_ACTIVE(Member, Member, MemberArgument, MemberArgument)
            READ_SINGLE_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
            READ_SINGLE_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
            READ_MULTI_ACTIVE(Member, Member, RelationMember, RelationMember)
            READ_MULTI_ACTIVE(Member, Member, MemberShape, MemberShape)
            READ_MULTI_ACTIVE(Member, Member, MemberContext, MemberContext)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Member)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Member, Member, MemberMethod, Method)
METHODS_ITERATOR_MULTI_ACTIVE(Member, Member, MemberMethod, Method)
METHODS_MULTI_OWNED_ACTIVE(Member, Member, MemberArgument, MemberArgument)
METHODS_ITERATOR_MULTI_ACTIVE(Member, Member, MemberArgument, MemberArgument)
METHODS_SINGLE_OWNED_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
METHODS_SINGLE_OWNED_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
METHODS_MULTI_OWNED_ACTIVE(Member, Member, RelationMember, RelationMember)
METHODS_ITERATOR_MULTI_ACTIVE(Member, Member, RelationMember, RelationMember)
METHODS_MULTI_OWNED_ACTIVE(Member, Member, MemberShape, MemberShape)
METHODS_ITERATOR_MULTI_ACTIVE(Member, Member, MemberShape, MemberShape)
METHODS_MULTI_OWNED_ACTIVE(Member, Member, MemberContext, MemberContext)
METHODS_ITERATOR_MULTI_ACTIVE(Member, Member, MemberContext, MemberContext)
METHODS_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
METHODS_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
