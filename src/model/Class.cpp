/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Class.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Class'
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
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "ClassBuilderDoc.h"
// (ClassBuilderView.h include removed -- the MFC tree view is retired)
#include "qt/QtClassDialog.h"
#include "SourceLogInterface.h"
#include "ParseLogInterface.h"
#include "qt/QtContextDialog.h"
#include "qt/QtIsClassMethodsDialog.h"

extern CbString EscapeQuotes(const CbString& str);
extern void Note2Mdl(std::ostream& os, const CbString& offset, const CbString& note);

/* Save this piece of code to insert in SerializeRelations
        if (230 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Class, FromClass, Relation, FromRelation);
            READ_MULTI_ACTIVE(Class, ToClass, Relation, ToRelation);
            READ_SINGLE_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod);
            READ_MULTI_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod);
        }
        else
        {
            READ_MULTI_ACTIVE(Class, FromClass, Relation, FromRelation);
            READ_MULTI_ACTIVE(Class, ToClass, Relation, ToRelation);
            READ_MULTI_ACTIVE(BaseClass, BaseClass, Member, Member);
            READ_MULTI_ACTIVE(ExternClass, ExternClass, Inherit, Inherit);
            READ_SINGLE_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod);
            READ_MULTI_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup);
            READ_MULTI_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod);
        }
*/
//@END_USER2


// Static members


Class::Class(DataModel* pDataModel) //@INIT_737
    : ExternClass(pDataModel->GetDataModelDoc())
    , _cppFile("")
    , _cppHeader("")
    , _cppUser1("")
    , _cppUser2("")
    , _cppUser3("")
    , _hFile("")
    , _hHeader("")
    , _hUser1("")
    , _hUser2("")
    , _modified("")
    , _note("")
    , _flag(0)
    , _replace(0)
    , _dllExport(false)
    , _serialize(pDataModel->GetSerialize())
    , _relationMacrosLast(0)
{//@CODE_737
    ConstructorInclude(pDataModel);

    // Put in your own code
    (void)new ConstructorIncludeMethod(this);
    Destructor* pDestructor = dynamic_cast<Destructor*>(GetFirstMethod());
    if (pDestructor)
    {
        pDestructor->SetImplement(true);
        pDestructor->SetDeclare(true);
        (void)new DestructorIncludeMethod(pDestructor);
    }
    else
    {
        CbMessageBox("Some internal error, could not find destructor");
    }

    if (GetSerialize())
        (void)new SerializeConstructor(this);

    MemberAndMethodGroup* pMemberAndMethodGroup = new MemberAndMethodGroup(this);
    pMemberAndMethodGroup->SetName("ClassBuilder methods");
    MethodIterator method(this, &Method::IsNonMacroMethod);
    while (++method)
    {
        if (!method->IsDestructor())
        {
            method->SaveState(1);
            pMemberAndMethodGroup->AddMethodLast(method);
        }
    }
    
    InitMemberPrefix();
    InitPhase();
}//@CODE_737


/*@NOTE_126
Constructor needed for serialization, not meant to use for other purposes!
*/
Class::Class() //@INIT_126
    : ExternClass()
    , _flag(0)
    , _dllExport(false)
    , _serialize(true)
    , _cppUser3("")
    , _hUser3("")
    , _relationMacrosLast(0)
{//@CODE_126
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_126


/*@NOTE_124
Destructor method
*/
Class::~Class()
{//@CODE_124
    DestructorInclude();

    // Put in your own code
}//@CODE_124


void Class::Add()
{//@CODE_756
    if (!GetAdded())
    {
        SaveState(1);
        if (GetClassGroup())
            GetClassGroup()->AddChildLast(this);
        else
            GetDataModel()->AddChildLast(this);

        SetIcon(ICON_CLASS);
        if (GetDllExport() && GetDataModel()->GetShowDllExport())
        {
            SetItemText("AFX_EXT_CLASS " + GetName());
        }
        else
        {
            SetItemText(GetName());
        }

        Gti::Add();

        Gti::ChildIterator iClassDiagram(this, &Gti::IsClassDiagram);
        while (++iClassDiagram)
            iClassDiagram->Add();

        Gti::ChildIterator iSequenceDiagram(this, &Gti::IsSequenceDiagram);
        while (++iSequenceDiagram)
            iSequenceDiagram->Add();
        
        InheritIterator inherit(this);
        while (++inherit)
            inherit->Add();

        FromRelationIterator fromRel(this);
        while (++fromRel)
            fromRel->GetFromRelation()->Add();

        ToRelationIterator toRel(this);
        while (++toRel)
            toRel->GetToRelation()->Add();

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
}//@CODE_756


void Class::AddModified(const CbString& val)
{//@CODE_739
    SaveState();
    _modified += val; 
}//@CODE_739


int Class::CompareName(Class* pA, Class* pB)
{//@CODE_1826
    int result = pA->GetName().CompareNoCase(pB->GetName());

    if (result > 0)
    {
        pB->SaveState();
        pB->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_1826


int Class::ComparePhase(Class* pA, Class* pB)
{//@CODE_23484
    int result = pA->GetPhase() - pB->GetPhase();

    if (result > 0)
    {
        pB->SaveState();
        pB->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_23484


Context* Class::CreateContext(ContextDeclaration* pContextDeclaration)
{//@CODE_27114
    return new ClassContext(this, pContextDeclaration);
}//@CODE_27114


bool Class::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1321
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = true;
    }
    else
    {
        DataModel* pDataModel = GetDataModel();
        ClassGroup* pClassGroup = GetClassGroup();

        if (pClassGroup)
        {
            if (pClassGroup->GetPrevClass(this))
                pGtiDropDefault = pClassGroup->GetPrevClass(this);
            else
                pGtiDropDefault = pClassGroup;

            SaveState(1);
            pClassGroup->RemoveClass(this);
        }
        else
        {
            if (pDataModel->GetPrevClass(this))
                pGtiDropDefault = pDataModel->GetPrevClass(this);
            else
                pGtiDropDefault = pDataModel;
        }

        Remove();
        value = true;
    }

    return value;
}//@CODE_1321


void Class::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1327
    if (ctrlKeyDown)
    {
        BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
        MemberAndMethodGroup* pDropMemberAndMethodGroup = 
            dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);
        
        if (pDropClass || pDropMemberAndMethodGroup)
        {
            if (pDropMemberAndMethodGroup)
                pDropClass = pDropMemberAndMethodGroup->GetBaseClass();
            
            Member* pNewMember = new Member(pDropClass, this);
            pNewMember->SetName(GetFirstLowerName());
            
            if (pNewMember->OnEditAttributes())
            {
                if (pDropMemberAndMethodGroup)
                {
                    pDropMemberAndMethodGroup->AddMemberLast(pNewMember);
                }
                
                pNewMember->Add();
                pDropClass->NotifyAddMember(pNewMember);
            }
            else
                GetDataModelDoc()->RollBack();
        }
        else
            BaseClass::Drop(ctrlKeyDown, pGtiDrop);
    }
    else
    {
        DataModelDoc* pDocument = GetDataModelDoc();
        ExternClasses* pExternClasses = dynamic_cast<ExternClasses*>(pGtiDrop);
        
        if (pExternClasses)
        {
            CbString str;
            str.Format("Are you sure you want to degrade class '%s' into an external class, this action can not be undone!!", 
                GetName());
            if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_OKCANCEL) == CBMB_IDOK)
            {
                CbViewLock lock(pDocument);
                
                ExternClass* pExternClass = new ExternClass(this);
                pExternClass->Add();
                pExternClass->DataModelDocObject::GetDataModelDoc()->DeleteAllUndoBase();
                pExternClass->DataModelDocObject::GetDataModelDoc()->DeleteAllRedoBase();
                
            }
            else
            {
                CbViewLock lock(pDocument);
                
                Add();
                GetDataModelDoc()->RollBack();
                
            }
        }
        else
        {
            CbViewLock lock(pDocument);
            
            DataModel* pDataModel = dynamic_cast<DataModel*>(pGtiDrop);
            ClassGroup* pClassGroup = dynamic_cast<ClassGroup*>(pGtiDrop);
            Class* pClass = dynamic_cast<Class*>(pGtiDrop);
            if (pDataModel)
            {
                pDataModel->MoveClassFirst(this);
            }
            else if (pClassGroup)
            {
                pClassGroup->AddClassFirst(this);
            }
            else if (pClass)
            {
                pClassGroup = pClass->GetClassGroup();
                if (pClassGroup)
                    pClassGroup->AddClassAfter(this, pClass);
                else
                    GetDataModel()->MoveClassAfter(this, pClass);
            }
            
            DataModel::ClassIterator iClass(GetDataModel());
            while (++iClass)
            {
                iClass->SaveState(1);
            }
            
            if (pClassGroup)
            {
                int i = 0;
                ClassGroup::ClassIterator iClass(pClassGroup);
                while (++iClass)
                {
                    iClass->SetOrder(i++);
                }
            }
            else
            {
                int i = 0;
                DataModel::ClassIterator iClass(GetDataModel());
                while (++iClass)
                {
                    if (!iClass->GetClassGroup())
                    {
                        iClass->SetOrder(i++);
                    }
                }
            }
            Add();
            
        }
    }
}//@CODE_1327


