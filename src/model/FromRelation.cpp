/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FromRelation.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'FromRelation'
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
#include "qt/QtRelationDialog.h"
//@END_USER2


// Static members


/*@NOTE_217
Constructor needed for serialization, not meant to use for other purposes!
*/
FromRelation::FromRelation() //@INIT_217
    : Gti()
{//@CODE_217
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_217


FromRelation::FromRelation(Relation* pRelation) //@INIT_878
    : Gti(pRelation->GetDataModelDoc())
{//@CODE_878
    ConstructorInclude(pRelation);

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_878


/*@NOTE_215
Destructor method
*/
FromRelation::~FromRelation()
{//@CODE_215
    DestructorInclude();

    // Put in your own code
}//@CODE_215


void FromRelation::Add()
{//@CODE_882
    if (!GetAdded())
    {
        SaveState(1);
        GetRelation()->GetFromClass()->AddChildLast(this);
        SetIcon();
        SetItemText();

        Gti::Add();

        MethodIterator method(this);
        while (++method)
            method->Add();
        
        if (GetFromRelationMacroMethods())
            GetFromRelationMacroMethods()->Add();
    }
}//@CODE_882


int FromRelation::OnAddMethod(bool checkOnly)
{//@CODE_886
    if (GetRelation()->GetMulti())
    {
        if (!checkOnly)
        {
            GetDataModelDoc()->MarkLastUndo();
            FindMethod* pFindMethod = new FindMethod(this);
            
            if (pFindMethod->OnEditAttributes())
            {
                pFindMethod->Add();
                pFindMethod->GetBaseClass()->NotifyAddMethod(pFindMethod);
            }
            else
                GetDataModelDoc()->RollBack();
        }
        
        return 1;
    }
    else
        return 0;
}//@CODE_886


int FromRelation::OnDelete(bool checkOnly)
{//@CODE_885
    if (GetRelation()->GetFromClass() == GetDataModelDoc()->GetDataModel()->GetDocument() &&
        GetRelation()->GetToClass() == GetDataModelDoc()->GetDataModel()->GetDocumentObject() &&
        GetRelation()->GetFromClass()->GetFirstFromRelation() == GetRelation())
    {
        if (!checkOnly)
            CbMessageBox("Can not delete this relation", CBMB_ICONEXCLAMATION);

        return 0;
    }
    else if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete relation '%s'", 
            GetRelation()->GetNotation().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            GetRelation()->Delete();
        }
    }

    return 1;
}//@CODE_885


int FromRelation::OnEditAttributes(bool checkOnly)
{//@CODE_884
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool changed = false;
    if (Qt_ShowRelationDialog(GetRelation(), changed, ownerHwnd))
    {
        if (changed)
        {
            // Relation::Update() rebuilds the relation's whole tree/diagram
            // representation (a refresh per touched method/shape). The CbViewLock
            // coalesces that AND shows the wait cursor (so no manual SetCursor).
            CbViewLock lock(GetDataModelDoc());
            GetRelation()->Update();
        }

        return 1;
    }

    return 0;
}//@CODE_884


void FromRelation::SetIcon()
{//@CODE_880
    int icon = ICON_SINGLE_ACT;
    if (GetRelation()->GetOwned())
        icon += 2;
    if (GetRelation()->GetMulti())
        icon += 4;
    if (GetRelation()->GetStatic())
        icon += 4;
    if (GetRelation()->GetCritical())
        icon += 12;
    Gti::SetIcon(icon);
}//@CODE_880


void FromRelation::SetItemText()
{//@CODE_881
    CbString itemText;
    if (GetRelation()->GetToClassName() == GetRelation()->GetToName())
        itemText = GetRelation()->GetToName();
    else
    {
        itemText = GetRelation()->GetToName() + 
                   " (" + GetRelation()->GetToClassName() + ")";
    }
    Gti::SetItemText(itemText);
}//@CODE_881


bool FromRelation::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40835
    Relation* pRelation = GetRelation();
    // Static relations are always shown -- there are few and they are easy to
    // spot when the rest is filtered out, so they bypass the cardinality/
    // aggregation gate entirely.
    if (pRelation && !pRelation->GetStatic())
    {
        bool cardOk = pRelation->GetMulti()
            ? pTreeViewModel->GetShowMultiRelations()
            : pTreeViewModel->GetShowSingleRelations();
        bool aggOk = pRelation->GetOwned()
            ? pTreeViewModel->GetShowAggregationRelations()
            : pTreeViewModel->GetShowNonAggregationRelations();
        if (!cardOk || !aggOk)
        {
            return false;
        }
    }

    return Gti::ShownByFilter(pTreeViewModel);
}//@CODE_40835


void FromRelation::Update()
{//@CODE_883
    if (GetAdded())
    {
        SaveState(1);
        SetIcon();
        SetItemText();

        Gti::Update();

        MethodIterator method(this);
        while (++method)
            method->Update();
        
        if (GetFromRelationMacroMethods())
            GetFromRelationMacroMethods()->Update();
    }
}//@CODE_883


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5426
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void FromRelation::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)
}


/*@NOTE_214
Method which must be called first in a constructor
*/
void FromRelation::ConstructorInclude(Relation* pRelation)
{
    INIT_MULTI_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
    INIT_SINGLE_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    INIT_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)
}


/*@NOTE_216
Method which must be called first in a destructor
*/
void FromRelation::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
    EXIT_SINGLE_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    EXIT_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)
}


/*@NOTE_5427
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void FromRelation::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    REMOVE_MULTI_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
    Gti::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)
}


/*@NOTE_5428
Bring the current object relations into the same state as pDataModelDocObject.
*/
void FromRelation::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FromRelation* pFromRelation = (FromRelation*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5430
Save the state of the current object relations to pDataModelDocObject.
*/
void FromRelation::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    FromRelation* pFromRelation = (FromRelation*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)
}


/*@NOTE_219
Serialize the members only to a CbObject object
*/
void FromRelation::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_218
Method which must be called first in a serialize constructor
*/
void FromRelation::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
    INIT_SINGLE_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    INIT_SINGLE_PASSIVE(Relation, Relation, FromRelation, FromRelation)
}


/*@NOTE_221
Serialize the relations to a CbObject object
*/
void FromRelation::SerializeRelations(CbArchive& archive,
                                      DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
        WRITE_SINGLE_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
            READ_SINGLE_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(FromRelation)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
METHODS_ITERATOR_MULTI_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
METHODS_SINGLE_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
METHODS_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
