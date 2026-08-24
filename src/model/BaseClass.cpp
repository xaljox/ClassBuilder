/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          BaseClass.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'BaseClass'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
#include "ClassBuilderDoc.h"
//@END_USER2


// Static members


BaseClass::BaseClass(DataModelDoc* pDataModelDoc) //@INIT_710
    : Type(pDataModelDoc)
    , _struct(0)
    , _memberPrefix("")
    , _suppressForwardDeclaration(false)
{//@CODE_710
    ConstructorInclude(pDataModelDoc);

    // Put in your own code 
}//@CODE_710


/*@NOTE_1564
Constructor needed for putting a new object in the old one's context
*/
BaseClass::BaseClass(BaseClass* pOld) //@INIT_1564
    : Type(pOld)
{//@CODE_1564
    ReplaceConstructorInclude(pOld);

    _struct = pOld->_struct;
    _memberPrefix = pOld->_memberPrefix;
    _templateDeclaration = pOld->_templateDeclaration;
    _template = pOld->_template;
    _suppressForwardDeclaration = pOld->_suppressForwardDeclaration;

    // Put in your own code
}//@CODE_1564


BaseClass::BaseClass(OtherType* pOld) //@INIT_7505
    : Type(pOld)
    , _struct(0)
    , _memberPrefix("")
    , _templateDeclaration()
    , _template()
    , _suppressForwardDeclaration(false)
{//@CODE_7505
    ConstructorInclude(Type::GetDataModelDoc());

    // Put in your own code
}//@CODE_7505


/*@NOTE_7543
Constructor method needed to copy BaseClass from one project to the other.
*/
BaseClass::BaseClass(DataModelDoc* pDataModelDoc,
                     BaseClass* pBaseClass) //@INIT_7543
    : Type(pDataModelDoc, pBaseClass)
    , _struct(pBaseClass->_struct)
    , _memberPrefix(pBaseClass->_memberPrefix)
    , _templateDeclaration(pBaseClass->_templateDeclaration)
    , _template(pBaseClass->_template)
    , _suppressForwardDeclaration(false)
{//@CODE_7543
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
    MemberIterator iMember(pBaseClass);
    while (++iMember)
    {
        if (!iMember->IsPrivate())
        {
            (void)new Member(this, GetDataModelDoc()->FindOrDuplicateType(iMember->GetType()), iMember);
        }
    }

    MethodIterator iMethod(pBaseClass);
    while (++iMethod)
    {
        if (!iMethod->IsPrivateMethod())
        {
            Method* pMethod = 0;
            Constructor* pConstructor = dynamic_cast<Constructor*>(iMethod.Get());
            Destructor* pDestructor = dynamic_cast<Destructor*>(iMethod.Get());
            if(pConstructor)
            {
                pMethod = new Constructor(this, pConstructor);
            }
            else if (pDestructor)
            {
                pMethod = new Destructor(this, pDestructor);
            }
            else
            {
                pMethod = new Method(this, GetDataModelDoc()->FindOrDuplicateType(iMethod->GetType()), iMethod);
            }
            
            pMethod->SetCode("");
        }
    }
}//@CODE_7543


