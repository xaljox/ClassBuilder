/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          OtherType.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'OtherType'
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
#include "qt/QtTypeDialog.h"
//@END_USER2


// Static members


OtherType::OtherType(DataModelDoc* pDataModelDoc) //@INIT_704
    : Type(pDataModelDoc)
    , _declaration("")
    , _serializeMap(0)
{//@CODE_704
    ConstructorInclude();

    // Put in your own code
}//@CODE_704


OtherType::OtherType(ExternClass* pOld) //@INIT_7517
    : Type(pOld)
    , _declaration("")
    , _serializeMap(0)
{//@CODE_7517
    ConstructorInclude();

    // Put in your own code
    delete pOld;
}//@CODE_7517


/*@NOTE_7540
Constructor method needed to copy OtherType from one project to the other.
*/
OtherType::OtherType(DataModelDoc* pDataModelDoc,
                     OtherType* pOtherType) //@INIT_7540
    : Type(pDataModelDoc, pOtherType)
    , _declaration(pOtherType->_declaration)
    , _serializeMap(pOtherType->_serializeMap)
{//@CODE_7540
    ConstructorInclude();

    // Put in your own code
}//@CODE_7540


/*@NOTE_87
Constructor needed for serialization, not meant to use for other purposes!
*/
OtherType::OtherType() //@INIT_87
    : Type()
    , _declaration("")
    , _serializeMap(0)
{//@CODE_87
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_87


/*@NOTE_85
Destructor method
*/
OtherType::~OtherType()
{//@CODE_85
    DestructorInclude();

    // Put in your own code
}//@CODE_85


void OtherType::Add()
{//@CODE_706
    if (!GetAdded() && !GetName().IsEmpty() && GetName() != "...")
    {
        SaveState(1);
        GetDataModelDoc()->GetOtherTypes()->AddChildLast(this);
        SetItemText(GetName());
        SetIcon(ICON_TYPE);

        Gti::Add();
    }
}//@CODE_706


bool OtherType::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_7508
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = Type::Drag(ctrlKeyDown, pGtiDropDefault);
    }
    else
    {
        pGtiDropDefault = GetDataModelDoc()->GetOtherTypes();
		DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsOtherType, this);
        if (--iType)
            pGtiDropDefault = iType.Get();

        Remove();
        value = true;
    }

    return value;
}//@CODE_7508


void OtherType::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_7511
    if (ctrlKeyDown)
    {
        Type::Drop(ctrlKeyDown, pGtiDrop);
    }
    else
    {
        ExternClasses* pExternClasses = dynamic_cast<ExternClasses*>(pGtiDrop);
        
        if (pExternClasses)
        {
            CbString str;
            str.Format("Are you sure you want to promote other type '%s' into an external class, this action can not be undone!!", 
                GetName().c_str());
            if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_OKCANCEL) == CBMB_IDOK)
            {
                ExternClass* pExternClass = new ExternClass(this);
                pExternClass->Add();
                pExternClass->DataModelDocObject::GetDataModelDoc()->DeleteAllUndoBase();
                pExternClass->DataModelDocObject::GetDataModelDoc()->DeleteAllRedoBase();
            }
            else
            {
                Add();
                GetDataModelDoc()->RollBack();
            }
        }
        else
        {
            OtherTypes* pOtherTypes = dynamic_cast<OtherTypes*>(pGtiDrop);
            OtherType* pOtherType = dynamic_cast<OtherType*>(pGtiDrop);
            if (pOtherTypes)
            {
                //SaveState(1);
                GetDataModelDoc()->MoveTypeFirst(this);
            }
            else if (pOtherType)
            {
                //SaveState(1);
                GetDataModelDoc()->MoveTypeAfter(this, pOtherType);
            }
            Add();
        }
    }
}//@CODE_7511


bool OtherType::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_7514
    bool value = false;

    if (ctrlKeyDown)
    {
        value = Type::DropTarget(ctrlKeyDown, pGtiDrop);
    }
    else
    {
        if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
            return value;

        if (pGtiDrop->IsOtherTypes() || pGtiDrop->IsOtherType() || pGtiDrop->IsExternClasses())
        {
            value = true;
        }
    }

    return value;

}//@CODE_7514