bool Class::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1324
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        if (pGtiDrop->IsClass() ||
            pGtiDrop->IsMemberAndMethodGroup())
        {
            value = true;
        }
        else
            return BaseClass::DropTarget(ctrlKeyDown, pGtiDrop);
    }
    else
    {
        if (pGtiDrop->IsDataModel() || pGtiDrop->IsClassGroup() || 
            pGtiDrop->IsClass() || pGtiDrop->IsExternClasses())
        {
            value = true;
        }
    }

    return value;
}//@CODE_1324


Relation* Class::FindFromRelation(const CbString& rFromName,
                                  const CbString& rToName)
{//@CODE_1529
    FromRelationIterator iRelation(this);
    while (++iRelation)
    {
        if (rFromName == iRelation->GetFromName() &&
            rToName == iRelation->GetToName())
        {
            return iRelation;
        }
    }

    return 0;
}//@CODE_1529


/*@NOTE_1537
Returns the name of the class without prefix.
*/
CbString Class::GetBaseName()
{//@CODE_1537
    int length = GetDataModel()->GetClassPrefix().GetLength();
    if (length && Type::GetName().Left(length) == GetDataModel()->GetClassPrefix())
    {
        return Type::GetName().Mid(length);
    }
    else
    {
        return Type::GetName();
    }
}//@CODE_1537


CbString Class::GetContextList()
{//@CODE_27308
    CbString str;

    if (GetClassGroup())
    {
        ClassGroup::ClassGroupContextIterator 
            iClassGroupContext(GetClassGroup());
        while (++iClassGroupContext)
        {
            if (!str.IsEmpty())
                str += ", ";
            str += iClassGroupContext->GetContextDeclaration()->GetName();
        }
    }
    
    ClassContextIterator iClassContext(this);
    while (++iClassContext)
    {
        if (!str.IsEmpty())
            str += ", ";
        str += iClassContext->GetContextDeclaration()->GetName();
    }
    
    return str;
}//@CODE_27308


CbString Class::GetCppFileWithoutPath()
{//@CODE_11054
    CbString cppFile = _cppFile;

    int index = cppFile.ReverseFind('/');
    if (index != -1)
    {
        cppFile = cppFile.Mid(index+1);
    }
    
    index = cppFile.ReverseFind('\\');
    if (index != -1)
    {
        cppFile = cppFile.Mid(index+1);
    }
    
    return cppFile;
}//@CODE_11054


CbString Class::GetEndContextDeclaration()
{//@CODE_27283
    CbString str;
    
    ClassContextIterator iClassContext(this);
    while (--iClassContext)
    {
        str += iClassContext->GetContextDeclaration()->GetEndContextDeclaration();
    }

    if (GetClassGroup())
    {
        ClassGroup::ClassGroupContextIterator 
            iClassGroupContext(GetClassGroup());
        while (--iClassGroupContext)
        {
            str += iClassGroupContext->GetContextDeclaration()->GetEndContextDeclaration();
        }
    }
    
    return str;
}//@CODE_27283


CbString Class::GetEndContextImplementation()
{//@CODE_27284
    CbString str;
    
    ClassContextIterator iClassContext(this);
    while (--iClassContext)
    {
        str += iClassContext->GetContextDeclaration()->GetEndContextImplementation();
    }

    if (GetClassGroup())
    {
        ClassGroup::ClassGroupContextIterator 
            iClassGroupContext(GetClassGroup());
        while (--iClassGroupContext)
        {
            str += iClassGroupContext->GetContextDeclaration()->GetEndContextImplementation();
        }
    }
    
    return str;
}//@CODE_27284


Context* Class::GetFirstContext()
{//@CODE_27116
    return GetFirstClassContext();
}//@CODE_27116


CbString Class::GetHFileWithoutPath()
{//@CODE_11053
    CbString hFile = _hFile;

    int index = hFile.ReverseFind('/');
    if (index != -1)
    {
        hFile = hFile.Mid(index+1);
    }
    
    index = hFile.ReverseFind('\\');
    if (index != -1)
    {
        hFile = hFile.Mid(index+1);
    }
    
    return hFile;
}//@CODE_11053


/*@NOTE_3345
Returns the name of the class without prefix, with using the new prefix.
*/
CbString Class::GetNewBaseName()
{//@CODE_3345
    int length = GetDataModel()->GetNewClassPrefix().GetLength();
    if (length && Type::GetName().Left(length) == GetDataModel()->GetNewClassPrefix())
    {
        return Type::GetName().Mid(length);
    }
    else
    {
        return Type::GetName();
    }
}//@CODE_3345


Context* Class::GetNextContext(Context* pContextPos)
{//@CODE_27117
    return GetNextClassContext((ClassContext*)pContextPos);
}//@CODE_27117


CbString Class::GetStartContextDeclaration()
{//@CODE_27285
    CbString str;

    if (GetClassGroup())
    {
        ClassGroup::ClassGroupContextIterator 
            iClassGroupContext(GetClassGroup());
        while (++iClassGroupContext)
        {
            str += iClassGroupContext->GetContextDeclaration()->GetStartContextDeclaration();
        }
    }
    
    ClassContextIterator iClassContext(this);
    while (++iClassContext)
    {
        str += iClassContext->GetContextDeclaration()->GetStartContextDeclaration();
    }
    
    return str;
}//@CODE_27285


CbString Class::GetStartContextImplementation()
{//@CODE_27286
    CbString str;

    if (GetClassGroup())
    {
        ClassGroup::ClassGroupContextIterator 
            iClassGroupContext(GetClassGroup());
        while (++iClassGroupContext)
        {
            str += iClassGroupContext->GetContextDeclaration()->GetStartContextImplementation();
        }
    }
    
    ClassContextIterator iClassContext(this);
    while (++iClassContext)
    {
        str += iClassContext->GetContextDeclaration()->GetStartContextImplementation();
    }
    
    return str;
}//@CODE_27286


int Class::HasPureVirtualMethod()
{//@CODE_2033
    BaseClass::MethodIterator iMethod(this, &Method::GetPure);
    while (++iMethod)
    {
        return 1;
    }

    return 0;
}//@CODE_2033


void Class::InitMemberPrefix()
{//@CODE_7427
    _memberPrefix = GetDataModel()->GetMemberPrefix();
}//@CODE_7427


void Class::InitPhase()
{//@CODE_23462
    if (GetDataModel()->GetPhase() > Design_Phase)
    {
        SetPhase(Implementation_Phase);
    }
    else
    {
        SetPhase(GetDataModel()->GetPhase());
    }
    Destructor* pDestructor = dynamic_cast<Destructor*>(GetFirstMethod());
    if (pDestructor)
    {
        pDestructor->SetPhase(GetPhase());
    }
}//@CODE_23462


void Class::MoveFromRelation(Relation* pRelation)
{//@CODE_752
    MoveFromRelationLast(pRelation);
    pRelation->GetToClass()->GetConstructorIncludeMethod()->UpdateArguments();

    FromRelation::MethodIterator method(pRelation->GetFromRelation());
    while (++method)
    {
        Method::MethodShapeIterator iMethodShape(method);
        while (++iMethodShape)
        {
            iMethodShape->Delete();
        }
        method->SaveState(1);
        MoveMethodLast(method);
        method->InitCode();
    }
}//@CODE_752


void Class::MoveToRelation(Relation* pRelation)
{//@CODE_754
    Class* pOldToClass = pRelation->GetToClass();
    MoveToRelationLast(pRelation);
    pOldToClass->GetConstructorIncludeMethod()->UpdateArguments();
    GetConstructorIncludeMethod()->UpdateArguments();

    FromRelation::MethodIterator method(pRelation->GetFromRelation());
    while (++method)
        method->Delete();
}//@CODE_754


void Class::NotifyAddMember(Member* pMember)
{//@CODE_22998
	BaseClass::NotifyAddMember(pMember);

    BaseClass::MethodIterator iMethod(this);
    while (++iMethod)
    {
        iMethod->NotifyAddMember(pMember);
    }
}//@CODE_22998


void Class::NotifyRemoveMember(Member* pMember)
{//@CODE_23000
    BaseClass::MethodIterator iMethod(this);
    while (++iMethod)
    {
        iMethod->NotifyRemoveMember(pMember);
    }
}//@CODE_23000


int Class::OnAddClass(bool checkOnly)
{//@CODE_760
    if (GetClassGroup())
        return GetClassGroup()->OnAddClass(checkOnly);
    else
        return GetDataModel()->OnAddClass(checkOnly);
}//@CODE_760


int Class::OnAddIsClassMethods(bool checkOnly)
{//@CODE_762
    if (!checkOnly)
    {
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowIsClassMethodsDialog(this, nullptr, ownerHwnd);
    }

    return 1;
}//@CODE_762


int Class::OnAddRelation(bool checkOnly)
{//@CODE_761
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Relation* pRelation =
            new Relation(this, this, "", "", 0, 1, 0, 1, 0);

        if (pRelation->GetFromRelation()->OnEditAttributes())
        {
            // Post-dialog: coalesce the relation + its methods being added.
            CbViewLock lock(GetDataModelDoc());
            pRelation->Add();
        }
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_761


int Class::OnDelete(bool checkOnly)
{//@CODE_759
    if (this == GetDataModelDoc()->GetDataModel()->GetDocument())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete the document class", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (this == GetDataModelDoc()->GetDataModel()->GetDocumentObject())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete the document base class", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (BaseClass::GetInheritCount())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete an inherited class", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete class '%s'", GetName());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }

    return 1;
}//@CODE_759


int Class::OnEditAttributes(bool checkOnly)
{//@CODE_758
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;
    if (Qt_ShowClassDialog(this, changed, ownerHwnd))
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
}//@CODE_758