/*@NOTE_100
Constructor needed for serialization, not meant to use for other purposes!
*/
BaseClass::BaseClass() //@INIT_100
    : Type()
    , _struct(0)
    , _suppressForwardDeclaration(false)
{//@CODE_100
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_100


/*@NOTE_98
Destructor method
*/
BaseClass::~BaseClass()
{//@CODE_98
    DestructorInclude();

    // Put in your own code
}//@CODE_98


void BaseClass::Add()
{//@CODE_714
    if (!GetAdded())
    {
        SaveState(1);
        GetDataModelDoc()->GetExternClasses()->AddChildLast(this);

        SetIcon(ICON_EXTERNCLASS);
        SetItemText(GetName());

        Gti::Add();

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
}//@CODE_714


CbString BaseClass::DefineTemplate()
{//@CODE_7466
    CbString value;
    CbString templateDefine = GetTemplateDefine();
    
    if (GetTemplateDefine() != GetTemplate())
    {
        DataModelDoc::BaseClassIterator iBaseClass(GetDataModelDoc());
        while (++iBaseClass && iBaseClass.Get() != this)
        {
            if (templateDefine == iBaseClass->GetTemplateDefine())
            {
                return value;
            }
        }
        
        value = "#define" + templateDefine + "\t" + GetTemplate() + NL;
    }
    
    return value;
}//@CODE_7466


bool BaseClass::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_4127
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = true;
    }
    else
    {
        ExternClass* pExternClass = dynamic_cast<ExternClass*>(this);
        
        if (pExternClass)
        {
            pGtiDropDefault = GetDataModelDoc()->GetExternClasses();
    
            Remove();
            value = true;
        }
    }

    return value;
}//@CODE_4127


void BaseClass::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_4121
    if (ctrlKeyDown)
    {
        ClassDiagram* pClassDiagram = dynamic_cast<ClassDiagram*>(pGtiDrop);
        SequenceDiagram* pSequenceDiagram = dynamic_cast<SequenceDiagram*>(pGtiDrop);
        if (pSequenceDiagram)
        {
            int lastRight = 80;
            if (pSequenceDiagram->GetLastLifeLineShape())
            {
                lastRight = pSequenceDiagram->GetLastLifeLineShape()->GetRect().right;
            }
            CbPoint point(lastRight + 20, 0);
            Shape::Round(point);
            (void)new ClassLifeLineShape(pSequenceDiagram, this, point);
        }
        else if (pClassDiagram)
        {
            bool stop = false;
            int x;
            int y = 200;

            while (!stop && y < pClassDiagram->GetHeight())
            {
                x = 100;
                while (!stop && x < pClassDiagram->GetWidth())
                {
                    stop = true;
                    CbRect rect(x, -(y+250), x+250, -y);
                    ClassDiagram::ClassDiagramShapeIterator 
                        iClassDiagramShape(pClassDiagram, &ClassDiagramShape::IsClassShape);
                    while (++iClassDiagramShape)
                    {
                        CbRect tmpRect;
                        if (tmpRect.IntersectRect(rect, iClassDiagramShape->GetRect()))
                        {
                            stop = false;
                            break;
                        }
                    }

                    if (!stop)
                    {
                        x += 500;
                    }
                }

                if (!stop)
                {
                    y += 600;
                }
            }

            if (!stop)
            {
                x = y = 0;
            }

            CbPoint point(x, -y);
            (void)new ClassShape(pClassDiagram, this, point);

            // CD-only: dropping a class onto the diagram adds a ClassShape (the
            // class is already in the tree), so a CD-canvas repaint suffices --
            // NotifyStructureChanged would needlessly rebuild the whole tree.
            GetDataModelDoc()->NotifyCdViews();
        }
        else
            Type::Drop(ctrlKeyDown, pGtiDrop);
    }
    else
    {
        OtherTypes* pOtherTypes = dynamic_cast<OtherTypes*>(pGtiDrop);
        ExternClass* pExternClass = dynamic_cast<ExternClass*>(this);
        
        if (pOtherTypes && pExternClass)
        {
            CbString str;
            str.Format("Are you sure you want to degrade extern class '%s' into an other type, this action can not be undone!!", 
                GetName().c_str());
            if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_OKCANCEL) == CBMB_IDOK)
            {
                OtherType* pOtherType = new OtherType(pExternClass);
                pOtherType->Add();
                pOtherType->DataModelDocObject::GetDataModelDoc()->DeleteAllUndoBase();
                pOtherType->DataModelDocObject::GetDataModelDoc()->DeleteAllRedoBase();
            }
            else
            {
                Add();
                GetDataModelDoc()->RollBack();
            }
        }
        else
        {
            Add();
            GetDataModelDoc()->RollBack();
        }
    }
}//@CODE_4121