int OtherType::OnDelete(bool checkOnly)
{//@CODE_709
    if (GetVariableCount())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete a type which is referenced", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (GetName() == "void")
    {
        if (!checkOnly)
            CbMessageBox("Can not delete type 'void'", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (GetName() == "int")
    {
        if (!checkOnly)
            CbMessageBox("Can not delete type 'int'", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (GetName() == "")
    {
        if (!checkOnly)
            CbMessageBox("Can not delete type ''", CBMB_ICONEXCLAMATION);
        
        return 0;
    }
    else if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete type '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }
    
    return 1;
}//@CODE_709


int OtherType::OnEditAttributes(bool checkOnly)
{//@CODE_708
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowTypeDialog(this, modelChanged, ownerHwnd))
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
}//@CODE_708


/*@NOTE_23235
Virtual method to replace strings at various places, called if a type name changes.
*/
void OtherType::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23235
    ReplaceInStr(_declaration, oldString, newString);
}//@CODE_23235


void OtherType::Update()
{//@CODE_707
    if (GetAdded())
    {
        // A type rename cascades Update() to EVERY variable of this type (their
        // tree text shows the type name) + every method using it in a throw
        // list. Each Update() fires NotifyStructureChanged = a full Qt tree rebuild, so
        // for a widely-used type that was hundreds of rebuilds (the ~40s rename).
        // Lock once so the whole cascade collapses to a single refresh.
        CbViewLock lock(GetDataModelDoc());

        SetItemText(GetName());

        Gti::Update();

        Type::VariableIterator iVariable(this);
        while (++iVariable)
            iVariable->Update();

        Type::ExceptionSpecificationTypeIterator iExceptionSpecificationType(this);
        while (++iExceptionSpecificationType)
            iExceptionSpecificationType->GetExceptionSpecification()->GetMethod()->Update();
    }
}//@CODE_707


/*@NOTE_1660
Returns the value of member '_declaration'.
*/
const CbString& OtherType::GetDeclaration()
{//@CODE_1660
    return _declaration;
}//@CODE_1660


/*@NOTE_1661
Set the value of member '_declaration' to 'rDeclaration'.
*/
void OtherType::SetDeclaration(const CbString& rDeclaration)
{//@CODE_1661
    _declaration = rDeclaration;
    if (!rDeclaration.IsEmpty())
    {
        if (rDeclaration[rDeclaration.GetLength()-1] != '\n')
            _declaration += NL;
    }
}//@CODE_1661


/*@NOTE_3157
Returns the value of member '_serializeMap'.
*/
bool OtherType::GetSerializeMap()
{//@CODE_3157
    return _serializeMap;
}//@CODE_3157


/*@NOTE_3158
Set the value of member '_serializeMap' to 'serializeMap'.
*/
void OtherType::SetSerializeMap(bool serializeMap)
{//@CODE_3158
    _serializeMap = serializeMap;
}//@CODE_3158


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5498
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void OtherType::CleanupReferences()
{
    Type::CleanupReferences();
}


/*@NOTE_84
Method which must be called first in a constructor
*/
void OtherType::ConstructorInclude()
{
}


/*@NOTE_86
Method which must be called first in a destructor
*/
void OtherType::DestructorInclude()
{
}


/*@NOTE_5499
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void OtherType::RemoveReferences()
{
    Type::RemoveReferences();
}


/*@NOTE_5500
Bring the current object relations into the same state as pDataModelDocObject.
*/
void OtherType::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Type::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5502
Save the state of the current object relations to pDataModelDocObject.
*/
void OtherType::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Type::SaveReferences(pDataModelDocObject);
}


/*@NOTE_89
Serialize the members only to a CbObject object
*/
void OtherType::Serialize(CbArchive& archive)
{
    Type::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _declaration;
        archive << _serializeMap;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _declaration;
            archive >> _serializeMap;
        }
    }
}


/*@NOTE_88
Method which must be called first in a serialize constructor
*/
void OtherType::SerializeConstructorInclude()
{
}


/*@NOTE_91
Serialize the relations to a CbObject object
*/
void OtherType::SerializeRelations(CbArchive& archive,
                                   DataModelDocObject* pointerArray[])
{
    Type::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(OtherType)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
