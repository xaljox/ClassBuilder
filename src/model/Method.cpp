/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Method.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Method'
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
// (ClassBuilderView.h include removed -- the MFC tree view is retired)
#include "qt/QtMethodDialog.h"
#include "qt/QtMethodCodeDialog.h"
#include "qt/QtCodeEditor.h"   // Qt_CloseCodeEditor (close editor on method delete)
#include "qt/QtSelectReplace.h"
#include "qt/QtContextDialog.h"
#include "qt/QtVirtualMethodsDialog.h"
#include "qt/QtExceptionSpecificationDialog.h"
//@END_USER2


// Static members


Method::Method(BaseClass* pBaseClass, Type* pType) //@INIT_800
    : Variable(pType)
    , _access(PRIVATE)
    , _pOpenWidget(NULL)
    , _code("")
    , _const(0)
    , _inlineX(0)
    , _pure(0)
    , _static(0)
    , _virtual(0)
    , _untouched(1)
    , _dllExport(false)
    , _callingConvention("")
    , _declare(pBaseClass->IsClass())
    , _implement(pBaseClass->IsClass())
    , _delete(false)
{//@CODE_800
    ConstructorInclude(pBaseClass);

    // Put in your own code
    Class* pClass = dynamic_cast<Class*>(pBaseClass);
    if (!pClass)
    {
        // if extern class, no private members allowed, they arren't visable
        _access = PROTECTED;
    }

    InitPhase();
}//@CODE_800


Method::Method(BaseClass* pBaseClass, Method* pMethod) //@INIT_803
    : Variable(*pMethod)
    , _access(pMethod->_access)
    , _pOpenWidget(NULL)
    , _code(pMethod->_code)
    , _const(pMethod->_const)
    , _inlineX(pMethod->_inlineX)
    , _pure(pMethod->_pure)
    , _static(pMethod->_static)
    , _virtual(pMethod->_virtual)
    , _untouched(pMethod->_untouched)
    , _dllExport(pMethod->_dllExport)
    , _callingConvention(pMethod->_callingConvention)
    , _declare(pMethod->_declare)
    , _implement(pMethod->_implement)
    , _delete(false)
{//@CODE_803
    // Rename while the name already exists
    if (pMethod->_virtual && !pMethod->IsDestructor())
    {
        while (pBaseClass->FindMethodWithName(Variable::GetName()))
        {
            Variable::SetName(Variable::GetName() + "X");
        }
    }

    ConstructorInclude(pBaseClass);

    // Put in your own code
    ArgumentIterator argument(pMethod);
    while (++argument)
        (void)new Argument(this, argument);

    if (!pBaseClass->IsClass())
        _code.Empty();

    if (pMethod->GetExceptionSpecification())
    {
        ExceptionSpecification* pExceptionSpecification = new ExceptionSpecification(this);
        ExceptionSpecification::ExceptionSpecificationTypeIterator iExceptionSpecificationType(pMethod->GetExceptionSpecification());
        while (++iExceptionSpecificationType)
        {
            (void)new ExceptionSpecificationType(pExceptionSpecification, iExceptionSpecificationType->GetType());
        }
    }

    InitPhase();
}//@CODE_803


/*@NOTE_1572
Constructor needed for putting a new object in the old one's context
*/
Method::Method(Method* pOld) //@INIT_1572
    : Variable(pOld)
{//@CODE_1572
    ReplaceConstructorInclude(pOld);

    _access = pOld->_access;
    _pOpenWidget = pOld->_pOpenWidget;
    _code = pOld->_code;
    _const = pOld->_const;
    _inlineX = pOld->_inlineX;
    _pure = pOld->_pure;
    _static = pOld->_static;
    _virtual = pOld->_virtual;
    _untouched = pOld->_untouched;
    _dllExport = pOld->_dllExport;
    _callingConvention = pOld->_callingConvention;
    _declare = pOld->_declare;
    _implement = pOld->_implement;
    
    // Put in your own code
}//@CODE_1572


/*@NOTE_7566
Constructor method needed to copy Method from one project to another.
*/
Method::Method(BaseClass* pBaseClass, Type* pType, Method* pMethod) //@INIT_7566
    : Variable(pType, pMethod)
    , _access(pMethod->_access)
    , _pOpenWidget(NULL)
    , _code(pMethod->_code)
    , _const(pMethod->_const)
    , _inlineX(pMethod->_inlineX)
    , _pure(pMethod->_pure)
    , _static(pMethod->_static)
    , _virtual(pMethod->_virtual)
    , _untouched(pMethod->_untouched)
    , _dllExport(pMethod->_dllExport)
    , _callingConvention(pMethod->_callingConvention)
    , _declare(pMethod->_declare)
    , _implement(pMethod->_implement)
    , _delete(false)
{//@CODE_7566
    // Rename while the name already exists
    if (pMethod->_virtual && !pMethod->IsDestructor())
    {
        while (pBaseClass->FindMethodWithName(Variable::GetName()))
        {
            Variable::SetName(Variable::GetName() + "X");
        }
    }

    ConstructorInclude(pBaseClass);

    // Put in your own code
    ArgumentIterator iArgument(pMethod);
    while (++iArgument)
    {
        (void)new Argument(this, 
            GetDataModelDoc()->FindOrDuplicateType(iArgument->GetType()), 
            iArgument);
    }
    
    if (pMethod->GetExceptionSpecification())
    {
        ExceptionSpecification* pExceptionSpecification = new ExceptionSpecification(this);
        ExceptionSpecification::ExceptionSpecificationTypeIterator iExceptionSpecificationType(pMethod->GetExceptionSpecification());
        while (++iExceptionSpecificationType)
        {
            (void)new ExceptionSpecificationType(pExceptionSpecification, 
                GetDataModelDoc()->FindOrDuplicateType(iExceptionSpecificationType->GetType()));
        }
    }

    InitPhase();
}//@CODE_7566