bool BaseClass::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_4124
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        if (pGtiDrop->IsClassDiagram() || pGtiDrop->IsSequenceDiagram())
        {
            value = true;
        }
        else
            return Type::DropTarget(ctrlKeyDown, pGtiDrop);
    }
    else
    {
        if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
            return value;

        if (pGtiDrop->IsOtherTypes() || pGtiDrop->IsExternClasses())
        {
            value = true;
        }
    }

    return value;
}//@CODE_4124


ClassShape* BaseClass::FindClassShape(ClassDiagram* pClassDiagram,
                                      ClassShape* startAfterClassShape)
{//@CODE_4115
    ClassShapeIterator iClassShape(this, 0, startAfterClassShape);
    while (++iClassShape)
    {
        if (pClassDiagram == iClassShape->GetClassDiagram())
        {
            return iClassShape;
        }
    }

    return 0;
}//@CODE_4115


Member* BaseClass::FindMember(const CbString& rName)
{//@CODE_3283
    MemberIterator iMember(this);
    while (++iMember)
    {
        if (rName == iMember->GetName())
        {
            return iMember;
        }
    }

    return 0;
}//@CODE_3283


Method* BaseClass::FindMethodWithId(unsigned int id)
{//@CODE_1278
    MethodIterator iMethod(this);
    while (++iMethod)
    {
        if (id == iMethod->GetId())
        {
            return iMethod;
        }
    }

    return 0;
}//@CODE_1278


Method* BaseClass::FindMethodWithName(const CbString& name)
{//@CODE_23035
    MethodIterator iMethod(this);
    while (++iMethod)
    {
        if (name == iMethod->GetName())
        {
            return iMethod;
        }
    }

    return 0;
}//@CODE_23035


Method* BaseClass::FindSimilarMethod(Method* pMethod)
{//@CODE_3342
    MethodIterator iMethod(this);
    while (++iMethod)
    {
        if (!iMethod->IsFixed() && iMethod->IsSimilar(pMethod))
        {
            return iMethod;
        }
    }

    return 0;
}//@CODE_3342


/*@NOTE_7438
Returns the value of member '_name'.
*/
CbString BaseClass::GetName()
{//@CODE_7438
    return Type::GetName() + GetTemplate();
}//@CODE_7438


CbString BaseClass::GetStrippedTemplateDeclaration()
{//@CODE_35829
    CbString strippedTemplateDeclaration = _templateDeclaration;
    StripTemplateDeclaration(strippedTemplateDeclaration);

    return DataModel::ConvertToHtmlStringIfNeeded(strippedTemplateDeclaration);
}//@CODE_35829


CbString BaseClass::GetTemplateDefine()
{//@CODE_7460
    CbString value = GetTemplate();

    if (value.Find(',') != -1)
    {
        value.Replace(',', '_');
        value.Replace("*", "_PTR_");
        value.Replace("&", "_REF_");
        value.Remove('<');
        value.Remove('>');
        value.Remove(' ');
        value.TrimLeft();
        value.TrimRight();
        value.MakeUpper();
        value = " _CBT_" + value;
    }

    return value;
}//@CODE_7460


int BaseClass::IsInheritBase(ExternClass* pExternClass)
{//@CODE_712
    if (pExternClass == this)
        return 0;

    ExternClass::InheritIterator iClassInherit(pExternClass);
    while (++iClassInherit)
    {
        if (this == iClassInherit->GetBaseClass())
            return 0;
    }

    ExternClass* pBaseClass = dynamic_cast<ExternClass*>(this);
    if (pBaseClass)
    {
        Class* pDocumentObject = 
            pExternClass->GetDataModelDoc()->GetDataModel()->GetDocumentObject();
        ExternClass::InheritIterator inherit(pBaseClass);
        while (++inherit)
        {
            // Class, which inherit from the DocumentObject are allowed
            if (inherit->GetBaseClass() != pDocumentObject)
            {
                if (!inherit->GetBaseClass()->IsInheritBase(pExternClass))
                    return 0;
            }
        }
    }

    return 1;
}//@CODE_712


