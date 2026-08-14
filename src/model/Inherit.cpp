/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Inherit.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Inherit'
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
#include "qt/QtInheritDialog.h"
//@END_USER2


// Static members


Inherit::Inherit(ExternClass* pExternClass, BaseClass* pBaseClass,
                 AccessType access) //@INIT_896
    : Gti(pExternClass->GetDataModelDoc())
    , _access(access)
    , _virtual(0)
    , _template(pBaseClass->GetTemplate())
{//@CODE_896
    ConstructorInclude(pBaseClass, pExternClass);

    // Put in your own code
    ClassDiagram::AddInherit(this);
}//@CODE_896


/*@NOTE_7557
Constructor method needed to copy Inherit from one project to another.
*/
Inherit::Inherit(ExternClass* pExternClass, BaseClass* pBaseClass,
                 Inherit* pInherit) //@INIT_7557
    : Gti(pExternClass->GetDataModelDoc())
    , _access(pInherit->_access)
    , _note(pInherit->_note)
    , _virtual(pInherit->_virtual)
    , _template(pInherit->_template)
{//@CODE_7557
    ConstructorInclude(pBaseClass, pExternClass);

    // Put in your own code
}//@CODE_7557


/*@NOTE_243
Constructor needed for serialization, not meant to use for other purposes!
*/
Inherit::Inherit() //@INIT_243
    : Gti()
    , _virtual(0)
{//@CODE_243
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_243


/*@NOTE_241
Destructor method
*/
Inherit::~Inherit()
{//@CODE_241
    DestructorInclude();

    // Put in your own code
}//@CODE_241


void Inherit::Add()
{//@CODE_900
    if (!GetAdded())
    {
        SaveState(1);
        GetExternClass()->AddChildLast(this);
        SetIcon(ICON_INHERIT);
        SetItemText(GetBaseClass()->Type::GetName() + GetTemplate());

        Gti::Add();
    }
}//@CODE_900


/*@NOTE_5820
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void Inherit::Delete()
{//@CODE_5820
    int version = GetDataModelDoc()->GetVersion();
    if (GetVersion() <= version && GetExternClass()->IsClass())
    {
        GetExternClass()->SetVersion(version + 1);

        CbString str;
        str.Format("@Deleted inheritance '%s'", GetBaseName().c_str());
        ((Class*)GetExternClass())->AddModified(str);
    }

	if (GetExternClass()->IsClass())
	{
        BaseClass::MethodIterator iConstructor(GetExternClass(), &Method::IsConstructor);
		while (++iConstructor)
		{
            // Constructors at are effected, check phase is at most
            // Implementation_Phase 
            if (iConstructor->GetPhase() > Implementation_Phase)
            {
                iConstructor->SetPhaseUpwards(Implementation_Phase);
            }
		}
	}

    DataModelDocObject::Delete();
}//@CODE_5820


InheritShape* Inherit::FindInheritShape(ClassDiagram* pClassDiagram)
{//@CODE_4119
    InheritShapeIterator iBaseInheritShape(this);
    while (++iBaseInheritShape)
    {
        if (pClassDiagram == iBaseInheritShape->GetClassDiagram())
        {
            return iBaseInheritShape;
        }
    }

    return 0;
}//@CODE_4119


CbString Inherit::GetBaseName()
{//@CODE_7469
    return GetBaseClass()->Type::GetName() + GetTemplate();
}//@CODE_7469


int Inherit::OnDelete(bool checkOnly)
{//@CODE_903
    if (GetExternClass()->IsClass() &&
		((Class*)GetExternClass())->GetSerialize())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete inheritance", CBMB_ICONEXCLAMATION);

        return 0;
    }
    else if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you don't want to inherit from '%s'", 
            GetBaseName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }
    
    return 1;
}//@CODE_903


int Inherit::OnEditAttributes(bool checkOnly)
{//@CODE_902
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;
    if (Qt_ShowInheritDialog(this, changed, ownerHwnd))
    {
        if (changed)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows
            // the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_902


void Inherit::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23081
    if (ReplaceInStr(_template, oldString, newString))
    {
        Update();
    }
}//@CODE_23081


bool Inherit::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40839
    if (!pTreeViewModel->GetShowInheritance())
    {
        return false;
    }

    return Gti::ShownByFilter(pTreeViewModel);
}//@CODE_40839


void Inherit::Update()
{//@CODE_901
    if (GetAdded())
    {
        if (GetParent() == GetExternClass())
        {
            SetItemText(GetBaseClass()->Type::GetName() + GetTemplate());
            Gti::Update();
        }
        else
        {
            Remove();
            Add();
        }
    }
}//@CODE_901


AccessType Inherit::GetAccess()
{//@CODE_1178
    return _access;
}//@CODE_1178


void Inherit::SetAccess(AccessType access)
{//@CODE_1179
    _access = access;
}//@CODE_1179


const CbString& Inherit::GetNote()
{//@CODE_1442
    return _note;
}//@CODE_1442


void Inherit::SetNote(const CbString& rNote)
{//@CODE_1443
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_1443


/*@NOTE_7445
Returns the value of member '_template'.
*/
CbString Inherit::GetTemplate()
{//@CODE_7445
    return DataModel::ConvertToHtmlStringIfNeeded(_template);
}//@CODE_7445


/*@NOTE_7446
Set the value of member '_template' to 'rTemplate'.
*/
void Inherit::SetTemplate(const CbString& rTemplate)
{//@CODE_7446
    _template = rTemplate;
}//@CODE_7446


/*@NOTE_1664
Returns the value of member '_virtual'.
*/
bool Inherit::GetVirtual()
{//@CODE_1664
    return _virtual;
}//@CODE_1664


/*@NOTE_1665
Set the value of member '_virtual' to 'virtual'.
*/
void Inherit::SetVirtual(bool val)
{//@CODE_1665
    _virtual = val;
}//@CODE_1665


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5456
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Inherit::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    CLEANUP_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_240
Method which must be called first in a constructor
*/
void Inherit::ConstructorInclude(BaseClass* pBaseClass,
                                 ExternClass* pExternClass)
{
    INIT_MULTI_OWNED_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
    INIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    INIT_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_242
Method which must be called first in a destructor
*/
void Inherit::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
    EXIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    EXIT_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_5457
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Inherit::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
    Gti::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
    REMOVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
}


/*@NOTE_5458
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Inherit::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Inherit* pInherit = (Inherit*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    RESTORE_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5460
Save the state of the current object relations to pDataModelDocObject.
*/
void Inherit::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    Inherit* pInherit = (Inherit*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    SAVE_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_245
Serialize the members only to a CbObject object
*/
void Inherit::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _access;
        archive << _note;
        archive << _virtual;
        archive << _template;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _access;
            archive >> _note;
            archive >> _virtual;
            archive >> _template;
        }
    }
}


/*@NOTE_244
Method which must be called first in a serialize constructor
*/
void Inherit::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
    INIT_MULTI_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    INIT_MULTI_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)
}


/*@NOTE_247
Serialize the relations to a CbObject object
*/
void Inherit::SerializeRelations(CbArchive& archive,
                                 DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Inherit)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
METHODS_ITERATOR_MULTI_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
METHODS_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
METHODS_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