/*@NOTE_165
Constructor needed for serialization, not meant to use for other purposes!
*/
Method::Method() //@INIT_165
    : Variable()
    , _pOpenWidget(NULL)
    , _untouched(0)
    , _dllExport(false)
    , _callingConvention("")
    , _declare(false)
    , _implement(false)
    , _delete(false)
{//@CODE_165
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_165


/*@NOTE_163
Destructor method
*/
Method::~Method()
{//@CODE_163
    DestructorInclude();

    // Close any open modeless code editor before this method is freed, so it
    // can't dereference a dead method. Qt_CloseCodeEditor detaches it from the
    // model (no save prompt) and closes it.
    if (GetOpenWidget())
        Qt_CloseCodeEditor(GetOpenWidget());
}//@CODE_163


void Method::Add()
{//@CODE_819
    if (!GetAdded())
    {
        if (!GetParent())
        {
            SaveState(1);
            if (GetMemberAndMethodGroup())
            {
                GetMemberAndMethodGroup()->AddChildLast(this);
                GetMemberAndMethodGroup()->Update();
            }
            else
                GetBaseClass()->AddChildLast(this);
        }

        SetItemText();
        SetIcon();

        Gti::Add();

        ArgumentIterator argument(this);
        while (++argument)
            argument->Add();
    }
}//@CODE_819


/*@NOTE_6108
Change the name of the method if 'oldStr' occurs in its name, in the new name the 
occurrence of 'oldStr' is replaced by 'newStr'.
*/
int Method::ChangeName(CbString oldStr, CbString newStr)
{//@CODE_6108
    CbString name = GetName();
    int index = 0;

    if ((index = name.Find(oldStr)) != -1)
    {
        CbString newName = name.Left(index);
        newName += newStr;
        newName += name.Mid(index+oldStr.GetLength());

		bool uniqueName = true;
		DataModelDoc::BaseClassIterator iBaseClass(GetDataModelDoc());
		while (uniqueName && ++iBaseClass)
		{
			if (iBaseClass.Get() != GetBaseClass())
			{
				if (iBaseClass->FindMethodWithName(name))
					uniqueName = false;
			}
		}

		if (uniqueName)
		{
			SetNameAlways(newName);
		}
		else
		{
			SetName(newName);
		}

        return 1;
    }
    else
    {
        return 0;
    }
}//@CODE_6108


int Method::CompareFileSaveState(Method* pMethod1, Method* pMethod2)
{//@CODE_1829
    int order1 = 3;
    int order2 = 3;

    if (pMethod1->IsConstructor())
        order1 = 1;
    else if (pMethod1->IsDestructor())
        order1 = 2;
    else if (pMethod1->IsMemberMethod())
        order1 = 4;
    else if (pMethod1->IsFixed())
        order1 = 5;

    if (pMethod2->IsConstructor())
        order2 = 1;
    else if (pMethod2->IsDestructor())
        order2 = 2;
    else if (pMethod2->IsMemberMethod())
        order2 = 4;
    else if (pMethod2->IsFixed())
        order2 = 5;

    int result;
    if (order1 != order2)
        result = order1-order2;
    else if (order1 == 4) // both are member methods
    {
        Member* pMember1 = dynamic_cast<MemberMethod*>(pMethod1)->GetMember();
        Member* pMember2 = dynamic_cast<MemberMethod*>(pMethod2)->GetMember();
        result = pMember1->GetName().CompareNoCase(pMember2->GetName());
    }
    else
        result = pMethod1->GetName().CompareNoCase(pMethod2->GetName());

    if (result > 0)
    {
        pMethod2->SaveState();
        pMethod2->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_1829


int Method::CompareTree(Method* pMethod1, Method* pMethod2)
{//@CODE_4956
    if (pMethod1->IsNonMacroMethod() && pMethod2->IsMacroMethod())
        return -1;
    if (pMethod1->IsMacroMethod() && pMethod2->IsNonMacroMethod())
        return 1;
    
    if (pMethod1->IsConstructor() && !pMethod2->IsConstructor())
        return -1;
    if (!pMethod1->IsConstructor() && pMethod2->IsConstructor())
        return 1;
    if (pMethod1->IsDestructor() && !pMethod2->IsDestructor())
        return -1;
    if (!pMethod1->IsDestructor() && pMethod2->IsDestructor())
        return 1;
    
    if (pMethod1->GetStatic() == pMethod2->GetStatic())
    {
        if (pMethod1->GetAccess() == pMethod2->GetAccess())
            return pMethod1->GetName().CompareNoCase(pMethod2->GetName());
        else
            return int(pMethod1->GetAccess())-int(pMethod2->GetAccess());
    }
    else
        return int(pMethod1->GetStatic())-int(pMethod2->GetStatic());
}//@CODE_4956


int Method::CompareTreeSaveState(Method* pMethod1, Method* pMethod2)
{//@CODE_23501
    int result = CompareTree(pMethod1, pMethod2);

    if (result > 0)
    {
        pMethod2->SaveState();
        pMethod2->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_23501


Method& Method::CopyValuesFrom(Method& rMethod)
{//@CODE_1600
    Variable::CopyValuesFrom(rMethod);
    
    _access = rMethod._access;
    _code = rMethod._code;
    _const = rMethod._const;
    _inlineX = rMethod._inlineX;
    _pure = rMethod._pure;
    _static = rMethod._static;
    _virtual = rMethod._virtual;
    _dllExport = rMethod._dllExport;
    _callingConvention = rMethod._callingConvention;
    _declare = rMethod._declare;
    _implement = rMethod._implement;
    _delete = rMethod._delete;
    
    return *this;
}//@CODE_1600


Context* Method::CreateContext(ContextDeclaration* pContextDeclaration)
{//@CODE_25544
    return new MethodContext(this, pContextDeclaration);
}//@CODE_25544


/*@NOTE_5822
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void Method::Delete()
{//@CODE_5822
    // It can be that a ...Method object is changed to an Method object.
    // When this ...Method object is deleted, then it is already detached, 
    // this is the reason for this test.
    if (GetDataModelDoc())
    {
        int version = GetDataModelDoc()->GetVersion();
        if (GetVersion() <= version)
        {
            GetBaseClass()->SetVersion(version + 1);
            Class* pClass = dynamic_cast<Class*>(GetBaseClass());
            if (pClass)
            {
                CbString str;
                str.Format("@Deleted method '%s'", GetName().c_str());
        
                pClass->AddModified(str);
            }
        }
        GetBaseClass()->NotifyRemoveMethod(this);
        
        SetPhase(Complete_Phase);
        ChildActivationShapeIterator iChildActivationShape(this);
        while (++iChildActivationShape)
        {
            iChildActivationShape->GetSequenceDiagram()->CheckAndUpdatePhase();
        }
    }

    DataModelDocObject::Delete();
}//@CODE_5822


bool Method::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1351
    pGtiDropDefault = 0;
    bool value = false;

    if (ctrlKeyDown)
    {
        value = true;
    }
    else
    {
        if (GetBaseClass()->GetMemberAndMethodGroupCount())
        {
            MemberAndMethodGroup* pMemberAndMethodGroup = GetMemberAndMethodGroup();
            if (pMemberAndMethodGroup)
            {
                SaveState(1);
                pGtiDropDefault = pMemberAndMethodGroup;
                pMemberAndMethodGroup->RemoveMethod(this);
                pMemberAndMethodGroup->Update();
            }
            else
                pGtiDropDefault = GetBaseClass();

            Remove();
            value = true;
        }
    }

    return value;
}//@CODE_1351


void Method::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1357
    if (ctrlKeyDown)
    {
        Member* pDropMember = dynamic_cast<Member*>(pGtiDrop);

        BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
        MemberAndMethodGroup* pDropMemberAndMethodGroup = 
            dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);

        if (pDropClass || pDropMemberAndMethodGroup)
        {
            if (pDropMemberAndMethodGroup)
                pDropClass = pDropMemberAndMethodGroup->GetBaseClass();

            Method* pNewMethod = new Method(pDropClass, this);

            if (pDropMemberAndMethodGroup)
                pDropMemberAndMethodGroup->AddMethodLast(pNewMethod);

            pNewMethod->Add();
            pDropClass->NotifyAddMethod(pNewMethod);
        }
        else if (pDropMember)
        {
            Method* pNewMethod = new WrapMemberMethod(pDropMember, this);

            pNewMethod->Add();
            // The MEMBER's class -- pDropClass is null here by construction:
            // this branch is only reached when both it and the group were null.
            // Notifying through it dereferenced a null pointer, and
            // NotifyAddMethod is virtual, so this crashed on the vtable lookup
            // whenever a method was ctrl-dropped onto a member. The member's
            // class is the right one anyway: MemberMethod's constructor passes
            // exactly pMember->GetBaseClass() as the new method's class.
            pDropMember->GetBaseClass()->NotifyAddMethod(pNewMethod);
        }
    }
    else
    {
        MemberAndMethodGroup* pDropMemberAndMethodGroup = 
            dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);

        if (pDropMemberAndMethodGroup)
            pDropMemberAndMethodGroup->AddMethodLast(this);

        Add();
    }
}//@CODE_1357


bool Method::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1354
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        Member* pDropMember = dynamic_cast<Member*>(pGtiDrop);

        BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
        MemberAndMethodGroup* pDropMemberAndMethodGroup = 
            dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);

        if (pDropClass || pDropMemberAndMethodGroup ||
            (pDropMember && pDropMember->GetType() == GetBaseClass() && 
             !pDropMember->GetPointer() && !pDropMember->GetArray()))
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

        if (pDropClass == GetBaseClass())
            value = true;
    }

    return value;
}//@CODE_1354


Argument* Method::FindArgument(const CbString& rName)
{//@CODE_1288
    ArgumentIterator iArgument(this);
    while (++iArgument)
    {
        if (rName == iArgument->GetName())
        {
            return iArgument;
        }
    }

    return 0;
}//@CODE_1288


MethodShape* Method::FindMethodShape(ClassShape* pClassShape)
{//@CODE_3960
    MethodShapeIterator iMethodShape(this);
    while (++iMethodShape)
    {
        if (pClassShape == iMethodShape->GetClassShape())
        {
            return iMethodShape;
        }
    }

    return 0;
}//@CODE_3960


int Method::FindStringInStrippedCode(const CbString& string)
{//@CODE_3164
    int offset = 0;
    
    CbString code = _code;
    CbString newCode = _code;
    int index;

    // Strip line comment
    while ((index = code.Find("//")) != -1)
    {
        newCode = code.Left(index);
        code = code.Mid(index+2);
        if ((index = code.Find(NL)) != -1)
            newCode += NL + code.Mid(index+2);

        code = newCode;
    }

    // Strip block comment
    while ((index = code.Find("/*")) != -1)
    {
        newCode = code.Left(index);
        code = code.Mid(index+2);
        if ((index = code.Find("*/")) != -1)
            newCode += code.Mid(index+2);

        code = newCode;
    }
    
    while (1)
    {
        code = newCode.Mid(offset);
        int index = code.Find(string);

        if (index != -1)
        {
            if (index && __iscsym(code[index-1]))
            {
                offset += (index + 1);
                continue;
            }

            if (!index && offset && __iscsym(newCode[offset-1]))
            {
                offset += (index + 1);
                continue;
            }

            int afterIndex = index + string.GetLength();
            if (afterIndex < code.GetLength() && __iscsym(code[afterIndex]))
            {
                offset += (index + 1);
                continue;
            }

            return index + offset;
        }
        else
        {
            return -1;
        }
    }
}//@CODE_3164


Argument* Method::GetArgumentAtSamePosition(Argument* pArgument)
{//@CODE_1602
    ArgumentIterator iArgument(this);
    ArgumentIterator iArgumentMethod(pArgument->GetMethod());
    while (++iArgument && ++iArgumentMethod)
    {
        if (pArgument == iArgumentMethod.Get())
            return iArgument;
    }
    
    return 0;
}//@CODE_1602


CbString Method::GetContextList()
{//@CODE_27310
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
    
    MethodContextIterator iMethodContext(this);
    while (++iMethodContext)
    {
        if (!str.IsEmpty())
            str += ", ";
        str += iMethodContext->GetContextDeclaration()->GetName();
    }

    return str;
}//@CODE_27310


CbString Method::GetEndContextDeclaration()
{//@CODE_26339
    CbString str;
    
    MethodContextIterator iMethodContext(this);
    while (--iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetEndContextDeclaration();
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
}//@CODE_26339


CbString Method::GetEndContextImplementation()
{//@CODE_26340
    CbString str;
    
    MethodContextIterator iMethodContext(this);
    while (--iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetEndContextImplementation();
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
}//@CODE_26340


Context* Method::GetFirstContext()
{//@CODE_26304
    return GetFirstMethodContext();
}//@CODE_26304


const CbString Method::GetInterfaceCpp()
{//@CODE_812
    CbString str;

    if (GetDataModelDoc()->GetDataModel()->GetModifiers() && GetDllExport())
        str += "AFX_EXT_CLASS ";

    str += GetTypeName();

    if (GetDataModelDoc()->GetDataModel()->GetModifiers() && 
        !GetCallingConvention().IsEmpty())
        str += GetCallingConvention() + " ";

    str += GetBaseClass()->GetName() + "::" + GetName() + "(";
    ArgumentIterator argument(this);
    while (++argument)
    {
        str += argument->GetTypeName() + argument->GetVariableName();

        if (!argument.IsLast())
            str += ", ";
    }
    str += ")";

    if (GetConst())
        str += " const";

    if (GetExceptionSpecification())
        str += GetExceptionSpecification()->GetThrowString();

    if (IsConstructor())
    {
        CbString id;
        id.Format(" //@INIT_%d", GetId());
        str += id;
    }

    return str;
}//@CODE_812


Context* Method::GetNextContext(Context* pContextPos)
{//@CODE_26305
    return GetNextMethodContext((MethodContext*)pContextPos);
}//@CODE_26305


/*@NOTE_4748
Give the text as it should appear in the class diagram, it is a method of this class in 
instead of MethodShape, because of the Select dialog.
*/
CbString Method::GetShapeText(VerbosityType verbosity)
{//@CODE_4748
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

    if (GetConst() || GetVirtual())
    {
        text += "<<";
        if (GetVirtual())
        {
            if (GetPure())
                text += "Abstract";
            else
                text += "Virtual";

            if (GetConst())
                text += " Const";
        }
        else if (GetConst())
        {
            text += "Const";
        }

        text += ">> ";
    }

    text += GetName() + "(";
    if (verbosity&VERBOSITY_ARGUMENT)
    {
        ArgumentIterator argument(this);
        while (++argument)
        {
            text += argument->GetVariableName() + " : " + argument->GetTypeName();
            // Get rid of space 
            text.TrimRight();
    
            if (!argument->GetDefault().IsEmpty())
                text += " = " + argument->GetDefault();
    
            if (!argument.IsLast())
                text += ", ";
        }
    }

    if (GetTypeName().IsEmpty())
        text += ")";
    else
    {
        text += ") : ";
        if (GetStatic() && (verbosity&VERBOSITY_STATIC))
            text += "static ";
        text += GetTypeName();
    }

    if (GetDelete())
        text += " = delete";

    return text;
}//@CODE_4748


/*@NOTE_34220
Give the text as it should appear in the sequence diagram.
*/
CbString Method::GetSignalShapeText(bool arguments, bool argumentNames,
                                    bool scope)
{//@CODE_34220
    CbString text;

    if (scope)
    {
        text += GetBaseClass()->GetName() + "::";
    }
    
    text += GetName() + "(";
    if (arguments || argumentNames)
    {
        ArgumentIterator argument(this);
        while (++argument)
        {
            if (arguments && argumentNames)
            {
                text += argument->GetVariableName() + " : " + argument->GetTypeName();
                // Get rid of space 
                text.TrimRight();
        
                if (!argument->GetDefault().IsEmpty())
                    text += " = " + argument->GetDefault();
            }
            else
            {
                if (arguments)
                {
                    text += argument->GetTypeName();
                    // Get rid of space 
                    text.TrimRight();
        
                    if (!argument->GetDefault().IsEmpty())
                        text += " = " + argument->GetDefault();
                }
                
                if (argumentNames)
                {
                    text += argument->GetVariableName();
                }
            }
    
            if (!argument.IsLast())
                text += ", ";
        }
    }
    text += ")";

    if (arguments && !GetTypeName().IsEmpty())
    {
        text += " : ";
        if (GetStatic())
            text += "static ";
        text += GetTypeName();
    }
    else if (GetStatic())
    {
        text += " : static ";
    }

    return text;
}//@CODE_34220


CbString Method::GetStartContextDeclaration()
{//@CODE_26341
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
    
    MethodContextIterator iMethodContext(this);
    while (++iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetStartContextDeclaration();
    }
    
    return str;
}//@CODE_26341


CbString Method::GetStartContextImplementation()
{//@CODE_26342
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
    
    MethodContextIterator iMethodContext(this);
    while (++iMethodContext)
    {
        str += iMethodContext->GetContextDeclaration()->GetStartContextImplementation();
    }
    
    return str;
}//@CODE_26342


int Method::HasStringInCode(const CbString& string)
{//@CODE_7411
    int value = 0;

    if (FindStringInStr(_code, string) != -1)
    {
        value = 1;
    }
    
    return value;
}//@CODE_7411


void Method::InitCode()
{//@CODE_811
    CbString typeName = GetType()->GetName();
    int index = typeName.Find(" ");
    if (index != -1 && typeName.Left(index) != "unsigned" && typeName.Find(",") == -1)
    {
        typeName = typeName.Left(index);
    }

    if (typeName != "" && typeName != "void")
    {
        _code = GetIndent() + typeName;
        if (GetPointer())
            _code += "*";
        _code += " value;" NL NL;

        if (!GetDataModelDoc()->GetCommentInitialCode().IsEmpty())
        {
            CbString line = GetDataModelDoc()->GetCommentInitialCode();
            while ((index = line.Find("@")) != -1)
            {
                line = line.Left(index) + GetName() + line.Mid(index+1);
            }
            _code += GetIndent() + line + NL NL;
        }

        _code += GetIndent() + "return value;" NL;
    }
    else
    {
        if (!GetDataModelDoc()->GetCommentInitialCode().IsEmpty())
        {
            CbString line = GetDataModelDoc()->GetCommentInitialCode();
            while ((index = line.Find("@")) != -1)
            {
                line = line.Left(index) + GetName() + line.Mid(index+1);
            }
            _code = GetIndent() + line + NL;
        }
    }

    SetUntouched(1);
}//@CODE_811


void Method::InitPhase()
{//@CODE_23460
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
}//@CODE_23460


bool Method::IsDirectMethod() const
{//@CODE_814
    return ((IsMemberMethod() || IsFromRelationMethod() || IsMacroMethod())? 0: 1); 
}//@CODE_814


int Method::IsFixed() const
{//@CODE_35418
    return 0;
}//@CODE_35418


bool Method::IsNonMacroMethod() const
{//@CODE_2001
    return 1;
}//@CODE_2001


bool Method::IsNonMacroPrivateMethod() const
{//@CODE_817
 return ((_access == PRIVATE && IsNonMacroMethod())? 1: 0); 
}//@CODE_817


bool Method::IsNonMacroProtectedMethod() const
{//@CODE_816
 return ((_access == PROTECTED && IsNonMacroMethod())? 1: 0); 
}//@CODE_816


bool Method::IsNonMacroPublicMethod() const
{//@CODE_815
 return ((_access == PUBLIC && IsNonMacroMethod())? 1: 0); 
}//@CODE_815


int Method::IsNonPure() const
{//@CODE_810
 return (_pure ? 0: 1); 
}//@CODE_810


bool Method::IsNonStatic() const
{//@CODE_818
 return (_static? 0: 1); 
}//@CODE_818


bool Method::IsNormalConstructor() const
{//@CODE_813
    if (IsConstructor())
    {
        if (IsSerializeConstructor())
            return 0;
        else if (IsReplaceConstructor())
            return 0;
        else
            return 1;
    }
    return 0;
}//@CODE_813


bool Method::IsPrivateMethod() const
{//@CODE_7496
 return ((_access == PRIVATE)? 1: 0); 
}//@CODE_7496


bool Method::IsProtectedMethod() const
{//@CODE_7497
 return ((_access == PROTECTED)? 1: 0); 
}//@CODE_7497


bool Method::IsPublicMethod() const
{//@CODE_7498
 return ((_access == PUBLIC)? 1: 0); 
}//@CODE_7498


int Method::IsSimilar(Method* pMethod)
{//@CODE_806
    // similar, not exactly the same object !!
    if (this != pMethod && GetName() == pMethod->GetName() && 
        Variable::IsSimilar(pMethod) &&
        GetArgumentCount() == pMethod->GetArgumentCount())
    {
        ArgumentIterator argument1(this);
        ArgumentIterator argument2(pMethod);
        while (++argument1 && ++argument2)
        {
            if (!argument1->IsSimilar(argument2))
                return 0;
        }
        return 1;
    }

    return 0;
}//@CODE_806


void Method::NotifyAddMember(Member* pMember)
{//@CODE_22997
}//@CODE_22997


void Method::NotifyRemoveMember(Member* pMember)
{//@CODE_22995
}//@CODE_22995


int Method::OnAddArgument(bool checkOnly)
{//@CODE_824
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Type* pType = GetDataModelDoc()->FindType("");
        
        int argumentCount = 0;
        Argument** pArgumentArray = 0;
        
        // In case of virtual function, adjust virtual overides also.
        if (GetVirtual())
        {
            pArgumentArray = 
                new Argument*[GetDataModelDoc()->GetTypeCount()];
            
            DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsExternClass);
            while (++iType)
            {
                ExternClass* pDerivedClass = (ExternClass*)iType.Get();
                
                if (pDerivedClass->IsBaseClass(GetBaseClass()))
                {
                    Method* pMethod = pDerivedClass->FindSimilarMethod(this);
                    if (pMethod)
                    {
                        pArgumentArray[argumentCount++] = new Argument(pMethod, pType);
                    }
                }
            }
        }
        
        Argument* pArgument = new Argument(this, pType);
        
        if (pArgument->OnEditAttributes())
        {
            pArgument->Add();
            for (int i = 0; i < argumentCount; i++)
                pArgumentArray[i]->Add();
        }
        else
        {
            GetDataModelDoc()->RollBack();
        }
        
        delete pArgumentArray;
    }
    
    return 1;
}//@CODE_824


int Method::OnAddMethod(bool checkOnly)
{//@CODE_4385
    if (IsMemberMethod())
    {
        return Gti::OnAddMethod(checkOnly);
    }
    else
    {
        if (!checkOnly)
        {
            GetDataModelDoc()->MarkLastUndo();
            Method* pMethod = new Method(GetBaseClass(), GetDataModelDoc()->FindType(""));

            // Copy settings current method.
            pMethod->SetAccess(GetAccess());
            pMethod->SetVirtual(GetVirtual());
            pMethod->SetPure(GetPure());
            pMethod->SetStatic(GetStatic());
            pMethod->SetConst(GetConst());
            pMethod->SetInline(GetInline());

            if (pMethod->OnEditAttributes())
            {
                pMethod->Add();
                GetBaseClass()->NotifyAddMethod(pMethod);
            }
            else
                GetDataModelDoc()->RollBack();
        }

        return 1;
    }
}//@CODE_4385


int Method::OnAddVirtuals(bool checkOnly)
{//@CODE_36023
    if (GetVirtual() && GetBaseClass()->GetInheritCount())
    {
        if (!checkOnly)
        {
            void* ownerHwnd = Cb_OwnerHwnd();
            Qt_ShowVirtualMethodsDialogForMethod(this, ownerHwnd);
        }

        return 1;
    }
    else
    {
        return GetParent()->OnAddVirtuals(checkOnly);
    }
}//@CODE_36023


int Method::OnCopy(bool checkOnly)
{//@CODE_40791
    bool enable = (!IsFindMethod() && !IsDestructor() &&
        !IsFixed() && !IsMemberMethod());

    if (enable && !checkOnly)
    {
        SetGtiCopy(this);
    }

    return enable;
}//@CODE_40791


int Method::OnDelete(bool checkOnly)
{//@CODE_823
    if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete method '%s::%s'",
            GetBaseClass()->GetName().c_str(), GetItemText().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            // Close an open code editor first (no save prompt -- the body
            // dies with the method; undo restores the saved state). The
            // deleted method is PARKED on the undo stack until its final
            // destruction, so the editor must not stay open on it. The old
            // behaviour -- refusing the delete while an editor was open --
            // also disabled the Delete action via the checkOnly path, which
            // read as "nothing happens" (JV 2026-07-15).
            if (GetOpenWidget())
                Qt_CloseCodeEditor(GetOpenWidget());
            Delete();
        }
    }

    return 1;
}//@CODE_823


int Method::OnEditAttributes(bool checkOnly)
{//@CODE_821
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;
    if (Qt_ShowMethodDialog(this, changed, ownerHwnd))
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
}//@CODE_821


int Method::OnEditContext(bool checkOnly)
{//@CODE_25790
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
}//@CODE_25790


int Method::OnEditExceptionSpecification(bool checkOnly)
{//@CODE_22710
    if (!checkOnly)
    {
        UndoBase* pLastUndoBase = GetDataModelDoc()->MarkLastUndo();
        void* ownerHwnd = Cb_OwnerHwnd();

        if (Qt_ShowExceptionSpecificationDialog(this, ownerHwnd))
		{
			Update();
			return 1;
		}
		else
        {
			GetDataModelDoc()->RollBack(pLastUndoBase);
			Update();
            return 0;
        }
    }
    else
    {
        return 1;
    }
}//@CODE_22710


int Method::OnOpen(bool checkOnly)
{//@CODE_822
    // The code editors are MODELESS: Qt_ShowMethodCodeDialog opens one or, if
    // this method already has an editor open (GetOpenWidget() set), refocuses
    // it. So Open stays ENABLED while an editor is open -- reopening = refocus.
    // (The "can't delete while open" guard lives in OnDelete, not here.)
    if (!checkOnly)
    {
        if (!GetImplement())
            OnEditAttributes();
        else
        {
            if (GetBaseClass()->IsClass())
            {
                void* ownerHwnd = Cb_OwnerHwnd();
                Qt_ShowMethodCodeDialog(this, ownerHwnd);
            }
            else
                OnEditAttributes();
        }
    }

    int result = 0;
    if (GetBaseClass()->IsClass() && GetImplement())
        result = 1;

    return result;
}//@CODE_822


/*@NOTE_22946
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Method::OnUndoRedoAdded()
{//@CODE_22946
    Gti::OnUndoRedoAdded();
    
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }
}//@CODE_22946


/*@NOTE_22959
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Method::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22959
    Gti::OnUndoRedoChanged(pOldState);

    Method* pMethod = (Method*)pOldState;
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }
    if (pMethod && pMethod->GetMemberAndMethodGroup())
    {
        pMethod->GetMemberAndMethodGroup()->Update();
    }

    // The restore may have swapped the stored code behind an open modeless
    // code editor -- let it reload (only if it holds no unsaved edits).
    if (GetOpenWidget() && pMethod)
        Qt_UndoRedoOpenCodeEditor(GetOpenWidget(), pMethod);

}//@CODE_22959


/*@NOTE_22937
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called after the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Method::OnUndoRedoRemoved()
{//@CODE_22937
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }

    // The method just left the data structure (undo of its add, or redo of
    // its delete) and is parked on the undo stack until final destruction --
    // close an open code editor NOW, not then (it would edit a parked
    // object). This is the "direct remove" the old commented-out
    // DestroyWindow block always intended.
    if (GetOpenWidget())
        Qt_CloseCodeEditor(GetOpenWidget());
}//@CODE_22937


void Method::ReplaceInCode(const CbString& oldString, const CbString& newString)
{//@CODE_1400
    ReplaceInNote(oldString, newString);

    // If code is adjusted, check for open dialogs and update code in it
    if (ReplaceInStr(_code, oldString, newString))
    {
        ArgumentIterator iArgument(this);
        while (++iArgument)
        {
            iArgument->ReplaceInPath(oldString, newString);
        }

    }

    // Mirror the replacement into an open modeless code editor, so the
    // window tracks the model and an unedited editor stays identical to the
    // stored code (no phantom save prompt on close). Fired even when the
    // stored code had no hit -- unsaved editor text may have one. For a
    // Constructor this also covers the init editor (the dialog applies the
    // replacement to both of its editors).
    if (GetOpenWidget())
        Qt_ReplaceInOpenCodeEditor(GetOpenWidget(), oldString, newString);
}//@CODE_1400


/*@NOTE_23216
Virtual method to replace strings at various places, called if a type name changes.
*/
void Method::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23216
    ReplaceInCode(oldString, newString);
}//@CODE_23216