void BaseClass::MoveMember(Member* pMember)
{//@CODE_3286
    Class* pClass = dynamic_cast<Class*>(pMember->GetBaseClass());

    pMember->SaveState(1);
    int version = GetDataModelDoc()->GetVersion();
    Member::MethodIterator method(pMember);
    while (++method)
    {
        if (pClass && method->GetVersion() <= version)
        {
            pClass->SaveState();
            pClass->SetVersion(version + 1);

            CbString str;
            str.Format("@Deleted method '%s'", method->GetName().c_str());
    
            pClass->AddModified(str);
        }

        Method::MethodShapeIterator iMethodShape(method);
        while (++iMethodShape)
        {
            iMethodShape->Delete();
        }
        method->SaveState(1);
        MoveMethodLast(method);

        method->SetInitialVersion(version + 1);
        method->SetVersion(version + 1);
    }

    if (pClass && pMember->GetVersion() <= version)
    {
        pClass->SaveState();
        pClass->SetVersion(version + 1);

        CbString str;
        str.Format("@Deleted member '%s'", pMember->GetPrefixedName().c_str());

        pClass->AddModified(str);
    }

    Member::MemberShapeIterator iMemberShape(pMember);
    while (++iMemberShape)
    {
        iMemberShape->Delete();
    }
    pMember->SaveState(1);
    MoveMemberLast(pMember);

    pMember->SetInitialVersion(version + 1);
    pMember->SetVersion(version + 1);

    Member::MemberArgumentIterator memberArgument(pMember);
    while (++memberArgument)
    {
        Method* pMethod = memberArgument->GetMethod();
        if (!pMethod->IsMemberMethod())
        {
            FindMethod* pFindMethod = dynamic_cast<FindMethod*>(pMethod);
            if (pFindMethod)
            {
                Class* toClass = 
                    pFindMethod->GetFromRelation()->GetRelation()->GetToClass();
                if (!(toClass == this || toClass->IsBaseClass(this)))
                {
                    // The member isn't visable on the new spot, so delete & Update
                    memberArgument->Delete();
                    pMethod->Update();
                }
            }
            else
            {
                memberArgument->Delete();
                pMethod->Update();
            }
        }
    }
}//@CODE_3286


void BaseClass::NotifyAddMember(Member* pMember)
{//@CODE_22990
    ClassShapeIterator iClassShape(this);
    while (++iClassShape)
    {
        iClassShape->NotifyAddMember(pMember);
    }

	if (pMember->GetGetMemberMethod())
	{
		NotifyAddMethod(pMember->GetGetMemberMethod());
	}

	if (pMember->GetSetMemberMethod())
	{
		NotifyAddMethod(pMember->GetSetMemberMethod());
	}
}//@CODE_22990


void BaseClass::NotifyAddMethod(Method* pMethod)
{//@CODE_22991
    ClassShapeIterator iClassShape(this);
    while (++iClassShape)
    {
        iClassShape->NotifyAddMethod(pMethod);
    }
}//@CODE_22991


void BaseClass::NotifyRemoveMember(Member* pMember)
{//@CODE_22992
}//@CODE_22992


void BaseClass::NotifyRemoveMethod(Method* pMethod)
{//@CODE_22993
}//@CODE_22993


