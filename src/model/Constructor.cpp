/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Constructor.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Constructor'
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
#include "qt/QtConstructorDialog.h"
#include "qt/QtConstructorCodeDialog.h"
#include "qt/QtCodeEditor.h"   // Qt_UndoRedoOpenCodeEditor (undo behind an open editor)
//@END_USER2


// Static members


Constructor::Constructor(BaseClass* pBaseClass) //@INIT_962
    : Method(pBaseClass, pBaseClass->GetDataModelDoc()->FindType(""))
    , _init("\377")
    , _explicit(0)
{//@CODE_962
    ConstructorInclude();

    // Put in your own code
	_untouched = 0;
    SetName(pBaseClass->Type::GetName());
    SetAccess(PUBLIC);

    SetNote("Constructor method.");
}//@CODE_962


/*@NOTE_7577
Constructor method needed to copy constructor from one project to another.
*/
Constructor::Constructor(BaseClass* pBaseClass,
                         Constructor* pConstructor) //@INIT_7577
    : Method(pBaseClass, pBaseClass->GetDataModelDoc()->FindType(""), pConstructor)
    , _init(pConstructor->_init)
    , _explicit(0)
{//@CODE_7577
    ConstructorInclude();

    // Put in your own code
	_untouched = 0;
}//@CODE_7577


/*@NOTE_360
Constructor needed for serialization, not meant to use for other purposes!
*/
Constructor::Constructor() //@INIT_360
    : Method()
    , _explicit(0)
{//@CODE_360
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_360


/*@NOTE_358
Destructor method
*/
Constructor::~Constructor()
{//@CODE_358
    DestructorInclude();

    // Put in your own code
}//@CODE_358


void Constructor::CreateArguments()
{//@CODE_964
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        Method::ArgumentIterator argument(pClass->GetConstructorIncludeMethod());
        while (++argument)
            (void)new Argument(this, argument);

        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            BaseClass::MethodIterator constructor(inherit->GetBaseClass(), 
                &Method::IsNormalConstructor);
            if (++constructor)
            {
                Method::ArgumentIterator argument(constructor);
                while (++argument)
                {
                    Class* pDocument = pClass->GetDataModel()->GetDocument();
                    if (!(pDocument && argument->GetType() == pDocument && 
                          pClass->GetConstructorIncludeMethod()->GetArgumentCount()))
                    {
                        (void)new Argument(this, argument);
                    }
                }
            }
        }

        Class::MemberIterator member(pClass);
        while (++member)
        {
            if (member->GetInitialization().IsEmpty() && !member->GetArray())
            {
                BaseClass* pMemberClass = dynamic_cast<BaseClass*>(member->GetType());
                if (pMemberClass && !member->GetPointer() && !member->GetReference())
                {
                    BaseClass::MethodIterator constructor(pMemberClass, 
                        &Method::IsNormalConstructor);
                    if (++constructor)
                    {
                        Method::ArgumentIterator argument(constructor);
                        while (++argument)
                            (void)new Argument(this, argument);
                    }
                }
                else
                {
                    (void)new MemberArgument(this, member);
                }
            }
        }
    }
}//@CODE_964


bool Constructor::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1365
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        value = Method::Drag(ctrlKeyDown, pGtiDropDefault);
    }

    return value;
}//@CODE_1365


int Constructor::ExplicitAllowed()
{//@CODE_35417
    if (GetArgumentCount())
    {
        if (GetArgumentCount() > 1)
        {
            Method::ArgumentIterator iArgument(this);
            ++iArgument;
            while (++iArgument)
            {
                if (iArgument->GetDefault().IsEmpty())
                {
                    return 0;
                }
            }
        }
        
        return 1;
    }
    
    return 0;
}//@CODE_35417


/*@NOTE_16940
Give the text as it should appear in the class diagram, it is a method of this class in 
instead of MethodShape, because of the Select dialog.
*/
CbString Constructor::GetShapeText(VerbosityType verbosity)
{//@CODE_16940
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

    if (GetExplicit() && ExplicitAllowed())
    {
        text += "<<Explicit>> ";
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

    text += ")";

    if (GetDelete())
        text += " = delete";

    return text;
}//@CODE_16940


int Constructor::HasStringInCode(const CbString& string)
{//@CODE_7414
    int value = 0;

    if (FindStringInStr(_code, string) != -1 ||
        FindStringInStr(_init, string) != -1)
    {
        value = 1;
    }
    
    return value;
}//@CODE_7414


void Constructor::InitCode()
{//@CODE_965
    _code.Empty();

    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code += GetIndent() + "ConstructorInclude(";

        bool first = true;
        Class::ToRelationIterator relation(pClass);
        while (++relation)
        {
            if (relation->GetOwned() && !relation->GetStatic())
            {
                if (!first)
                    _code += ", ";
                else
                    first = false;

                _code += "p" + relation->GetFromName();
            }
        }
        _code += ");" NL NL;
        _code += GetIndent() + "// Put in your own code" NL;
    }
}//@CODE_965