void Method::SetIcon()
{//@CODE_809
    // The icon encodes the body state: hollow core = untouched (no code
    // yet), darker tint = inline, full = code in the .cpp. An untouched
    // inline method combines both: hollow core in the darker rim. Virtual
    // is NOT on the icon -- the tree paints the `virtual` keyword in
    // magenta (SignatureKeywordDelegate), JV 2026-07-12.
    int offset = _access;
	if (GetBaseClass()->IsClass())
	{
		if (GetUntouched())
		{
			if (GetInline() && !IsConstructor() && !IsDestructor())
			{
				Gti::SetIcon(ICON_PUBLIC_EMPTY_INLINE_METHOD + _access);
				return;
			}
			offset += EMPTY_OFFSET;
		}
		else if (GetInline())
		{
			offset += INLINE_OFFSET;
		}
	}

    if (IsConstructor())
        Gti::SetIcon(ICON_PUBLIC_CONSTRUCTOR + offset);
    else if (IsDestructor())
        Gti::SetIcon(ICON_PUBLIC_DESTRUCTOR + offset);
    else
        Gti::SetIcon(ICON_PUBLIC_METHOD + offset);
}//@CODE_809


void Method::SetItemText()
{//@CODE_808
    CbString itemText;

    if (GetStatic())
        itemText += "static ";

    if (GetVirtual())
        itemText += "virtual ";

    if (GetDllExport())
        itemText += "AFX_EXT_CLASS ";

    itemText += GetTypeName();

    if (!GetCallingConvention().IsEmpty())
        itemText += GetCallingConvention() + " ";

    itemText += GetName() + "(";
    ArgumentIterator argument(this);
    while (++argument)
    {
        itemText += argument->GetTypeName() + argument->GetVariableName();

        if (!argument->GetDefault().IsEmpty())
            itemText += " = " + argument->GetDefault();

        if (!argument.IsLast())
            itemText += ", ";
    }
    itemText += ")";

    if (GetConst())
        itemText += " const";

    if (GetExceptionSpecification())
        itemText += GetExceptionSpecification()->GetThrowString();
    
    if (GetPure())
        itemText += " = 0";
    else if (GetDelete())
        itemText += " = delete";

    Gti::SetItemText(itemText);
}//@CODE_808