int BaseClass::OnAddClassDiagram(bool checkOnly)
{//@CODE_3898
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        ClassDiagram* pClassDiagram = new ClassDiagram(this);

        if (pClassDiagram->OnEditAttributes())
            pClassDiagram->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_3898


int BaseClass::OnAddConstructor(bool checkOnly)
{//@CODE_717
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Constructor* pConstructor = new Constructor(this);
        pConstructor->CreateArguments();

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
}//@CODE_717


int BaseClass::OnAddGroup(bool checkOnly)
{//@CODE_3334
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        MemberAndMethodGroup* pMemberAndMethodGroup = new MemberAndMethodGroup(this);
        // Make it the last in the tree view
        pMemberAndMethodGroup->SetOrder(GetMemberAndMethodGroupCount()-1); 
    
        if (pMemberAndMethodGroup->OnEditAttributes())
            pMemberAndMethodGroup->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_3334


int BaseClass::OnAddMember(bool checkOnly)
{//@CODE_3290
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Member* pMember = new Member(this, GetDataModelDoc()->FindType(""));
        
        if (pMember->OnEditAttributes())
        {
            pMember->Add();
            NotifyAddMember(pMember);
        }
        else
            GetDataModelDoc()->RollBack();
    }
    
    return 1;
}//@CODE_3290


int BaseClass::OnAddMethod(bool checkOnly)
{//@CODE_716
    if (!checkOnly)
    {
        MethodIterator iMethod(this);
        while(--iMethod)
        {
            if (!iMethod->IsConstructor() &&
                !iMethod->IsDestructor() &&
                !iMethod->IsFixed() &&
                !iMethod->IsMemberMethod() &&
                !iMethod->IsFromRelationMethod())
            {
                break;
            }
        }
        GetDataModelDoc()->MarkLastUndo();
        Method* pMethod = new Method(this, GetDataModelDoc()->FindType(""));

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

        if (pMethod->OnEditAttributes())
        {
            pMethod->Add();
            NotifyAddMethod(pMethod);
        }
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_716


int BaseClass::OnAddSequenceDiagram(bool checkOnly)
{//@CODE_30479
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        SequenceDiagram* pSequenceDiagram = new SequenceDiagram(this);

        if (pSequenceDiagram->OnEditAttributes())
            pSequenceDiagram->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_30479


/*@NOTE_23041
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void BaseClass::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_23041
	Type::OnUndoRedoChanged(pOldState);

    BaseClass* pBaseClass = (BaseClass*)pOldState;

    if (pBaseClass && pBaseClass->GetMemberPrefix() != GetMemberPrefix())
    {
        Update();
    }
}//@CODE_23041


void BaseClass::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23063
    if (ReplaceInStr(_template, oldString, newString))
    {
        Update();
    }
}//@CODE_23063


/*@NOTE_35387
If no name yet give name to constructors & destructors.
Call SetName of its base class.
*/
void BaseClass::SetName(const CbString& rName)
{//@CODE_35387
    if (GetName() != rName)
    {
        MethodIterator method(this);
        while (++method)
        {
            if (method->IsConstructor())
                method->SetName(rName);
            else if (method->IsDestructor())
                method->SetName("~" + rName);
        }
        
        Type::SetName(rName);
    }
}//@CODE_35387


bool BaseClass::SetTemplate(const CbString& rTemplateDeclaration,
                            const CbString& rTemplate)
{//@CODE_7450
    bool value = false;
    if (_templateDeclaration != rTemplateDeclaration || _template != rTemplate)
    {
        SetTemplateDeclaration(rTemplateDeclaration);
        SetTemplate(rTemplate);
        value = true;
    }
    
    return value;
}//@CODE_7450


void BaseClass::StripTemplateDeclaration(CbString& templateDeclaration)
{//@CODE_35830
    TRACE("Strip: %s ->", templateDeclaration);
    int start = templateDeclaration.Find("=");
    while (start != -1)
    {
        CbString left = templateDeclaration.Left(start);
        left.TrimRight();

        templateDeclaration = templateDeclaration.Mid(start);

        int stop = templateDeclaration.Find(",");
        if (stop == -1)
        {
            stop = templateDeclaration.Find(">");
        }

        if (stop != -1)
        {
            templateDeclaration = left + templateDeclaration.Mid(stop);
        }
        else
        {
            templateDeclaration = left;
        }

        start = templateDeclaration.Find("=");
    }

    TRACE(" %s\n", templateDeclaration);
}//@CODE_35830


void BaseClass::Update()
{//@CODE_715
    if (GetAdded())
    {
        SetItemText(GetName());

        Gti::Update();

        InheritIterator iInherit(this);
        while (++iInherit)
            iInherit->Update();

        Type::VariableIterator iVariable(this);
        while (++iVariable)
            iVariable->Update();

        Type::ExceptionSpecificationTypeIterator iExceptionSpecificationType(this);
        while (++iExceptionSpecificationType)
            iExceptionSpecificationType->GetExceptionSpecification()->GetMethod()->Update();

        MemberIterator iMember(this);
        while (++iMember)
            iMember->Update();

        MethodIterator iMethod(this);
        while (++iMethod)
        {
            if (iMethod->IsConstructor() || iMethod->IsDestructor())
                iMethod->Update();
        }
    }
}//@CODE_715


/*@NOTE_7423
Returns the value of member '_memberPrefix'.
*/
const CbString& BaseClass::GetMemberPrefix() const
{//@CODE_7423
    return _memberPrefix;
}//@CODE_7423


/*@NOTE_7424
Set the value of member '_memberPrefix' to 'rMemberPrefix'.
*/
void BaseClass::SetMemberPrefix(const CbString& rMemberPrefix)
{//@CODE_7424
    if (rMemberPrefix != _memberPrefix)
    {
        // Do modification on code of methods first
        CbString oldMemberPrefix = _memberPrefix;
        _memberPrefix = rMemberPrefix;
        
        BaseClass::MemberIterator member(this);
        while (++member)
        {
            CbString oldString = oldMemberPrefix + member->GetName();
            CbString newString = rMemberPrefix + member->GetName();

            DataModel::ClassIterator iClass(GetDataModelDoc()->GetDataModel());
            while (++iClass)
            {
                BaseClass::MethodIterator method(iClass, &Method::IsNonMacroMethod);
                while (++method)
                {
                    method->ReplaceInCode(oldString, newString);
                }
            }
        }
    }
}//@CODE_7424


/*@NOTE_4920
Returns the value of member '_struct'.
*/
bool BaseClass::GetStruct()
{//@CODE_4920
    return _struct;
}//@CODE_4920


/*@NOTE_7448
Returns the value of member '_template'.
*/
CbString BaseClass::GetTemplate()
{//@CODE_7448
	return DataModel::ConvertToHtmlStringIfNeeded(_template);
}//@CODE_7448


/*@NOTE_7436
Set the value of member '_template' to 'rTemplate'.
*/
void BaseClass::SetTemplate(const CbString& rTemplate)
{//@CODE_7436
    if (_template != rTemplate)
    {
        CbString oldTemplate = _template;
        _template = rTemplate;
        
        InheritIterator iInherit(this);
        while (++iInherit)
        {
            if (iInherit->GetTemplate() == oldTemplate)
            {
                iInherit->SaveState(1);
                iInherit->SetTemplate(rTemplate);
            }
        }
        
        Type::VariableIterator iVariable(this);
        while (++iVariable)
        {
            if (iVariable->GetTemplate() == oldTemplate)
            {
                iVariable->SaveState(1);
                iVariable->SetTemplate(rTemplate);
            }
        }
        
        Type::ExceptionSpecificationTypeIterator iExceptionSpecificationType(this);
        while (++iExceptionSpecificationType)
        {
            if (iExceptionSpecificationType->GetTemplate() == oldTemplate)
            {
                iExceptionSpecificationType->SaveState(1);
                iExceptionSpecificationType->SetTemplate(rTemplate);
            }
        }
        
        ClassLifeLineShapeIterator iClassLifeLineShape(this);
        while (++iClassLifeLineShape)
        {
            if (iClassLifeLineShape->GetTemplate() == oldTemplate)
            {
                iClassLifeLineShape->SaveState(1);
                iClassLifeLineShape->SetTemplate(rTemplate);
            }
        }
    }
}//@CODE_7436


/*@NOTE_7431
Returns the value of member '_templateDeclaration'.
*/
CbString BaseClass::GetTemplateDeclaration()
{//@CODE_7431
    return DataModel::ConvertToHtmlStringIfNeeded(_templateDeclaration);
}//@CODE_7431


/*@NOTE_7432
Set the value of member '_templateDeclaration' to 'rTemplateDeclaration'.
*/
void BaseClass::SetTemplateDeclaration(const CbString& rTemplateDeclaration)
{//@CODE_7432
    _templateDeclaration = rTemplateDeclaration;
}//@CODE_7432


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5276
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void BaseClass::CleanupReferences()
{
    Type::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_97
Method which must be called first in a constructor
*/
void BaseClass::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
    INIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Method, Method)
    INIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Member, Member)
    INIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    INIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    INIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_99
Method which must be called first in a destructor
*/
void BaseClass::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
    EXIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Method, Method)
    EXIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Member, Member)
    EXIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    EXIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    EXIT_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_5277
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void BaseClass::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    REMOVE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    REMOVE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    REMOVE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Member, Member)
    REMOVE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Method, Method)
    REMOVE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
    Type::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_1566
