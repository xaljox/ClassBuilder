/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ToRelation.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ToRelation'
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


ToRelation::ToRelation(Relation* pRelation) //@INIT_887
    : Gti(pRelation->GetDataModelDoc())
{//@CODE_887
    ConstructorInclude(pRelation);

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_887


/*@NOTE_230
Constructor needed for serialization, not meant to use for other purposes!
*/
ToRelation::ToRelation() //@INIT_230
    : Gti()
{//@CODE_230
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_230


/*@NOTE_228
Destructor method
*/
ToRelation::~ToRelation()
{//@CODE_228
    DestructorInclude();

    // Put in your own code
}//@CODE_228


void ToRelation::Add()
{//@CODE_891
    if (!GetAdded())
    {
        SaveState(1);
        GetRelation()->GetToClass()->AddChildLast(this);
        SetIcon();
        SetItemText();

        Gti::Add();
        
        if (GetToRelationMacroMethods())
            GetToRelationMacroMethods()->Add();
    }
}//@CODE_891


int ToRelation::OnDelete(bool checkOnly)
{//@CODE_894
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
}//@CODE_894


int ToRelation::OnEditAttributes(bool checkOnly)
{//@CODE_893
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
}//@CODE_893


void ToRelation::SetIcon()
{//@CODE_889
    int icon = ICON_SINGLE_PAS;
    if (GetRelation()->GetOwned())
        icon += 2;
    if (GetRelation()->GetMulti())
        icon += 4;
    if (GetRelation()->GetStatic())
        icon += 4;
    if (GetRelation()->GetCritical())
        icon += 12;
    Gti::SetIcon(icon);
}//@CODE_889


void ToRelation::SetItemText()
{//@CODE_890
    CbString itemText;
    if (GetRelation()->GetFromClassName() == GetRelation()->GetFromName())
        itemText = GetRelation()->GetFromName();
    else
    {
        itemText = GetRelation()->GetFromName() + 
                   " (" + GetRelation()->GetFromClassName() + ")";
    }
    Gti::SetItemText(itemText);
}//@CODE_890


bool ToRelation::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40837
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
}//@CODE_40837


void ToRelation::Update()
{//@CODE_892
    if (GetAdded())
    {
        SaveState(1);
        SetIcon();
        SetItemText();

        Gti::Update();
        
        if (GetToRelationMacroMethods())
            GetToRelationMacroMethods()->Update();
    }
}//@CODE_892


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5558
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ToRelation::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)
}


/*@NOTE_227
Method which must be called first in a constructor
*/
void ToRelation::ConstructorInclude(Relation* pRelation)
{
    INIT_SINGLE_OWNED_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    INIT_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)
}


/*@NOTE_229
Method which must be called first in a destructor
*/
void ToRelation::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    EXIT_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)
}


/*@NOTE_5559
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ToRelation::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    Gti::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)
}


/*@NOTE_5560
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ToRelation::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ToRelation* pToRelation = (ToRelation*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5562
Save the state of the current object relations to pDataModelDocObject.
*/
void ToRelation::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    ToRelation* pToRelation = (ToRelation*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)
}


/*@NOTE_232
Serialize the members only to a CbObject object
*/
void ToRelation::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_231
Method which must be called first in a serialize constructor
*/
void ToRelation::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    INIT_SINGLE_PASSIVE(Relation, Relation, ToRelation, ToRelation)
}


/*@NOTE_234
Serialize the relations to a CbObject object
*/
void ToRelation::SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ToRelation)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
METHODS_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