void Method::SetName(const CbString& rName)
{//@CODE_1398
    if (GetName().IsEmpty() || IsConstructor() || IsDestructor())
    {
        SaveState();

        Variable::SetName(rName);
    }
    else
    {
        if (rName != GetName())
        {
            SaveState();

            // Do modification on code of methods first
            bool used = false;
            DataModel::ClassIterator iClass(GetDataModelDoc()->GetDataModel());
            while (!used && ++iClass)
            {
                BaseClass::MethodIterator method(iClass);
                while (!used && ++method)
                {
                    if (method->HasStringInCode(GetName()))
                    {
                        used = true;
                    }
                }
            }
            
            if (used)
            {
                void* ownerHwnd = Cb_OwnerHwnd();
                Qt_ShowSelectReplace(GetDataModelDoc()->GetDataModel(),
                                     GetName(), rName, ownerHwnd);
            }

            Variable::SetName(rName);
        }
    }
}//@CODE_1398


/*@NOTE_6106
Same as SetName, but now no questions are asked.
*/
void Method::SetNameAlways(const CbString& rName)
{//@CODE_6106
    SaveState();

    if (rName != GetName())
    {
        // Do modification on code of methods first
        DataModel::ClassIterator iClass(GetDataModelDoc()->GetDataModel());
        while (++iClass)
        {
            BaseClass::MethodIterator method(iClass);
            while (++method)
            {
                if (method->HasStringInCode(GetName()))
                {
                    method->ReplaceInCode(GetName(), rName);
                }
            }
        }
        
        Variable::SetName(rName);
    }
}//@CODE_6106