Method which must be called first in a replace constructor
*/
void BaseClass::ReplaceConstructorInclude(BaseClass* pOld)
{
    REPLACE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
    REPLACE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Method, Method)
    REPLACE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Member, Member)
    REPLACE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    REPLACE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    REPLACE_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    REPLACE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_5278
Bring the current object relations into the same state as pDataModelDocObject.
*/
void BaseClass::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    BaseClass* pBaseClass = (BaseClass*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
    Type::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5280
Save the state of the current object relations to pDataModelDocObject.
*/
void BaseClass::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Type::SaveReferences(pDataModelDocObject);
    BaseClass* pBaseClass = (BaseClass*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_102
Serialize the members only to a CbObject object
*/
void BaseClass::Serialize(CbArchive& archive)
{
    Type::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _struct;
        archive << _memberPrefix;
        archive << _templateDeclaration;
        archive << _template;
        archive << _suppressForwardDeclaration;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _struct;
            archive >> _memberPrefix;
            archive >> _templateDeclaration;
            archive >> _template;
            archive >> _suppressForwardDeclaration;
        }
    }
}


/*@NOTE_101
Method which must be called first in a serialize constructor
*/
void BaseClass::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
    INIT_MULTI_ACTIVE(BaseClass, BaseClass, Method, Method)
    INIT_MULTI_ACTIVE(BaseClass, BaseClass, Member, Member)
    INIT_MULTI_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    INIT_MULTI_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    INIT_MULTI_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
}


