/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Argument.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Argument'
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
#include "qt/QtArgumentDialog.h"
//@END_USER2


// Static members


Argument::Argument(Method* pMethod, Type* pType) //@INIT_779
    : Variable(pType)
    , _default("")
{//@CODE_779
    ConstructorInclude(pMethod);

    // Put in your own code
}//@CODE_779


Argument::Argument(Method* pMethod, Argument* pArgument) //@INIT_782
    : Variable(*pArgument)
    , _default(pArgument->_default)
{//@CODE_782
    ConstructorInclude(pMethod);

    // Put in your own code
}//@CODE_782


/*@NOTE_1576
Constructor needed for putting a new object in the old one's context
*/
Argument::Argument(Argument* pOld) //@INIT_1576
    : Variable(pOld)
{//@CODE_1576
    ReplaceConstructorInclude(pOld);

    _default = pOld->_default;
    _path = pOld->_path;

    // Put in your own code
}//@CODE_1576


/*@NOTE_7584
Constructor method needed to copy argument from one project to another.
*/
Argument::Argument(Method* pMethod, Type* pType,
                   Argument* pArgument) //@INIT_7584
    : Variable(pType, pArgument)
    , _default(pArgument->_default)
    , _path(pArgument->_path)
{//@CODE_7584
    ConstructorInclude(pMethod);

    // Put in your own code
}//@CODE_7584