void Method::SetPhaseDownAndUpwards(Phase phase)
{//@CODE_35986
    if (IsAllowedToEditPhase(phase) && phase != None_Phase && GetPhase() != phase)
    {
        SaveState();
        
        Gti::SetPhaseDownAndUpwards(phase);
        
        ChildActivationShapeIterator iChildActivationShape(this);
        while (++iChildActivationShape)
        {
            iChildActivationShape->GetSequenceDiagram()->CheckAndUpdatePhase();
        }
    }
}//@CODE_35986


bool Method::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40780
    AccessType access = GetAccess();
    bool show = (access == PUBLIC    && pTreeViewModel->GetShowPublicMethods())
             || (access == PROTECTED && pTreeViewModel->GetShowProtectedMethods())
             || (access == PRIVATE   && pTreeViewModel->GetShowPrivateMethods());
    if (!show)
    {
        return false;
    }

    if (!(GetStatic() ? pTreeViewModel->GetShowStaticMethods()
                      : pTreeViewModel->GetShowNonStaticMethods()))
    {
        return false;
    }

    return Gti::ShownByFilter(pTreeViewModel);
}//@CODE_40780


void Method::Update()
{//@CODE_820
    if (GetAdded())
    {
        SetItemText();
        SetIcon();

        Gti::Update();
    }
}//@CODE_820