/*@NOTE_104
Serialize the relations to a CbObject object
*/
void BaseClass::SerializeRelations(CbArchive& archive,
                                   DataModelDocObject* pointerArray[])
{
    Type::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
        WRITE_MULTI_ACTIVE(BaseClass, BaseClass, Method, Method)
        WRITE_MULTI_ACTIVE(BaseClass, BaseClass, Member, Member)
        WRITE_MULTI_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
        WRITE_MULTI_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
        WRITE_MULTI_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
            READ_MULTI_ACTIVE(BaseClass, BaseClass, Method, Method)
            READ_MULTI_ACTIVE(BaseClass, BaseClass, Member, Member)
            READ_MULTI_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
            READ_MULTI_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
            READ_MULTI_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(BaseClass)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
METHODS_ITERATOR_MULTI_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
METHODS_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Method, Method)
METHODS_ITERATOR_MULTI_ACTIVE(BaseClass, BaseClass, Method, Method)
METHODS_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Member, Member)
METHODS_ITERATOR_MULTI_ACTIVE(BaseClass, BaseClass, Member, Member)
METHODS_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
METHODS_ITERATOR_MULTI_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
METHODS_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
METHODS_ITERATOR_MULTI_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
METHODS_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
METHODS_ITERATOR_MULTI_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