int Class::OnEditContext(bool checkOnly)
{//@CODE_27119
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
}//@CODE_27119


int Class::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_35083
    SequenceDiagram* pSequenceDiagram = dynamic_cast<SequenceDiagram*>(pGti);
    if (pSequenceDiagram && pSequenceDiagram->GetDataModelDoc() == GetDataModelDoc())
    {
        if (!checkOnly)
        {
            SequenceDiagram* pNewSequenceDiagram = 
                new SequenceDiagram(this, pSequenceDiagram);
            pNewSequenceDiagram->Add();
        }
        
        return 1;
    }
        
    ClassDiagram* pClassDiagram = dynamic_cast<ClassDiagram*>(pGti);
    if (pClassDiagram && pClassDiagram->GetDataModelDoc() == GetDataModelDoc())
    {
        if (!checkOnly)
        {
            ClassDiagram* pNewClassDiagram = 
                new ClassDiagram(this, pClassDiagram);
            pNewClassDiagram->Add();
        }
        
        return 1;
    }
        
    return ExternClass::OnPaste(pGti, checkOnly);
}//@CODE_35083


void Class::ReadCppFile(ParseLogInterface* pDialog, bool unconditional)
{//@CODE_746
    extern int ReadCppSource(ParseLogInterface* pDialog, Class* pClass);

    struct _stat buf;
    if (_stat(GetCppFile(), &buf) == 0)
    {
        CbTime time = CbTime(buf.st_mtime);
        if (unconditional || GetDataModel()->GetLastSave() < time)
        {
            ReadCppSource(pDialog, this);

            GetDataModel()->SetMaxLastSave(time);
        }
    }
}//@CODE_746


void Class::ReadHFile(ParseLogInterface* pDialog, bool unconditional)
{//@CODE_748
    extern int ReadHSource(ParseLogInterface* pDialog, Class* pClass);

    struct _stat buf;
    if (_stat(GetHFile(), &buf) == 0)
    {
        CbTime time = CbTime(buf.st_mtime);
        if (unconditional || GetDataModel()->GetLastSave() < time)
        {
            ReadHSource(pDialog, this);

            GetDataModel()->SetMaxLastSave(time);
        }
    }
}//@CODE_748


/*@NOTE_23232
Virtual method to replace strings at various places, called if a type name changes.
*/
void Class::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23232
    BaseClass::ReplaceInX(oldString, newString);
    ReplaceInStr(_hUser1, oldString, newString);
    ReplaceInStr(_hUser2, oldString, newString);
    ReplaceInStr(_hUser3, oldString, newString);
    ReplaceInStr(_cppUser1, oldString, newString);
    ReplaceInStr(_cppUser2, oldString, newString);
    ReplaceInStr(_cppUser3, oldString, newString);
}//@CODE_23232


/*@NOTE_1546
If no name yet give name to constructors & destructors.
Call SetName of its base class.
*/
void Class::SetName(const CbString& rName)
{//@CODE_1546
    if (GetName() != rName)
    {
        ReplaceInStr(_cppHeader, GetName(), rName);
        ReplaceInStr(_hHeader, GetName(), rName);
        
        CbString oldBaseName = GetBaseName();
        BaseClass::SetName(rName);
        CbString newBaseName = GetNewBaseName();
        
        if (oldBaseName != newBaseName)
        {
            ReplaceInStr(_cppHeader, oldBaseName, newBaseName);
            ReplaceInStr(_hHeader, oldBaseName, newBaseName);
        
            IsClassMethodIterator iMethod(this);
            while (++iMethod)
            {
                iMethod->SetName("Is" + newBaseName);
                iMethod->SetNote("Method that returns a non zero value if it is actually a " +
                    rName + " Object." NL);
            }
        }
            
        FromRelationIterator fRel(this);
        while (++fRel)
        {
            if (fRel->GetFromName() == oldBaseName)
            {
                fRel->SaveState();
                fRel->SetFromName(newBaseName);
            }
        }
        
        ToRelationIterator tRel(this);
        while (++tRel)
        {
            if (tRel->GetToName() == oldBaseName)
            {
                tRel->SaveState();
                tRel->SetToName(newBaseName);
            }
        }
    }
    
    FromRelationIterator fRel(this);
    while (++fRel)
    {
        fRel->GetToClass()->GetConstructorIncludeMethod()->UpdateArguments();
    }

    if (GetDocumentObject() && GetDocumentObject()->GetUndoRedo())
    {
        DataModel::ClassIterator iClass(GetDataModel(), &Class::GetSerialize);
        while (++iClass)
        {
            BaseClass::MethodIterator iMethod(iClass);
            while (++iMethod)
            {
                if (iMethod->IsRestoreReferencesMethod() || 
                    iMethod->IsSaveReferencesMethod())
                {
                    Argument* pArgument = iMethod->GetFirstArgument();
                    pArgument->SetName("p" + pArgument->GetType()->GetName());
                    iMethod->SetNote("Bring the current object relations into the same state as " + pArgument->GetName() + ".");
                }
            }
        }
    }
}//@CODE_1546


bool Class::SetTemplate(const CbString& rTemplateDeclaration,
                        const CbString& rTemplate)
{//@CODE_7451
    bool value = BaseClass::SetTemplate(rTemplateDeclaration, rTemplate);
    if (value)
    {   
        Class::FromRelationIterator iFromRelation(this);
        while (++iFromRelation)
        {
            iFromRelation->GetToClass()->SaveState(1);

            if (iFromRelation->GetStatic())
            {
                iFromRelation->SaveState(1);
                iFromRelation->SetStatic(0);
            }

            if (iFromRelation->GetToClass()->SetTemplate(rTemplateDeclaration, rTemplate))
            {
                iFromRelation->GetToClass()->Update();
            }
        }
    
        Class::ToRelationIterator iToRelation(this);
        while (++iToRelation)
        {
            iToRelation->GetFromClass()->SaveState(1);

            if (iToRelation->GetCritical() || iToRelation->GetStatic())
            {
                iToRelation->SaveState(1);
                iToRelation->SetCritical(0);
                iToRelation->SetStatic(0);
            }

            if (iToRelation->GetFromClass()->SetTemplate(rTemplateDeclaration, rTemplate))
            {
                iToRelation->GetFromClass()->Update();
            }
        }
    }
    
    return value;
}//@CODE_7451


bool Class::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40797
    bool show = true;
    if (pTreeViewModel->GetShowOnlyClassesWithoutConstructor())
    {
        if (GetDocument())
        {
            show = false;
        }

        BaseClass::MethodIterator iMethod(this, &Method::IsNormalConstructor);
        while (show && ++iMethod)
        {
            show = false;
        }
    }

    return show && Gti::ShownByFilter(pTreeViewModel);
}//@CODE_40797


/*@NOTE_1527
Sort items alphabetically on their name
*/
int Class::SortOnName(bool checkOnly)
{//@CODE_1527
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortMemberAndMethodGroup(MemberAndMethodGroup::CompareName);

        int i = 0;
        MemberAndMethodGroupIterator memberAndMethodGroup(this);
        while (++memberAndMethodGroup)
        {
            memberAndMethodGroup->SaveState();
            memberAndMethodGroup->SetOrder(i++);
        }
    }

    return 1;
}//@CODE_1527


int Class::SortOnPhase(bool checkOnly)
{//@CODE_23474
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortMemberAndMethodGroup(MemberAndMethodGroup::ComparePhase);

        int i = 0;
        MemberAndMethodGroupIterator memberAndMethodGroup(this);
        while (++memberAndMethodGroup)
        {
            memberAndMethodGroup->SaveState();
            memberAndMethodGroup->SetOrder(i++);
        }
    }

    return GetDataModel()->GetPhaseSupport();
}//@CODE_23474


/*@NOTE_7364
Check if constructors are present and if constructors and destructors are OK.
*/
void Class::SourceCheck(SourceLogInterface* pDialog)
{//@CODE_7364
    CbString str;

    int normalConstructor = 0;
    Class::MethodIterator method(this, &Method::IsNonMacroMethod);
    while (++method)
    {
        if (method->IsConstructor())
        {
            if (method->IsNormalConstructor())
            {
                normalConstructor++;
            }
            if (GetFromRelationCount() || GetToRelationCount())
            {
                // Make sure code is there if not initialise it at default
                (void)method->GetCode();
                
                if (method->IsNormalConstructor())
                {
                    if (method->FindStringInStrippedCode("ConstructorInclude") == -1)
                    {
                        str.Format("  ! Error: No call to '%s' in constructor '%s'", 
                            GetConstructorIncludeMethod()->GetItemText(), 
                            method->GetItemText());
                        pDialog->AddLogError(str);
                    }
                }
                if (method->IsSerializeConstructor())
                {
                    if (method->FindStringInStrippedCode("SerializeConstructorInclude") == -1)
                    {
                        str.Format("  ! Error: No call to 'SerializeConstructorInclude()' in constructor '%s'", 
                            method->GetItemText());
                        pDialog->AddLogError(str);
                    }
                }
                if (method->IsReplaceConstructor())
                {
                    if (method->FindStringInStrippedCode("ReplaceConstructorInclude") == -1)
                    {
                        str.Format("  ! Error: No call to 'ReplaceConstructorInclude()' in constructor '%s'", 
                            method->GetItemText());
                        pDialog->AddLogError(str);
                    }
                }
            }
        }
        
        if (method->IsDestructor() && (GetFromRelationCount() || GetToRelationCount()))
        {
            // Make sure code is there if not initialise it at default
            (void)method->GetCode();
            
            if (method->FindStringInStrippedCode("DestructorInclude") == -1)
            {
                str.Format("  ! Error: No call to 'DestructorInclude()' in destructor  '%s'",
                    method->GetItemText());
                pDialog->AddLogError(str);
            }
        }
    }
    
    // No normal constructor detected and it isn't the datamodel top of a serialize project
    if (!normalConstructor && !GetDocument())
    {
        if (!GetFromRelationCount() && !GetToRelationCount())
        {
            str.Format("  ! Warning: No constructor present for class '%s'", GetName());
            pDialog->AddLogWarning(str);
        }
        else
        {
            str.Format("  ! Error: No constructor present for class '%s'", GetName());
            pDialog->AddLogError(str);
        }
    }
}//@CODE_7364