AccessType Method::GetAccess()
{//@CODE_1214
    return _access;
}//@CODE_1214


void Method::SetAccess(AccessType access)
{//@CODE_1215
    _access = access;
}//@CODE_1215


const CbString& Method::GetCode()
{//@CODE_1217
    if (_code.IsEmpty() || IsFixed())
    {
        InitCode();
    }

    return _code;
}//@CODE_1217


void Method::SetCode(const CbString& rCode)
{//@CODE_1218
    if (_code != rCode)
    {
        _code = rCode;

        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseDownAndUpwards(Implementation_Phase);
        }
        
        if (!rCode.IsEmpty())
        {
            SetUntouched(0);
            if (rCode[rCode.GetLength()-1] != '\n')
                _code += NL;
        }
        else
        {
            SetUntouched(1);
        }

    }
}//@CODE_1218


bool Method::GetConst()
{//@CODE_1220
    return _const;
}//@CODE_1220


void Method::SetConst(bool val)
{//@CODE_1221
    _const = val;
}//@CODE_1221


bool Method::GetInline()
{//@CODE_1223
    return _inlineX;
}//@CODE_1223


void Method::SetInline(bool val)
{//@CODE_1224
    _inlineX = val;
}//@CODE_1224


QWidget* Method::GetOpenWidget()
{//@CODE_1211
    return _pOpenWidget;
}//@CODE_1211