void Constructor::InitInit()
{//@CODE_966
    _init.Empty();

    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        bool first = true;
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            BaseClass::MethodIterator constructor(inherit->GetBaseClass(), 
                &Method::IsNormalConstructor);
            if (++constructor)
            {
                if (first) 
                { 
                    first = false; 
                    _init += GetIndent() + ": "; 
                } 
                else  
                    _init += GetIndent() + ", ";

                _init += inherit->GetBaseClass()->Type::GetName() + inherit->GetTemplate() + "(";
                Method::ArgumentIterator argument(constructor);
                while (++argument)
                {
                    Argument* pArgument = pClass->GetConstructorIncludeMethod()->
                                          GetFirstArgument();
                    Class* pDocument = pClass->GetDataModel()->GetDocument();
                    if (pDocument && argument->GetType() == pDocument && 
                        pArgument && pArgument->GetType() != pDocument)
                    {
                        _init += pArgument->GetName() + "->Get";
                        _init += pDocument->GetFirstFromRelation()->GetFromName();
                        _init += "()";
                    }
                    else
                    {
                        _init += argument->GetName();
                    }
                    if (!argument.IsLast())
                        _init += ", ";
                }
                _init += ")" NL;
            }
        }

        Class::MemberIterator member(pClass);
        while (++member)
        {
            if (!member->GetStatic() && !member->GetArray())
            {
                _init += member->GetStartContextImplementation();
                if (first) 
                { 
                    first = false; 
                    _init += GetIndent() + ": "; 
                } 
                else  
                    _init += GetIndent() + ", ";

                _init += member->GetPrefixedName() + "(";
                if (member->GetInitialization().IsEmpty())
                {
                    BaseClass* pMemberClass = dynamic_cast<BaseClass*>(member->GetType());
                    if (pMemberClass && !member->GetPointer() && !member->GetReference())
                    {
                        BaseClass::MethodIterator constructor(pMemberClass, 
                            &Method::IsNormalConstructor);
                        if (++constructor)
                        {
                            Method::ArgumentIterator argument(constructor);
                            while (++argument)
                            {
                                _init += argument->GetName();
                                if (!argument.IsLast())
                                    _init += ", ";
                            }
                        }
                         _init += ")" NL;
                    }
                    else
                    {
                        MemberArgument* pMemberArgument = member->FindMemberArgument(this);
                        if (pMemberArgument)
                        {
                            _init += pMemberArgument->GetName() + ")" NL;
                        }
                        else
                        {
                            _init += member->GetName() + ")" NL;
                        }
                    }
                }
                else
                {
                    _init += member->GetInitialization() + ")" NL;
                }
                _init += member->GetEndContextImplementation();
            }
        }
    }
}//@CODE_966


void Constructor::NotifyAddMember(Member* pMember)
{//@CODE_23012
	if (_init != "\377" && !pMember->GetStatic() && !pMember->GetArray())
	{
		SaveState();
		
        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }
        
		CbString init = GetIndent();
		if (_init.IsEmpty())
			init += ": ";
		else
			init += ", ";
		init += pMember->GetPrefixedName() + "(";
		
		if (!pMember->GetInitialization().IsEmpty())
		{
			init += pMember->GetInitialization();
		}
		init += ")" NL;
		
		_init += init;
	}
}//@CODE_23012


void Constructor::NotifyRemoveMember(Member* pMember)
{//@CODE_23014
    if (_init != "\377")
    {
        CbString string = pMember->GetPrefixedName() + "(";
        int index = _init.Find(string);
        
        if (index != -1)
        {
            SaveState();
            
            if (GetPhase() > Implementation_Phase)
            {
                SetPhaseUpwards(Implementation_Phase);
            }
        
            int endIndex = _init.Find(NL, index);
            if (endIndex == -1)
            {
                _init = _init.Left(index);
                _init.TrimRight();
                int len = _init.GetLength();
                if (len)
                {
                    _init.SetAt(len-1, ' ');
                    _init.TrimRight();
                }
            }
            else
            {
                CbString rest = _init.Mid(endIndex+2);
                rest.TrimLeft();
                if (rest.GetLength())
                {
                    rest.SetAt(0, ' ');
                    rest.TrimLeft();
                    _init = _init.Left(index) + rest;
                }
                else
                {
                    _init = _init.Left(index);
                    _init.TrimRight();
                    int len = _init.GetLength();
                    if (len)
                    {
                        _init.SetAt(len-1, ' ');
                        _init.TrimRight();
                    }
                }
            }
            
            // Make sure we always end with a newline
            if (_init[_init.GetLength()-1] != '\n')
            {
                _init += NL;
            }
        }
    }
}//@CODE_23014