void Class::Update()
{//@CODE_757
    if (GetAdded())
    {
        if ((GetClassGroup() && GetParent() != GetClassGroup()) ||
            (!GetClassGroup() && GetParent() != GetDataModel()))
        {
            Remove();
            Add();
        }

        FromRelationIterator fromRel(this);
        while (++fromRel)
        {
            if (fromRel->GetFromRelation())
                fromRel->GetFromRelation()->Update();
            if (fromRel->GetToRelation())
                fromRel->GetToRelation()->Update();
        }

        ToRelationIterator toRel(this);
        while (++toRel)
        {
            if (toRel->GetFromRelation())
                toRel->GetFromRelation()->Update();
            if (toRel->GetToRelation())
                toRel->GetToRelation()->Update();
        }

        IsClassMethodIterator iMethod(this);
        while (++iMethod)
            iMethod->Update();

        BaseClass::Update();

        if (GetDllExport() && GetDataModel()->GetShowDllExport())
        {
            SetItemText("AFX_EXT_CLASS " + GetName());
            Gti::Update();
        }
    }
}//@CODE_757


void Class::UpdateCppHeader()
{//@CODE_735
    if (_cppHeader.IsEmpty())
    {
        _cppHeader = GetDataModel()->GetCppHeader();

        CbString left;
        CbString right;
        int index;
        while ((index = _cppHeader.Find("@INSERT_DATE")) != -1)
        {
            left = _cppHeader.Left(index);
            right = _cppHeader.Mid(index + 12);
            CbTime time = CbTime::GetCurrentTime();
            _cppHeader = left + time.Format( "%B %d, %Y %H:%M" ) + right;
        }
        while ((index = _cppHeader.Find("@INSERT_FILENAME")) != -1)
        {
            left = _cppHeader.Left(index);
            right = _cppHeader.Mid(index + 16);
            _cppHeader = left + _cppFile + right;
        }
        while ((index = _cppHeader.Find("@INSERT_CLASSNAME")) != -1)
        {
            left = _cppHeader.Left(index);
            right = _cppHeader.Mid(index + 17);
            _cppHeader = left + GetName() + right;
        }
    }
    CbString deleted;
    CbString added;
    CbString updated;

    while (!_modified.IsEmpty())
    {
        int index = _modified.ReverseFind('@');
        if (_modified[index+1] == 'D')
            deleted += _modified.Mid(index);
        else if (_modified[index+1] == 'A')
            added += _modified.Mid(index);
        else if (_modified[index+1] == 'U')
            updated += _modified.Mid(index);

        _modified = _modified.Left(index);
    }

    CbString str;
    MemberIterator member(this);
    while (++member)
    {
        if (member->GetVersion() > GetDataModelDoc()->GetVersion())
        {
            if (member->GetVersion() == member->GetInitialVersion())
            {
                str.Format("@Added member '%s'", member->GetPrefixedName());
                added += str;
            }
            else
            {
                str.Format("@Updated member '%s'", member->GetPrefixedName());
                updated += str;
            }
        }
    }

    InheritIterator inherit(this);
    while (++inherit)
    {
        if (inherit->GetVersion() > GetDataModelDoc()->GetVersion())
        {
            if (inherit->GetVersion() == inherit->GetInitialVersion())
            {
                str.Format("@Added inheritance '%s'", inherit->GetBaseName());
                added += str;
            }
            else
            {
                str.Format("@Updated inheritance '%s'", inherit->GetBaseName());
                updated += str;
            }
        }
    }

    FromRelationIterator fromRelation(this);
    while (++fromRelation)
    {
        int version = fromRelation->GetFromRelation()->GetVersion();
        if (version > GetDataModelDoc()->GetVersion())
        {
            if (version == fromRelation->GetFromRelation()->GetInitialVersion())
            {
                str.Format("@Added relation '%s'", fromRelation->GetNotation());
                added += str;
            }
            else
            {
                str.Format("@Updated relation '%s'", fromRelation->GetNotation());
                updated += str;
            }
        }
    }

    ToRelationIterator toRelation(this);
    while (++toRelation)
    {
        int version = toRelation->GetToRelation()->GetVersion();
        if (version > GetDataModelDoc()->GetVersion())
        {
            if (toRelation->GetFromClass() != toRelation->GetToClass())
            {
                if (version == toRelation->GetToRelation()->GetInitialVersion())
                {
                    str.Format("@Added relation '%s'", toRelation->GetNotation());
                    added += str;
                }
                else
                {
                    str.Format("@Updated relation '%s'", toRelation->GetNotation());
                    updated += str;
                }
            }
        }
    }

    MethodIterator method(this, &Method::IsNonMacroMethod);
    while (++method)
    {
        if (method->GetVersion() > GetDataModelDoc()->GetVersion())
        {
            if (method->GetVersion() == method->GetInitialVersion())
            {
                str.Format("@Added method '%s'", method->GetName());
                added += str;
            }
            else
            {
                bool codeChanged = true;
                Method::ArgumentIterator argument(method);
                while (++argument)
                {
                    if (argument->GetVersion() > GetDataModelDoc()->GetVersion())
                    {
                        str.Format("@Updated interface of method '%s'", 
                            method->GetName());
                        updated += str;
                        codeChanged = false;
                        break;
                    }
                }

                if (codeChanged)
                {
                    str.Format("@Updated code of method '%s'", 
                        method->GetName());
                    updated += str;
                }
            }
        }
    }

    if (!deleted.IsEmpty() || !added.IsEmpty() || !updated.IsEmpty())
    {
        CbTime time = CbTime::GetCurrentTime();
        CbString dateAuthor = time.Format("%B %d, %Y %H:%M ") + GetDataModel()->GetAuthor();

        int index;
        if ((index = _cppHeader.Find("@INSERT_MODIFICATIONS")) != -1)
        {
            index += 21;

            CbString lineStart;
            if (_cppHeader[index] == '(')
            {
                for (index += 1; _cppHeader[index] != ')'; index++)
                    lineStart += _cppHeader[index];
            }
            while (_cppHeader[index] != '\n')
                index++;

            CbString left = _cppHeader.Left(++index);
            CbString right = _cppHeader.Mid(index);

            _cppHeader = left;
            _cppHeader += lineStart + dateAuthor + NL;
        
            while (!deleted.IsEmpty())
            {
                int index = deleted.ReverseFind('@');
                deleted.SetAt(index, ' ');
                _cppHeader += lineStart + "   " + deleted.Mid(index) + NL;

                deleted = deleted.Left(index);
            }

            while (!added.IsEmpty())
            {
                int index = added.ReverseFind('@');
                added.SetAt(index, ' ');
                _cppHeader += lineStart + "   " + added.Mid(index) + NL;

                added = added.Left(index);
            }

            while (!updated.IsEmpty())
            {
                int index = updated.ReverseFind('@');
                updated.SetAt(index, ' ');
                _cppHeader += lineStart + "   " + updated.Mid(index) + NL;

                updated = updated.Left(index);
            }

            _cppHeader += right;
        }
    }

    _modified.Empty();
}//@CODE_735