void Method::SetOpenWidget(QWidget* pDialog)
{//@CODE_1212
    _pOpenWidget = pDialog;
}//@CODE_1212


bool Method::GetPure() const
{//@CODE_1226
    return _pure;
}//@CODE_1226


void Method::SetPure(bool pure)
{//@CODE_1227
    _pure = pure;
}//@CODE_1227


bool Method::GetStatic() const
{//@CODE_1229
    return _static;
}//@CODE_1229


void Method::SetStatic(bool val)
{//@CODE_1230
    _static = val;
}//@CODE_1230


bool Method::GetVirtual() const
{//@CODE_1232
    return _virtual;
}//@CODE_1232


void Method::SetVirtual(bool val)
{//@CODE_1233
    _virtual = val;
}//@CODE_1233


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5492
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Method::CleanupReferences()
{
    Variable::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    CLEANUP_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
}


/*@NOTE_162
Method which must be called first in a constructor
*/
void Method::ConstructorInclude(BaseClass* pBaseClass)
{
    INIT_MULTI_OWNED_ACTIVE(Method, Method, Argument, Argument)
    INIT_MULTI_OWNED_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    INIT_MULTI_OWNED_ACTIVE(Method, Method, MethodShape, MethodShape)
    INIT_SINGLE_OWNED_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    INIT_MULTI_OWNED_ACTIVE(Method, Method, MethodContext, MethodContext)
    INIT_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    INIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
}