int Constructor::OnDelete(bool checkOnly)
{//@CODE_969
    if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete constructor '%s::%s'",
            GetBaseClass()->GetName().c_str(), GetItemText().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            // Close an open code editor first (no save prompt -- the body
            // dies with the constructor; undo restores the saved state).
            // See Method::OnDelete for the full rationale.
            if (GetOpenDialog())
                Qt_CloseCodeEditor(GetOpenDialog());
            Delete();
        }
    }

    return 1;
}//@CODE_969


int Constructor::OnEditAttributes(bool checkOnly)
{//@CODE_967
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowConstructorDialog(this, modelChanged, ownerHwnd))
    {
        if (modelChanged)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_967


int Constructor::OnOpen(bool checkOnly)
{//@CODE_968
    if (!checkOnly)
    {
        // No implementation (Implement off, incl. a "= delete"d ctor) -> there
        // is no body to edit, so open the attributes dialog instead of the code
        // editor. Mirrors Method::OnOpen.
        if (!GetImplement())
            OnEditAttributes();
        else if (GetBaseClass()->IsClass())
        {
            void* ownerHwnd = Cb_OwnerHwnd();
            Qt_ShowConstructorCodeDialog(this, ownerHwnd);
        }
        else
            OnEditAttributes();
    }

    int result = 0;
    if (GetBaseClass()->IsClass() && GetImplement())
        result = 1;

    return result;
}//@CODE_968


/*@NOTE_22961
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Constructor::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22961
    Gti::OnUndoRedoChanged(pOldState);

    Constructor* pConstructor = (Constructor*)pOldState;
    if (GetMemberAndMethodGroup())
    {
        GetMemberAndMethodGroup()->Update();
    }
    if (pConstructor && pConstructor->GetMemberAndMethodGroup())
    {
        pConstructor->GetMemberAndMethodGroup()->Update();
    }

    // The restore may have swapped the stored code/init behind an open
    // modeless code editor -- let it reload (only editors with no unsaved
    // edits). The dialog casts the old state back to Constructor.
    if (GetOpenDialog() && pConstructor)
        Qt_UndoRedoOpenCodeEditor(GetOpenDialog(), pConstructor);

}//@CODE_22961


void Constructor::ReplaceInCode(const CbString& oldString,
                                const CbString& newString)
{//@CODE_7402
    Method::ReplaceInCode(oldString, newString);
    
    ReplaceInInit(oldString, newString);
}//@CODE_7402


void Constructor::ReplaceInInit(const CbString& oldString,
                                const CbString& newString)
{//@CODE_1406
    ReplaceInStr(_init, oldString, newString);
}//@CODE_1406


void Constructor::SetItemText()
{//@CODE_16939
    CbString itemText;

    if (GetExplicit() && ExplicitAllowed())
        itemText += "explicit ";

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

    if (GetDelete())
        itemText += " = delete";

    Gti::SetItemText(itemText);
}//@CODE_16939


/*@NOTE_16936
Returns the value of member '_explicit'.
*/
bool Constructor::GetExplicit() const
{//@CODE_16936
    return _explicit;
}//@CODE_16936


/*@NOTE_16937
Set the value of member '_explicit' to 'value'.
*/
void Constructor::SetExplicit(bool value)
{//@CODE_16937
    _explicit = value;
}//@CODE_16937


const CbString& Constructor::GetInit()
{//@CODE_1124
    if (_init == "\377")
        InitInit();

    return _init;
}//@CODE_1124


void Constructor::SetInit(const CbString& rInit)
{//@CODE_1125
    if (_init != rInit)
    {
        _init = rInit;

        if (GetPhase() > Implementation_Phase)
        {
            SetPhaseUpwards(Implementation_Phase);
        }

        if (!rInit.IsEmpty())
        {
            if (rInit[rInit.GetLength()-1] != '\n')
                _init += NL;
        }
    }
}//@CODE_1125


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5294
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Constructor::CleanupReferences()
{
    Method::CleanupReferences();
}


/*@NOTE_357
Method which must be called first in a constructor
*/
void Constructor::ConstructorInclude()
{
}


/*@NOTE_359
Method which must be called first in a destructor
*/
void Constructor::DestructorInclude()
{
}


/*@NOTE_5295
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Constructor::RemoveReferences()
{
    Method::RemoveReferences();
}


/*@NOTE_5296
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Constructor::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5298
Save the state of the current object relations to pDataModelDocObject.
*/
void Constructor::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::SaveReferences(pDataModelDocObject);
}


/*@NOTE_362
Serialize the members only to a CbObject object
*/
void Constructor::Serialize(CbArchive& archive)
{
    Method::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _init;
        archive << _explicit;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _init;
            archive >> _explicit;
        }
    }
}


/*@NOTE_361
Method which must be called first in a serialize constructor
*/
void Constructor::SerializeConstructorInclude()
{
}


/*@NOTE_364
Serialize the relations to a CbObject object
*/
void Constructor::SerializeRelations(CbArchive& archive,
                                     DataModelDocObject* pointerArray[])
{
    Method::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Constructor)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