int Class::UpdateHHeader()
{//@CODE_736
    int val = 0;
    if (_hHeader.IsEmpty())
    {
        val = 1;
        _hHeader = GetDataModel()->GetHHeader();

        CbString left;
        CbString right;
        int index;
        while ((index = _hHeader.Find("@INSERT_DATE")) != -1)
        {
            left = _hHeader.Left(index);
            right = _hHeader.Mid(index + 12);
            CbTime time = CbTime::GetCurrentTime();
            _hHeader = left + time.Format( "%B %d, %Y %H:%M" ) + right;
        }
        while ((index = _hHeader.Find("@INSERT_FILENAME")) != -1)
        {
            left = _hHeader.Left(index);
            right = _hHeader.Mid(index + 16);
            _hHeader = left + _hFile + right;
        }
        while ((index = _hHeader.Find("@INSERT_CLASSNAME")) != -1)
        {
            left = _hHeader.Left(index);
            right = _hHeader.Mid(index + 17);
            _hHeader = left + GetName() + right;
        }
    }

    CbString deleted;
    CbString added;
    CbString updated;
    CbString modified = _modified;

    while (!modified.IsEmpty())
    {
        int index = modified.ReverseFind('@');
        if (modified[index+1] == 'D')
            deleted += modified.Mid(index);
        else if (modified[index+1] == 'A')
            added += modified.Mid(index);
        else if (modified[index+1] == 'U')
            updated += modified.Mid(index);

        modified = modified.Left(index);
    }

    CbString str;
    MemberIterator member(this);
    while (++member)
    {
        if (member->GetVersion() > GetDataModelDoc()->GetVersion())
        {
            if (member->GetVersion() == member->GetInitialVersion())
            {
                str.Format("@Added member '%s'", member->GetPrefixedName());
                added += str;
            }
            else
            {
                str.Format("@Updated member '%s'", member->GetPrefixedName());
                updated += str;
            }
        }
    }

    InheritIterator inherit(this);
    while (++inherit)
    {
        if (inherit->GetVersion() > GetDataModelDoc()->GetVersion())
        {
            if (inherit->GetVersion() == inherit->GetInitialVersion())
            {
                str.Format("@Added inheritance '%s'", inherit->GetBaseName());
                added += str;
            }
            else
            {
                str.Format("@Updated inheritance '%s'", inherit->GetBaseName());
                updated += str;
            }
        }
    }

    FromRelationIterator fromRelation(this);
    while (++fromRelation)
    {
        int version = fromRelation->GetFromRelation()->GetVersion();
        if (version > GetDataModelDoc()->GetVersion())
        {
            if (version == fromRelation->GetFromRelation()->GetInitialVersion())
            {
                str.Format("@Added relation '%s'", fromRelation->GetNotation());
                added += str;
            }
            else
            {
                str.Format("@Updated relation '%s'", fromRelation->GetNotation());
                updated += str;
            }
        }
    }

    ToRelationIterator toRelation(this);
    while (++toRelation)
    {
        int version = toRelation->GetToRelation()->GetVersion();
        if (version > GetDataModelDoc()->GetVersion())
        {
            if (toRelation->GetFromClass() != toRelation->GetToClass())
            {
                if (version == toRelation->GetToRelation()->GetInitialVersion())
                {
                    str.Format("@Added relation '%s'", toRelation->GetNotation());
                    added += str;
                }
                else
                {
                    str.Format("@Updated relation '%s'", toRelation->GetNotation());
                    updated += str;
                }
            }
        }
    }

    MethodIterator method(this, &Method::IsNonMacroMethod);
    while (++method)
    {
        if (method->GetVersion() > GetDataModelDoc()->GetVersion())
        {
            if (method->GetVersion() == method->GetInitialVersion())
            {
                str.Format("@Added method '%s'", method->GetName());
                added += str;
            }
            else
            {
                Method::ArgumentIterator argument(method);
                while (++argument)
                {
                    if (argument->GetVersion() > GetDataModelDoc()->GetVersion())
                    {
                        str.Format("@Updated interface of method '%s'", 
                            method->GetName());
                        updated += str;
                        break;
                    }
                }
            }
        }
    }

    if (!deleted.IsEmpty() || !added.IsEmpty() || !updated.IsEmpty())
    {
        val = 1;
        CbTime time = CbTime::GetCurrentTime();
        CbString dateAuthor = time.Format("%B %d, %Y %H:%M ") + GetDataModel()->GetAuthor();

        int index;
        if ((index = _hHeader.Find("@INSERT_MODIFICATIONS")) != -1)
        {
            index += 21;

            CbString lineStart;
            if (_hHeader[index] == '(')
            {
                for (index += 1; _hHeader[index] != ')'; index++)
                    lineStart += _hHeader[index];
            }
            while (_hHeader[index] != '\n')
                index++;

            CbString left = _hHeader.Left(++index);
            CbString right = _hHeader.Mid(index);

            _hHeader = left;
            _hHeader += lineStart + dateAuthor + NL;
        
            while (!deleted.IsEmpty())
            {
                int index = deleted.ReverseFind('@');
                deleted.SetAt(index, ' ');
                _hHeader += lineStart + "   " + deleted.Mid(index) + NL;

                deleted = deleted.Left(index);
            }

            while (!added.IsEmpty())
            {
                int index = added.ReverseFind('@');
                added.SetAt(index, ' ');
                _hHeader += lineStart + "   " + added.Mid(index) + NL;

                added = added.Left(index);
            }

            while (!updated.IsEmpty())
            {
                int index = updated.ReverseFind('@');
                updated.SetAt(index, ' ');
                _hHeader += lineStart + "   " + updated.Mid(index) + NL;

                updated = updated.Left(index);
            }

            _hHeader += right;
        }
    }

    return val;
}//@CODE_736


void Class::WriteCppFile(SourceLogInterface* pDialog, bool unconditional)
{//@CODE_741
    bool write = false;
    
    if (GetTemplate().IsEmpty() || !GetDataModel()->GetTemplateClassHeaderOnly())
    {
        if (GetFromRelationCount() || GetToRelationCount())
        {
            write = true;
        }
        
        MemberIterator iMember(this, &Member::GetStatic);
        while (!write && ++iMember)
        {
            write = true;
        }
        
        MethodIterator iMethod(this);
        while (!write && ++iMethod)
        {
            if (!iMethod->GetInline() && iMethod->GetImplement() &&
                !iMethod->GetDelete())
            {
                write = true;
            }
        }
    }
    
    // If cpp file isn't needed, skip it 
    if (!write)
        return;
    
    // Set initial buffer size at 128Kbyte, with 16K grow
    static CbStringBuilder newContent(128*1024, 16*1024);
    
    // Empty string, while keeping alloc size.
    newContent.Empty();
    WriteCppFileBody(newContent);
    
    bool changed = true;
    
    if (!unconditional)
    {
        ifstream is(GetCppFile(), ios::in | ios::binary);
        if (is)
        {
            const int offset = _cppHeader.GetLength() + 2;
            is.seekg(0, ios::end);
            const long size = (long)is.tellg() - offset;
            if (size > 0 && size == (long)newContent.GetLength())
            {
                // Skip the rewrite when the existing body is byte-identical
                // (keeps the file timestamp stable). Portable replacement for
                // the old Win32 file-mapping compare.
                char* oldContent = new char[size];
                is.seekg(offset, ios::beg);
                is.read(oldContent, size);
                if (is.gcount() == size && strncmp(newContent, oldContent, (size_t)size) == 0)
                {
                    changed = false;
                }
                delete[] oldContent;
            }
        }
    }
    
    if (changed)
    {
        ofstream os(GetCppFile(), ios::out|ios::binary);
        if (os)
        {
            CbString str;
            str.Format("- Writing file '%s'", GetCppFile());
            pDialog->AddLog(str);
            
            UpdateCppHeader();
            os << _cppHeader << NL;
            os << newContent;
            
            os.close();
            struct _stat buf;
            if (_stat(GetCppFile(), &buf) == 0)
            {
                CbTime time = CbTime(buf.st_mtime);
                if (GetDataModel()->GetLastSave() < time)
                {
                    GetDataModel()->SetLastSave(time);
                }
            }
        }
        else
        {
            CbString str;
            str.Format("! Error can not open file '%s'", GetCppFile());
            pDialog->AddLogError(str);
        }
    }
}//@CODE_741


/*@NOTE_7356
Write the body of the cpp file (all exect the header) to the string 'str'.
*/
void Class::WriteCppFileBody(CbStringBuilder& str)
{//@CODE_7356
    str += "//@START_USER1" NL;
    str += GetCppUser1();
    str += "//@END_USER1" NL NL NL;
    
    str += GetStartContextImplementation();
    str += "// Master include file" NL;
    if (GetDataModel()->GetStdAfx())
        str += "#include \"StdAfx.h\"" NL NL NL;
    else
        str += "#include \"" + GetDataModel()->GetHFileWithoutPath() + "\"" NL NL NL;
    
    str += "//@START_USER2" NL;
    str += GetCppUser2();
    str += "//@END_USER2" NL NL NL;
    
    if (!GetDataModel()->GetNamespace().IsEmpty())
    {
        str += "using namespace " + GetDataModel()->GetNamespace() + ";" NL NL NL;
    }
    
    CbString templateDeclaration;
    if (!GetTemplate().IsEmpty())
    {
        templateDeclaration = GetStrippedTemplateDeclaration() + NL;
    }

    str += "// Static members" NL;
    MemberIterator member(this, &Member::GetStatic);
    while (++member)
    {
        str += member->GetStartContextImplementation();
        str += templateDeclaration;
        str += member->GetTypeName() + GetName() + "::" 
            + GetMemberPrefix() + member->GetVariableName();
        if (member->GetInitialization().IsEmpty())
            str += ";" NL;
        else
        {
            /*
            if (member->GetPointer() || member->GetReference() || member->GetArray())
            {
                str += " = " + member->GetInitialization() + ";" NL;
            }
            else
            {
                str += "(" + member->GetInitialization() + ");" NL;
            }
            */
            str += " = " + member->GetInitialization() + ";" NL;
        }
        str += member->GetEndContextImplementation();
    }
    if (GetDocumentObject() || GetDocument())
    {
        str += templateDeclaration;
        str += "int " + GetName() + "::_objectVersion = 0;" NL;
    }
    str += NL NL;

    bool firstFixedMethod = true;
    Class::MethodIterator method(this, &Method::IsNonMacroMethod);
    while (++method)
    {
        if (method->GetImplement() && !method->GetInline() &&
            !method->GetDelete())
        {
            if (firstFixedMethod && method->IsFixed())
            {
                firstFixedMethod = false;
                str += "//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!" NL NL;
            }
            
            if (!method->GetNote().IsEmpty()) //!! Comment this line out for empty Note
                str += "/*@NOTE_" + method->GetIdAsString() + NL + method->GetNote() + "*/" NL;
            
            str += method->GetStartContextImplementation();
            str += templateDeclaration;
            str += WrapArguments(method->GetInterfaceCpp()) + NL;
            
            if (method->IsConstructor())
                str += ((Constructor*)((Method*)method))->GetInit();
            
            if (method->IsFixed())
                str += "{" NL;
            else
                str += "{//" "@CODE_" + method->GetIdAsString() + NL;
            
            str += method->GetCode();
            
            if (method->IsFixed())
                str += "}" NL;
            else
                str += "}//" "@CODE_" + method->GetIdAsString() + NL;
            str += method->GetEndContextImplementation();

            str += NL NL;
        }
    }

    if (GetSerialize())
    {
        str += "// ClassBuilder macro to support serialization for this class" NL;
        if (!HasPureVirtualMethod())
        {
            str += "CB_IMPLEMENT_SERIAL(" + GetName() + ")" NL NL NL;
        }
    }

    str += "// Methods for the relation(s) of the class" NL;
    CbString macro;
    FromRelationIterator fromRel(this);
    while (++fromRel)
    {
        macro.Empty();
        fromRel->WriteFromMacro(macro, "METHODS_");
        if (fromRel->GetMulti())
        {
            if (fromRel->GetFilter())
                fromRel->WriteFromMacro(macro, "METHODS_ITERATOR_", 0);
            else
                fromRel->WriteFromMacro(macro, "METHODS_ITERATOR_NOFILTER_", 0);
        }

        str += macro;
    }

    ToRelationIterator toRel(this);
    while (++toRel)
    {
        if (!(toRel->GetSingle() && 
              toRel->GetFromClass() == toRel->GetToClass() &&
              toRel->GetFromName() == toRel->GetToName()))
        {
            macro.Empty();
            toRel->WriteToMacro(macro, "METHODS_");
            str += macro;
        }
    }
    str += NL "//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!" NL;
    str += GetEndContextImplementation();
    str += NL "//@START_USER3" NL;
    str += GetCppUser3();
}//@CODE_7356