/*@NOTE_164
Method which must be called first in a destructor
*/
void Method::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Method, Method, Argument, Argument)
    EXIT_MULTI_OWNED_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    EXIT_MULTI_OWNED_ACTIVE(Method, Method, MethodShape, MethodShape)
    EXIT_SINGLE_OWNED_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    EXIT_MULTI_OWNED_ACTIVE(Method, Method, MethodContext, MethodContext)
    EXIT_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    EXIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    EXIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
}


/*@NOTE_5493
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Method::RemoveReferences()
{
    REMOVE_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    REMOVE_MULTI_OWNED_ACTIVE(Method, Method, MethodContext, MethodContext)
    REMOVE_SINGLE_OWNED_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    REMOVE_MULTI_OWNED_ACTIVE(Method, Method, MethodShape, MethodShape)
    REMOVE_MULTI_OWNED_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    REMOVE_MULTI_OWNED_ACTIVE(Method, Method, Argument, Argument)
    Variable::RemoveReferences();
    REMOVE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    REMOVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
}


/*@NOTE_1574
Method which must be called first in a replace constructor
*/
void Method::ReplaceConstructorInclude(Method* pOld)
{
    REPLACE_MULTI_OWNED_ACTIVE(Method, Method, Argument, Argument)
    REPLACE_MULTI_OWNED_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    REPLACE_MULTI_OWNED_ACTIVE(Method, Method, MethodShape, MethodShape)
    REPLACE_SINGLE_OWNED_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    REPLACE_MULTI_OWNED_ACTIVE(Method, Method, MethodContext, MethodContext)
    REPLACE_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    REPLACE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    REPLACE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
}


/*@NOTE_5494
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Method::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Method* pMethod = (Method*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    RESTORE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    Variable::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5496
Save the state of the current object relations to pDataModelDocObject.
*/
void Method::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Variable::SaveReferences(pDataModelDocObject);
    Method* pMethod = (Method*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    SAVE_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
}


/*@NOTE_167
Serialize the members only to a CbObject object
*/
void Method::Serialize(CbArchive& archive)
{
    Variable::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _access;
        archive << _code;
        archive << _const;
        archive << _inlineX;
        archive << _pure;
        archive << _static;
        archive << _virtual;
        archive << _untouched;
        archive << _dllExport;
        archive << _callingConvention;
        archive << _declare;
        archive << _implement;
        archive << _delete;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _access;
            archive >> _code;
            archive >> _const;
            archive >> _inlineX;
            archive >> _pure;
            archive >> _static;
            archive >> _virtual;
            archive >> _untouched;
            archive >> _dllExport;
            archive >> _callingConvention;
            archive >> _declare;
            archive >> _implement;
        }
        if (2 <= _objectVersion)
        {
            archive >> _delete;
        }
    }
}


/*@NOTE_166
Method which must be called first in a serialize constructor
*/
void Method::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Method, Method, Argument, Argument)
    INIT_MULTI_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    INIT_MULTI_ACTIVE(Method, Method, MethodShape, MethodShape)
    INIT_SINGLE_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    INIT_MULTI_ACTIVE(Method, Method, MethodContext, MethodContext)
    INIT_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_PASSIVE(BaseClass, BaseClass, Method, Method)
    INIT_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
}


/*@NOTE_169
Serialize the relations to a CbObject object
*/
void Method::SerializeRelations(CbArchive& archive,
                                DataModelDocObject* pointerArray[])
{
    Variable::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Method, Method, Argument, Argument)
        WRITE_MULTI_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
        WRITE_MULTI_ACTIVE(Method, Method, MethodShape, MethodShape)
        WRITE_SINGLE_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
        WRITE_MULTI_ACTIVE(Method, Method, MethodContext, MethodContext)
        WRITE_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Method, Method, Argument, Argument)
            READ_MULTI_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
            READ_MULTI_ACTIVE(Method, Method, MethodShape, MethodShape)
            READ_SINGLE_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
            READ_MULTI_ACTIVE(Method, Method, MethodContext, MethodContext)
            READ_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Method)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Method, Method, Argument, Argument)
METHODS_ITERATOR_MULTI_ACTIVE(Method, Method, Argument, Argument)
METHODS_MULTI_OWNED_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
METHODS_ITERATOR_NOFILTER_MULTI_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
METHODS_MULTI_OWNED_ACTIVE(Method, Method, MethodShape, MethodShape)
METHODS_ITERATOR_MULTI_ACTIVE(Method, Method, MethodShape, MethodShape)
METHODS_SINGLE_OWNED_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
METHODS_MULTI_OWNED_ACTIVE(Method, Method, MethodContext, MethodContext)
METHODS_ITERATOR_MULTI_ACTIVE(Method, Method, MethodContext, MethodContext)
METHODS_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
METHODS_ITERATOR_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
METHODS_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
METHODS_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