/*@NOTE_152
Constructor needed for serialization, not meant to use for other purposes!
*/
Argument::Argument() //@INIT_152
    : Variable()
{//@CODE_152
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_152


/*@NOTE_150
Destructor method
*/
Argument::~Argument()
{//@CODE_150
    DestructorInclude();
}//@CODE_150


void Argument::Add()
{//@CODE_787
    if (!GetAdded())
    {
        SaveState(1);
        GetMethod()->AddChildLast(this);
        
        if (_default.IsEmpty())
            SetItemText(GetTypeName() + GetVariableName());
        else
            SetItemText(GetTypeName() + GetVariableName() + " = " + GetDefault());
        SetIcon(ICON_ARGUMENT);

        GetMethod()->Update();

        Gti::Add();
    }
}//@CODE_787


Argument& Argument::CopyValuesFrom(Argument& rArgument)
{//@CODE_1596
    // Remember origial name, if the name is set to empty, then we want
    // back the origial name.
    CbString orginalName = GetName();

    // Do this first before using the Variable assignment, otherwize the
    // name is already set and the code isn't updated.
    SetName(rArgument.GetName());
    Variable::CopyValuesFrom(rArgument);
    
    // Set back the original name
    if (rArgument.GetName().IsEmpty() && !orginalName.IsEmpty())
    {
        // The current name is empty and thus no rename in code is needed.
        Variable::SetName(orginalName);
    }

    _default = rArgument._default;
    _path = rArgument._path;
    
    return *this;
}//@CODE_1596


/*@NOTE_23459
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void Argument::Delete()
{//@CODE_23459
    if (!GetMethod()->IsFixed() && GetMethod()->GetBaseClass()->IsClass() &&
        GetMethod()->GetPhase() > Implementation_Phase)
    {
        GetMethod()->SetPhaseUpwards(Implementation_Phase);
    }
        
    DataModelDocObject::Delete();
}//@CODE_23459


bool Argument::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1331
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = true;
    }
    else
    {
        Method* pMethod = GetMethod();
        if (!pMethod->IsFixed() && 
             pMethod->GetArgumentCount() > 1)
        {
            if (pMethod->GetPrevArgument(this))
                pGtiDropDefault = pMethod->GetPrevArgument(this);
            else
                pGtiDropDefault = pMethod;

            Remove();
            value = true;
        }
    }

    return value;
}//@CODE_1331


void Argument::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1337
    int argumentCount = 0;
    Argument** pArgumentArray = 0;
    Method** pMethodArray = 0;

    if (ctrlKeyDown)
    {
        Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
        if (pDropMethod && !pDropMethod->IsFixed())
        {
            // In case of virtual function, adjust virtual overides also.
            if (pDropMethod->GetVirtual())
            {
                pArgumentArray = 
                    new Argument*[GetDataModelDoc()->GetTypeCount()];
                pMethodArray = 
                    new Method*[GetDataModelDoc()->GetTypeCount()];

                DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsExternClass);
                while (++iType)
                {
                    ExternClass* pDerivedClass = (ExternClass*)iType.Get();

                    if (pDerivedClass->IsBaseClass(GetMethod()->GetBaseClass()))
                    {
                        Method* pMethod = pDerivedClass->FindSimilarMethod(pDropMethod);
                        if (pMethod)
                        {
                            pMethodArray[argumentCount] = pMethod;
                            pArgumentArray[argumentCount++] = 
                                pMethod->GetArgumentAtSamePosition(this);
                        }
                    }
                }
            }

            Argument* pNewArgument = new Argument(pDropMethod, this);
            pNewArgument->Add();

            for (int i = 0; i < argumentCount; i++)
            {
                pNewArgument = new Argument(pMethodArray[i], this);
                pNewArgument->Add();
            }
        }
    }
    else
    {
        // In case of virtual function, adjust virtual overides also.
        if (GetMethod()->GetVirtual())
        {
            pArgumentArray = 
                new Argument*[GetDataModelDoc()->GetTypeCount()];
            pMethodArray = 
                new Method*[GetDataModelDoc()->GetTypeCount()];

            DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsExternClass);
            while (++iType)
            {
                ExternClass* pDerivedClass = (ExternClass*)iType.Get();

                if (pDerivedClass->IsBaseClass(GetMethod()->GetBaseClass()))
                {
                    Method* pMethod = pDerivedClass->FindSimilarMethod(GetMethod());
                    if (pMethod)
                    {
                        pMethodArray[argumentCount] = pMethod;
                        pArgumentArray[argumentCount++] = 
                            pMethod->GetArgumentAtSamePosition(this);
                    }
                }
            }
        }

        Method* pMethod = GetMethod();

        Argument* pDropArgument = dynamic_cast<Argument*>(pGtiDrop);
        Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
        if (pDropMethod)
        {
            for (int i = 0; i < argumentCount; i++)
            {
                pArgumentArray[i]->SaveState(1);
                pMethodArray[i]->MoveArgumentFirst(pArgumentArray[i]);
                pMethodArray[i]->Update();
            }
            pMethod->MoveArgumentFirst(this);
        }
        else if (pDropArgument)
        {
            for (int i = 0; i < argumentCount; i++)
            {
                pArgumentArray[i]->SaveState(1);
                pMethodArray[i]->MoveArgumentAfter(pArgumentArray[i], 
                    pMethodArray[i]->GetArgumentAtSamePosition(pDropArgument));
                pMethodArray[i]->Update();
            }
            SaveState(1);
            pMethod->MoveArgumentAfter(this, pDropArgument);
        }
        Add();
    }

    delete pArgumentArray;
    delete pMethodArray;
}//@CODE_1337


bool Argument::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1334
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
        if (pDropMethod && 
            !pDropMethod->IsFixed() &&
            !pDropMethod->IsMemberMethod() && 
            !pDropMethod->IsFindMethod() && 
            !pDropMethod->IsDestructor())
            value = true;
    }
    else
    {
        Argument* pDropArgument = dynamic_cast<Argument*>(pGtiDrop);
        Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
        if ((pDropArgument && pDropArgument->GetMethod() == GetMethod()) ||
            (pDropMethod && pDropMethod == GetMethod()))
        {
            value = true;
        }
    }

    return value;
}//@CODE_1334


int Argument::IsSimilar(Argument* pArgument)
{//@CODE_785
    // similar, not exactly the same object !!
    if (this != pArgument && GetType() == pArgument->GetType() && 
        Variable::IsSimilar(pArgument))
    {
        return 1;
    }

    return 0;
}//@CODE_785


int Argument::OnDelete(bool checkOnly)
{//@CODE_790
    if (GetMethod()->IsMemberMethod())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete an argument of a member method", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (GetMethod()->IsFixedMethod())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete an argument of a fixed method", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (GetMethod()->IsFixed())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete an argument of a fixed find method", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (GetMethod()->IsMacroMethod())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete an argument of a relation method", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete argument '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            // In case of virtual function, adjust virtual overides also.
            if (GetMethod()->GetVirtual())
            {
                DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsExternClass);
                while (++iType)
                {
                    ExternClass* pDerivedClass = (ExternClass*)iType.Get();

                    if (pDerivedClass->IsBaseClass(GetMethod()->GetBaseClass()))
                    {
                        Method* pMethod = pDerivedClass->FindSimilarMethod(GetMethod());
                        if (pMethod)
                        {
                            pMethod->GetArgumentAtSamePosition(this)->Delete();
                        }
                    }
                }
            }
            
            Delete();
        }
    }
    
    return 1;
}//@CODE_790


int Argument::OnEditAttributes(bool checkOnly)
{//@CODE_789
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;

    if (Qt_ShowArgumentDialog(this, changed, ownerHwnd))
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
}//@CODE_789


/*@NOTE_22948
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Argument::OnUndoRedoAdded()
{//@CODE_22948
    Gti::OnUndoRedoAdded();
    
    GetMethod()->Update();

}//@CODE_22948


/*@NOTE_22949
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Argument::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22949
    Gti::OnUndoRedoChanged(pOldState);
        
    GetMethod()->Update();
}//@CODE_22949


/*@NOTE_22936
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called after the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Argument::OnUndoRedoRemoved()
{//@CODE_22936
    GetMethod()->Update();
}//@CODE_22936


void Argument::ReplaceInPath(const CbString& oldString,
                             const CbString& newString)
{//@CODE_35777
    ReplaceInStr(_path, oldString, newString);
}//@CODE_35777


void Argument::SetName(const CbString& rName)
{//@CODE_1396
    SaveState();

    if (GetName().IsEmpty() || rName.IsEmpty())
    {
        Variable::SetName(rName);
    }
    else
    {
        if (rName != GetName())
        {
            // Do modification on code of methods first
            GetMethod()->ReplaceInCode(GetName(), rName);
            Variable::SetName(rName);
        }
    }
}//@CODE_1396


void Argument::Update()
{//@CODE_788
    if (GetAdded())
    {
        if (_default.IsEmpty())
            SetItemText(GetTypeName() + GetVariableName());
        else
            SetItemText(GetTypeName() + GetVariableName() + " = " + GetDefault());
        SetIcon(ICON_ARGUMENT);

        GetMethod()->Update();

        Gti::Update();
    }
}//@CODE_788


void Argument::UpdatePhaseMethod()
{//@CODE_23466
    if (GetMethod()->IsAllowedToEditPhase(Implementation_Phase) &&
        GetMethod()->GetPhase() > Implementation_Phase)
    {
        GetMethod()->SetPhaseUpwards(Implementation_Phase);
    }
}//@CODE_23466


const CbString& Argument::GetDefault()
{//@CODE_1083
    return _default;
}//@CODE_1083


void Argument::SetDefault(const CbString& rDefault)
{//@CODE_1084
    _default = rDefault;
}//@CODE_1084


/*@NOTE_1668
Returns the value of member '_path'.
*/
const CbString& Argument::GetPath()
{//@CODE_1668
    return _path;
}//@CODE_1668


/*@NOTE_1669
Set the value of member '_path' to 'rPath'.
*/
void Argument::SetPath(const CbString& rPath)
{//@CODE_1669
    _path = rPath;
}//@CODE_1669


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5270
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Argument::CleanupReferences()
{
    Variable::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_149
Method which must be called first in a constructor
*/
void Argument::ConstructorInclude(Method* pMethod)
{
    INIT_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_151
Method which must be called first in a destructor
*/
void Argument::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_5271
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Argument::RemoveReferences()
{
    Variable::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_1578
Method which must be called first in a replace constructor
*/
void Argument::ReplaceConstructorInclude(Argument* pOld)
{
    REPLACE_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_5272
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Argument::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Argument* pArgument = (Argument*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
    Variable::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5274
Save the state of the current object relations to pDataModelDocObject.
*/
void Argument::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Variable::SaveReferences(pDataModelDocObject);
    Argument* pArgument = (Argument*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_154
Serialize the members only to a CbObject object 
*/
void Argument::Serialize(CbArchive& archive)
{
    Variable::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _default;
        archive << _path;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _default;
            archive >> _path;
        }
    }
}


/*@NOTE_153
Method which must be called first in a serialize constructor
*/
void Argument::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Method, Method, Argument, Argument)
}


/*@NOTE_156
Serialize the relations to a CbObject object
*/
void Argument::SerializeRelations(CbArchive& archive,
                                  DataModelDocObject* pointerArray[])
{
    Variable::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Argument)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