int Class::WriteHFile(SourceLogInterface* pDialog, bool unconditional)
{//@CODE_743
    // Set initial buffer size at 16Kbyte
    static CbStringBuilder newContent(1<<14);

    // Empty string, while keeping alloc size.
    newContent.Empty();
    WriteHFileBody(newContent);

    bool changed = true;

    if (!unconditional)
    {
        ifstream is(GetHFile(), ios::in | ios::binary);
        if (is)
        {
            const int offset = _hHeader.GetLength() + 2;
            is.seekg(0, ios::end);
            const long size = (long)is.tellg() - offset;
            if (size > 0 && size == (long)newContent.GetLength())
            {
                // Skip the rewrite when the existing body is byte-identical
                // (keeps the file timestamp stable). Portable replacement for
                // the old Win32 file-mapping compare.
                char* oldContent = new char[size];
                is.seekg(offset, ios::beg);
                is.read(oldContent, size);
                if (is.gcount() == size && strncmp(newContent, oldContent, (size_t)size) == 0)
                {
                    changed = false;
                }
                delete[] oldContent;
            }
        }
    }

    if (changed)
    {
        ofstream os(GetHFile(), ios::out|ios::binary);
        if (os)
        {
            CbString str;
            str.Format("- Writing file '%s'", GetHFile());
            pDialog->AddLog(str);

            SourceCheck(pDialog);
        
            UpdateHHeader();
            os << _hHeader << NL;
            os << newContent;

            os.close();
            struct _stat buf;
            if (_stat(GetHFile(), &buf) == 0)
            {
                CbTime time = CbTime(buf.st_mtime);
                if (GetDataModel()->GetLastSave() < time)
                {
                    GetDataModel()->SetLastSave(time);
                }
            }
        }
        else
        {
            CbString str;
            str.Format("! Error can not open file '%s'", GetHFile());
            pDialog->AddLogError(str);
        }

        return 1;
    }
    else
        return 0;
}//@CODE_743


/*@NOTE_7352
Write the body of the include file (all exect the header) to the string 'str'.
*/
void Class::WriteHFileBody(CbStringBuilder& str)
{//@CODE_7352
    CbString def = "_" + GetHFileWithoutPath();
    def.MakeUpper();
    int index;
    while ((index = def.Find('.')) != -1)
        def.SetAt(index, '_');
    
    str += "#ifndef " + def + NL;
    str += "#define " + def + NL;
    str += GetStartContextDeclaration();
    str += NL;
    
    def += "_INLINES";

    str += "//@START_USER1" NL;
    str += GetHUser1();
    str += "//@END_USER1" NL NL NL;
    
    if (!GetNote().IsEmpty())
        str += "/*@NOTE_" + GetIdAsString() + NL + GetNote() + "*/" NL;
    
    CbString templateDeclaration;
    if (!GetTemplate().IsEmpty())
    {
        templateDeclaration = GetStrippedTemplateDeclaration() + NL;
    }
    str += GetTemplateDeclaration() + NL;
    
    if (GetStruct())
        str += "struct ";
    else
        str += "class ";
    
    if (GetDllExport())
    {
        str += "AFX_EXT_CLASS ";
    }
    
    str += Type::GetName() + NL;
    if (GetInheritCount())
    {
        InheritIterator inherit(this);
        while (++inherit)
        {
            if (inherit.IsFirst())
                str += GetIndent() + ": ";
            else
                str += GetIndent() + ", ";
            
            switch (inherit->GetAccess())
            {
            case PUBLIC:str += "public ";
                break;
                
            case PROTECTED:str += "protected ";
                break;
                
            case PRIVATE:str += "private ";
                break;
                
            default:break;
            }
            
            
            if (inherit->GetVirtual())
                str += "virtual ";
            
            str += inherit->GetBaseClass()->Type::GetName() + inherit->GetTemplate() + NL;
        }
    }
    
    str += "{" NL;
    if (GetSerialize())
    {
        if (!HasPureVirtualMethod())
        {
            str += GetIndent() + "CB_DECLARE_SERIAL(" + GetName() + ")" NL;
        }
    }
    
    if (!_relationMacrosLast)
    {
        CbString macro;
        FromRelationIterator fromRel(this);
        while (++fromRel)
        {
            macro.Empty();
            fromRel->WriteFromMacro(macro, GetIndent() + "RELATION_");
            str += macro;
        }
        
        ToRelationIterator toRel(this);
        while (++toRel)
        {
            if (!(toRel->GetSingle() && 
                toRel->GetFromClass() == toRel->GetToClass() &&
                toRel->GetFromName() == toRel->GetToName()))
            {
                macro.Empty();
                toRel->WriteToMacro(macro, GetIndent() + "RELATION_");
                str += macro;
            }
        }
    }
    
    str += NL "//@START_USER2" NL;
    str += GetHUser2();
    str += "//@END_USER2" NL;
    
    if (GetDocumentObject())
    {
        str += NL + GetIndent() + "friend class " + GetDataModel()->GetDocument()->GetName()
            + ";" NL;
    }
    
    str += NL "// Members";
    str += NL "private:" NL;
    MemberIterator privateMember(this, &Member::IsPrivate);
    while (++privateMember)
    {
        str += privateMember->GetStartContextDeclaration();
        str += GetIndent() + privateMember->GetItemText() + ";" NL;
        str += privateMember->GetEndContextDeclaration();
    }
    
    str += NL "protected:" NL;
    if (GetDocumentObject() || GetDocument())
    {
        str += GetIndent() + "static int _objectVersion;" NL;
    }
    MemberIterator protectedMember(this, &Member::IsProtected);
    while (++protectedMember)
    {
        str += protectedMember->GetStartContextDeclaration();
        str += GetIndent() + protectedMember->GetItemText() + ";" NL;
        str += protectedMember->GetEndContextDeclaration();
    }
    
    str += NL "public:" NL;
    if (GetDocumentObject())
    {
        str += GetIndent() + "union {" NL;
        str += GetIndent() + "    int      _index;" NL;
        str += GetIndent() + "    intptr_t _ptrIndex;" NL;
        str += GetIndent() + "};" NL;
    }
    MemberIterator publicMember(this, &Member::IsPublic);
    while (++publicMember)
    {
        str += publicMember->GetStartContextDeclaration();
        str += GetIndent() + publicMember->GetItemText() + ";" NL;
        str += publicMember->GetEndContextDeclaration();
    }
    
    str += NL "// Methods";
    str += NL "private:" NL;
    MethodIterator privateMethod(this, &Method::IsNonMacroPrivateMethod);
    while (++privateMethod)
    {
        str += privateMethod->GetStartContextDeclaration();
        if (privateMethod->GetDeclare())
        {
            str += WrapArguments(GetIndent() + privateMethod->GetItemText()) + ";" NL;
        }
        str += privateMethod->GetEndContextDeclaration();
    }
    
    str += NL "protected:" NL;
    MethodIterator protectedMethod(this, &Method::IsNonMacroProtectedMethod);
    while (++protectedMethod)
    {
        str += protectedMethod->GetStartContextDeclaration();
        if (protectedMethod->GetDeclare())
        {
            str += WrapArguments(GetIndent() + protectedMethod->GetItemText()) + ";" NL;
        }
        str += protectedMethod->GetEndContextDeclaration();
    }
    
    str += NL "public:" NL;
    MethodIterator publicMethod(this, &Method::IsNonMacroPublicMethod);
    while (++publicMethod)
    {
        str += publicMethod->GetStartContextDeclaration();
        if (publicMethod->GetDeclare())
        {
            str += WrapArguments(GetIndent() + publicMethod->GetItemText()) + ";" NL;
        }
        str += publicMethod->GetEndContextDeclaration();
    }
    
    if (_relationMacrosLast)
    {
        str += NL;
        CbString macro;
        FromRelationIterator fromRel(this);
        while (++fromRel)
        {
            macro.Empty();
            fromRel->WriteFromMacro(macro, GetIndent() + "RELATION_");
            str += macro;
        }
        
        ToRelationIterator toRel(this);
        while (++toRel)
        {
            if (!(toRel->GetSingle() && 
                toRel->GetFromClass() == toRel->GetToClass() &&
                toRel->GetFromName() == toRel->GetToName()))
            {
                macro.Empty();
                toRel->WriteToMacro(macro, GetIndent() + "RELATION_");
                str += macro;
            }
        }
    }
    
    str += "};" NL NL;
    str += GetEndContextDeclaration();
    str += "#endif" NL NL NL;
    
    str += "#ifdef CB_INLINES"  NL;
    str += "#ifndef " + def + NL;
    str += "#define " + def + NL;
    str += GetStartContextImplementation();
    str += NL;

    bool templateInHeader = false;
    if (!templateDeclaration.IsEmpty() && GetDataModel()->GetTemplateClassHeaderOnly())
    {
        templateInHeader = true;

        str += "// Static members" NL;
        MemberIterator member(this, &Member::GetStatic);
        while (++member)
        {
            str += member->GetStartContextImplementation();
            str += templateDeclaration;
            str += member->GetTypeName() + GetName() + "::" 
                + GetMemberPrefix() + member->GetVariableName();
            if (member->GetInitialization().IsEmpty())
                str += ";" NL;
            else
            {
                /*
                if (member->GetPointer() || member->GetReference() || member->GetArray())
                {
                    str += " = " + member->GetInitialization() + ";" NL;
                }
                else
                {
                    str += "(" + member->GetInitialization() + ");" NL;
                }
                */
                str += " = " + member->GetInitialization() + ";" NL;
            }
            str += member->GetEndContextImplementation();
        }
        str += NL NL;
    }
    
    Class::MethodIterator method(this, &Method::IsNonMacroMethod);
    while (++method)
    {
        if ((templateInHeader || method->GetInline()) && method->GetImplement() &&
            !method->GetDelete())
        {
            if (!method->GetNote().IsEmpty()) //!! Comment this line out for empty Note
                str += "/*@NOTE_" + method->GetIdAsString() + NL + method->GetNote() + "*/" NL;
            
            str += method->GetStartContextImplementation();
            str += templateDeclaration;
            if (method->GetInline())
            {
                str += WrapArguments("inline " + method->GetInterfaceCpp()) + NL;
            }
            else
            {
                str += WrapArguments(method->GetInterfaceCpp()) + NL;
            }
            
            if (method->IsConstructor())
                str += ((Constructor*)((Method*)method))->GetInit();
            
            if (method->IsFixed())
                str += "{" NL;
            else
                str += "{//" "@CODE_" + method->GetIdAsString() + NL;
            
            str += method->GetCode();
            
            if (method->IsFixed())
                str += "}" NL;
            else
                str += "}//" "@CODE_" + method->GetIdAsString() + NL;
            str += method->GetEndContextImplementation();
            
            str += NL NL NL;
        }
    }
    
    str += "//@START_USER3" NL;
    str += GetHUser3();
    str += "//@END_USER3" NL NL;
    
    str += GetEndContextImplementation();
    str += "#endif" NL;
    str += "#endif" NL;
}//@CODE_7352


void Class::WriteRecursiveInclude(CbString& str)
{//@CODE_750
    if (GetFlag())
    {
        if (GetFlag() == 2)
        {
            CbString msg;
            msg.Format("Recursive include file relation for class '%s'", GetName());
            CbMessageBox(msg, CBMB_ICONEXCLAMATION);
            SetFlag(1);
        }

        return;
    }

    SetFlag(2);
    InheritIterator inherit(this);
    while (++inherit)
    {
        Class* pClass = dynamic_cast<Class*>(inherit->GetBaseClass());
        if (pClass)
            pClass->WriteRecursiveInclude(str);
    }
    
    FromRelationIterator relation(this, &Relation::GetFilter);
    while (++relation)
    {
        if (relation->GetToClass() != this)
            relation->GetToClass()->WriteRecursiveInclude(str);
    }

    BaseClass::MemberIterator iMember(this);
    while (++iMember)
    {
        Class* pClass = dynamic_cast<Class*>(iMember->GetType());
        if (pClass && !iMember->GetPointer() && !iMember->GetReference() && 
			!iMember->GetStatic())
		{
            pClass->WriteRecursiveInclude(str);
		}
    }
    
    str += "#include \"" + GetHFileWithoutPath() + "\"" NL;
    SetFlag(1);
}//@CODE_750


const CbString& Class::GetCppFile()
{//@CODE_1089
    return _cppFile;
}//@CODE_1089


void Class::SetCppFile(const CbString& rCppFile)
{//@CODE_1090
    _cppFile = rCppFile;
}//@CODE_1090


const CbString& Class::GetCppHeader()
{//@CODE_1092
    return _cppHeader;
}//@CODE_1092


void Class::SetCppHeader(const CbString& rCppHeader)
{//@CODE_1093
    _cppHeader = rCppHeader;
}//@CODE_1093


const CbString& Class::GetCppUser1()
{//@CODE_1095
    return _cppUser1;
}//@CODE_1095


void Class::SetCppUser1(const CbString& rCppUser1)
{//@CODE_1096
    if (_cppUser1 != rCppUser1)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        _cppUser1 = rCppUser1;
        if (!rCppUser1.IsEmpty())
        {
            if (rCppUser1[rCppUser1.GetLength()-1] != '\n')
                _cppUser1 += NL;
        }
    }
}//@CODE_1096


const CbString& Class::GetCppUser2()
{//@CODE_1098
    return _cppUser2;
}//@CODE_1098


void Class::SetCppUser2(const CbString& rCppUser2)
{//@CODE_1099
    if (_cppUser2 != rCppUser2)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        _cppUser2 = rCppUser2;
        if (!rCppUser2.IsEmpty())
        {
            if (rCppUser2[rCppUser2.GetLength()-1] != '\n')
                _cppUser2 += NL;
        }
    }
}//@CODE_1099


/*@NOTE_3339
Returns the value of member '_cppUser3'.
*/
const CbString& Class::GetCppUser3()
{//@CODE_3339
    return _cppUser3;
}//@CODE_3339


/*@NOTE_3340
Set the value of member '_cppUser3' to 'rCppUser3'.
*/
void Class::SetCppUser3(const CbString& rCppUser3)
{//@CODE_3340
    if (_cppUser3 != rCppUser3)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        _cppUser3 = rCppUser3;
        if (!rCppUser3.IsEmpty())
        {
            if (rCppUser3[rCppUser3.GetLength()-1] != '\n')
                _cppUser3 += NL;
        }
    }
}//@CODE_3340


/*@NOTE_1539
Returns the value of member '_dllExport'.
*/
bool Class::GetDllExport()
{//@CODE_1539
    return _dllExport;
}//@CODE_1539


/*@NOTE_1540
Set the value of member '_dllExport' to 'dllExport'.
*/
void Class::SetDllExport(bool dllExport)
{//@CODE_1540
    _dllExport = dllExport;
}//@CODE_1540


int Class::GetFlag()
{//@CODE_1086
    return _flag;
}//@CODE_1086


void Class::SetFlag(int flag)
{//@CODE_1087
    _flag = flag;
}//@CODE_1087


const CbString& Class::GetHFile()
{//@CODE_1101
    return _hFile;
}//@CODE_1101


void Class::SetHFile(const CbString& rHFile)
{//@CODE_1102
    _hFile = rHFile;
}//@CODE_1102


const CbString& Class::GetHHeader()
{//@CODE_1104
    return _hHeader;
}//@CODE_1104


void Class::SetHHeader(const CbString& rHHeader)
{//@CODE_1105
    _hHeader = rHHeader;
}//@CODE_1105


const CbString& Class::GetHUser1()
{//@CODE_1107
    return _hUser1;
}//@CODE_1107


void Class::SetHUser1(const CbString& rHUser1)
{//@CODE_1108
    if (_hUser1 != rHUser1)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        _hUser1 = rHUser1;
        if (!rHUser1.IsEmpty())
        {
            if (rHUser1[rHUser1.GetLength()-1] != '\n')
                _hUser1 += NL;
        }
    }
}//@CODE_1108


const CbString& Class::GetHUser2()
{//@CODE_1110
    return _hUser2;
}//@CODE_1110


void Class::SetHUser2(const CbString& rHUser2)
{//@CODE_1111
    if (_hUser2 != rHUser2)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        _hUser2 = rHUser2;
        if (!rHUser2.IsEmpty())
        {
            if (rHUser2[rHUser2.GetLength()-1] != '\n')
                _hUser2 += NL;
        }
    }
}//@CODE_1111


/*@NOTE_6120
Returns the value of member '_hUser3'.
*/
const CbString& Class::GetHUser3() const
{//@CODE_6120
    return _hUser3;
}//@CODE_6120


/*@NOTE_6121
Set the value of member '_hUser3' to 'rHUser3'.
*/
void Class::SetHUser3(const CbString& rHUser3)
{//@CODE_6121
    if (_hUser3 != rHUser3)
    {
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        _hUser3 = rHUser3;
        if (!rHUser3.IsEmpty())
        {
            if (rHUser3[rHUser3.GetLength()-1] != '\n')
                _hUser3 += NL;
        }
    }
}//@CODE_6121


const CbString& Class::GetModified()
{//@CODE_1113
    return _modified;
}//@CODE_1113


void Class::SetModified(const CbString& rModified)
{//@CODE_1114
    _modified = rModified;
}//@CODE_1114


const CbString& Class::GetNote()
{//@CODE_1116
    return _note;
}//@CODE_1116


void Class::SetNote(const CbString& rNote)
{//@CODE_1117
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_1117


bool Class::GetReplace() const
{//@CODE_1119
    return _replace;
}//@CODE_1119


void Class::SetReplace(bool replace)
{//@CODE_1120
    if (_replace != replace)
    {
        _replace = replace;

        if (replace)
        {
            ReplaceConstructor* pReplaceConstructor = new ReplaceConstructor(this);
            if (GetAdded())
            {
                pReplaceConstructor->Add();
                pReplaceConstructor->GetReplaceConstructorIncludeMethod()->Add();
            }
        }
        else
        {
            BaseClass::MethodIterator method(this, &Method::IsReplaceConstructor);
            while (++method)
                method->Delete();
        }
    }
}//@CODE_1120


/*@NOTE_1820
Returns the value of member '_serialize'.
*/
bool Class::GetSerialize() const
{//@CODE_1820
    return _serialize;
}//@CODE_1820


/*@NOTE_1821
Set the value of member '_serialize' to 'serialize'.
*/
void Class::SetSerialize(bool serialize)
{//@CODE_1821
    if (_serialize != serialize)
    {
        if (serialize)
        {
            (void)new SerializeConstructor(this);

            Class* docObject = GetDataModel()->GetDocumentObject();
            Inherit* pInherit = 0;
            
            // Delete all inheritances expect the first one, which lead
            // to the document object class.
            ExternClass::InheritIterator iInherit(this);
            while (++iInherit)
            {
                ExternClass* pExternClass = 
                    dynamic_cast<ExternClass*>(iInherit->GetBaseClass());
                if (!pInherit && pExternClass && 
                    pExternClass->IsBaseClass(docObject))
                {
                    pInherit = iInherit;
                }
                else
                {
                    iInherit->Delete();
                }
            }

            // No suited inheritance, so make one
            if (!pInherit)
            {
                pInherit = new Inherit(this, docObject);
                if (GetAdded())
                {
                    pInherit->Add();
                }
            }

            if (GetAdded())
            {
                MemberAndMethodGroup* pMemberAndMethodGroup = 0;
                Class::MethodIterator iMethod(this);
                while (++iMethod)
                {
                    if (iMethod->IsConstructorIncludeMethod() ||
                        iMethod->IsDestructorIncludeMethod())
                    {
                        pMemberAndMethodGroup = iMethod->GetMemberAndMethodGroup();
                        break;
                    }
                }

                iMethod.Reset();
                while (++iMethod)
                {
                    if (!iMethod->GetAdded())
                    {
                        if (pMemberAndMethodGroup)
                        {
                            pMemberAndMethodGroup->AddMethodLast(iMethod);
                        }
                        iMethod->Add();
                    }
                }
            }
        }
        else if (GetDataModel()->GetSerialize())
        {
            if (GetFirstInherit())
                GetFirstInherit()->Delete();
            
            Class::MethodIterator iMethod(this);
            while (++iMethod)
            {
                if (iMethod->IsSerializeConstructor() ||
                    iMethod->IsSerializeMethod() ||
                    iMethod->IsSerializeRelationsMethod() ||
                    iMethod->IsCleanupReferencesMethod() ||
                    iMethod->IsRemoveReferencesMethod() ||
                    iMethod->IsRestoreReferencesMethod() ||
                    iMethod->IsSaveReferencesMethod())
                {
                    iMethod->Delete();
                }
            }
        }

        _serialize = serialize;
    }
}//@CODE_1821


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5282
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Class::CleanupReferences()
{
    ExternClass::CleanupReferences();
    CLEANUP_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    CLEANUP_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    CLEANUP_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    CLEANUP_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
}


/*@NOTE_123
Method which must be called first in a constructor
*/
void Class::ConstructorInclude(DataModel* pDataModel)
{
    INIT_MULTI_OWNED_ACTIVE(Class, FromClass, Relation, FromRelation)
    INIT_MULTI_OWNED_ACTIVE(Class, ToClass, Relation, ToRelation)
    INIT_SINGLE_OWNED_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
    INIT_MULTI_OWNED_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
    INIT_MULTI_OWNED_ACTIVE(Class, Class, ClassContext, ClassContext)
    INIT_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    INIT_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    INIT_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    INIT_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
}


/*@NOTE_125
Method which must be called first in a destructor
*/
void Class::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Class, FromClass, Relation, FromRelation)
    EXIT_MULTI_OWNED_ACTIVE(Class, ToClass, Relation, ToRelation)
    EXIT_SINGLE_OWNED_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
    EXIT_MULTI_OWNED_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
    EXIT_MULTI_OWNED_ACTIVE(Class, Class, ClassContext, ClassContext)
    EXIT_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    EXIT_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    EXIT_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    EXIT_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
}


/*@NOTE_5283
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Class::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(Class, Class, ClassContext, ClassContext)
    REMOVE_MULTI_OWNED_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
    REMOVE_SINGLE_OWNED_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
    REMOVE_MULTI_OWNED_ACTIVE(Class, ToClass, Relation, ToRelation)
    REMOVE_MULTI_OWNED_ACTIVE(Class, FromClass, Relation, FromRelation)
    ExternClass::RemoveReferences();
    REMOVE_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
    REMOVE_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    REMOVE_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    REMOVE_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
}


/*@NOTE_5284
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Class::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Class* pClass = (Class*)pDataModelDocObject;
    RESTORE_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    RESTORE_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    RESTORE_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    RESTORE_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
    ExternClass::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5286
Save the state of the current object relations to pDataModelDocObject.
*/
void Class::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ExternClass::SaveReferences(pDataModelDocObject);
    Class* pClass = (Class*)pDataModelDocObject;
    SAVE_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    SAVE_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    SAVE_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    SAVE_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
}


/*@NOTE_128
Serialize the members only to a CbObject object
*/
void Class::Serialize(CbArchive& archive)
{
    ExternClass::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _cppFile;
        archive << _cppHeader;
        archive << _cppUser1;
        archive << _cppUser2;
        archive << _hFile;
        archive << _hHeader;
        archive << _hUser1;
        archive << _hUser2;
        archive << _modified;
        archive << _note;
        archive << _replace;
        archive << _dllExport;
        archive << _serialize;
        archive << _cppUser3;
        archive << _hUser3;
        archive << _relationMacrosLast;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _cppFile;
            archive >> _cppHeader;
            archive >> _cppUser1;
            archive >> _cppUser2;
            archive >> _hFile;
            archive >> _hHeader;
            archive >> _hUser1;
            archive >> _hUser2;
            archive >> _modified;
            archive >> _note;
            archive >> _replace;
            archive >> _dllExport;
            archive >> _serialize;
            archive >> _cppUser3;
            archive >> _hUser3;
            archive >> _relationMacrosLast;
        }
    }
}


/*@NOTE_127
Method which must be called first in a serialize constructor
*/
void Class::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Class, FromClass, Relation, FromRelation)
    INIT_MULTI_ACTIVE(Class, ToClass, Relation, ToRelation)
    INIT_SINGLE_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
    INIT_MULTI_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
    INIT_MULTI_ACTIVE(Class, Class, ClassContext, ClassContext)
    INIT_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    INIT_MULTI_PASSIVE(DataModel, DataModel, Class, Class)
    INIT_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    INIT_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)
}


/*@NOTE_130
Serialize the relations to a CbObject object
*/
void Class::SerializeRelations(CbArchive& archive,
                               DataModelDocObject* pointerArray[])
{
    ExternClass::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Class, FromClass, Relation, FromRelation)
        WRITE_MULTI_ACTIVE(Class, ToClass, Relation, ToRelation)
        WRITE_SINGLE_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
        WRITE_MULTI_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
        WRITE_MULTI_ACTIVE(Class, Class, ClassContext, ClassContext)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Class, FromClass, Relation, FromRelation)
            READ_MULTI_ACTIVE(Class, ToClass, Relation, ToRelation)
            READ_SINGLE_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
            READ_MULTI_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
            READ_MULTI_ACTIVE(Class, Class, ClassContext, ClassContext)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Class)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Class, FromClass, Relation, FromRelation)
METHODS_ITERATOR_MULTI_ACTIVE(Class, FromClass, Relation, FromRelation)
METHODS_MULTI_OWNED_ACTIVE(Class, ToClass, Relation, ToRelation)
METHODS_ITERATOR_MULTI_ACTIVE(Class, ToClass, Relation, ToRelation)
METHODS_SINGLE_OWNED_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
METHODS_MULTI_OWNED_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
METHODS_ITERATOR_MULTI_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
METHODS_MULTI_OWNED_ACTIVE(Class, Class, ClassContext, ClassContext)
METHODS_ITERATOR_MULTI_ACTIVE(Class, Class, ClassContext, ClassContext)
METHODS_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
METHODS_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
METHODS_SINGLE_PASSIVE(DataModel, Document, Class, Document)
METHODS_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
