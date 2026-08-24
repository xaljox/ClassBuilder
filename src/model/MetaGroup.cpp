/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MetaGroup.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MetaGroup'
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


/*@NOTE_28964
Constructor needed for serialization, not meant to use for other purposes!
*/
MetaGroup::MetaGroup() //@INIT_28964
    : Group()
{//@CODE_28964
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_28964


/*@NOTE_29588
Constructor method.
*/
MetaGroup::MetaGroup(DataModel* pDataModel) //@INIT_29588
    : Group(pDataModel->GetDataModelDoc())
{//@CODE_29588
    ConstructorInclude(pDataModel);

    // Put in your own code
}//@CODE_29588


/*@NOTE_28962
Destructor method.
*/
MetaGroup::~MetaGroup()
{//@CODE_28962
    DestructorInclude();

    // Put in your own code
}//@CODE_28962


void MetaGroup::Add()
{//@CODE_29576
    if (!GetAdded())
    {
        SetItemText(GetName());
        SetIcon(ICON_FILE);

        Gti::Add();

        Gti::ChildIterator iClassDiagram(this, &Gti::IsClassDiagram);
        while (++iClassDiagram)
            iClassDiagram->Add();

        Gti::ChildIterator iSequenceDiagram(this, &Gti::IsSequenceDiagram);
        while (++iSequenceDiagram)
            iSequenceDiagram->Add();
        
        ClassGroupIterator iClassGroup(this);
        while (++iClassGroup)
            iClassGroup->Add();
    }
}//@CODE_29576


bool MetaGroup::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_29590
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        pGtiDropDefault = GetDataModel()->GetPrevMetaGroup(this);
        if (!pGtiDropDefault)
            pGtiDropDefault = GetDataModel();
        
        Remove();
        value = true;
    }

    return value;
}//@CODE_29590


void MetaGroup::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_29593
    if (ctrlKeyDown)
    {
    }
    else
    {
        CbViewLock lock(GetDataModelDoc());
        
        DataModel* pDataModel = GetDataModel();
        DataModel* pDropDataModel = dynamic_cast<DataModel*>(pGtiDrop);
        MetaGroup* pDropMetaGroup = dynamic_cast<MetaGroup*>(pGtiDrop);
        if (pDropDataModel)
            pDataModel->MoveMetaGroupFirst(this);
        else if (pDropMetaGroup)
            pDataModel->MoveMetaGroupAfter(this, pDropMetaGroup);
        
        int i = 0;
        DataModel::MetaGroupIterator iMetaGroup(pDataModel);
        while (++iMetaGroup)
        {
            iMetaGroup->SaveState(1);
            iMetaGroup->SetOrder(i++);
        }
        Add();
        
    }
}//@CODE_29593


bool MetaGroup::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_29596
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
    }
    else
    {
        if (pGtiDrop->IsDataModel() || pGtiDrop->IsMetaGroup())
        {
            value = true;
        }
    }

    return value;
}//@CODE_29596


Gti* MetaGroup::GetNext(Gti* pGti)
{//@CODE_35373
    Gti* pNextGti = Gti::GetNext(pGti);

    if (!pNextGti)
    {
        pNextGti = GetDataModelDoc()->GetDataModel()->GetNextMetaGroup(this);
    }
    
    if (!pNextGti)
    {
        pNextGti = GetDataModelDoc()->GetActors();
    }
    
    return pNextGti;
}//@CODE_35373


int MetaGroup::OnAddClassDiagram(bool checkOnly)
{//@CODE_29577
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
}//@CODE_29577


int MetaGroup::OnAddGroup(bool checkOnly)
{//@CODE_29579
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        ClassGroup* pClassGroup = new ClassGroup(this);
        pClassGroup->SetOrder(GetClassGroupCount()-1); // Make it the last in the tree view

        if (pClassGroup->OnEditAttributes())
            pClassGroup->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_29579


int MetaGroup::OnAddMetaGroup(bool checkOnly)
{//@CODE_29604
    return GetDataModel()->OnAddMetaGroup(checkOnly);
}//@CODE_29604


int MetaGroup::OnAddSequenceDiagram(bool checkOnly)
{//@CODE_30483
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
}//@CODE_30483


int MetaGroup::OnDelete(bool checkOnly)
{//@CODE_29606
    if (!checkOnly)
    {
        bool deleteGroup = false;
        
        CbString str;
        str.Format("Are you sure you want to delete meta group '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            if (GetChildCount())
            {
                CbViewLock lock(GetDataModelDoc());

                ClassGroupIterator iClassGroup(this);
                while (++iClassGroup)
                {
                    ClassGroup* pClassGroup = iClassGroup;
                    
                    // Remove from tree
                    pClassGroup->Remove();
                    
                    // Relocate
                    RemoveClassGroup(pClassGroup);
                    GetDataModel()->AddClassGroupLast(pClassGroup);
                    
                    // Add to tree
                    pClassGroup->Add();
                }

                // Put in correct order in tree
                int i = 0;
                DataModel::ClassGroupIterator iClassGroupOrder(GetDataModel());
                while (++iClassGroupOrder)
                {
                    iClassGroupOrder->SaveState();
                    iClassGroupOrder->SetOrder(i++);
                }
                
                // Process leftover, this must be classdiagrams, which must be attached to the tree manually
                Gti::ChildIterator iChild(this);
                while (++iChild)
                {
                    Gti* pGti = iChild;
                    
                    pGti->Remove();
                    GetDataModel()->AddChildLast(pGti);
                    pGti->Add();
                }
            
            }
            
            Delete();
        }
    }
    
    return 1;
}//@CODE_29606


int MetaGroup::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_35077
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
        
    return 0;
}//@CODE_35077


/*@NOTE_29583
Sort items alphabetically on their name
*/
int MetaGroup::SortOnName(bool checkOnly)
{//@CODE_29583
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortClassGroup(ClassGroup::CompareName);

        int i = 0;
        ClassGroupIterator iClassGroup(this);
        while (++iClassGroup)
        {
            iClassGroup->SaveState();
            iClassGroup->SetOrder(i++);
        }
    }

    return 1;
}//@CODE_29583


int MetaGroup::SortOnPhase(bool checkOnly)
{//@CODE_29585
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortClassGroup(ClassGroup::ComparePhase);

        int i = 0;
        ClassGroupIterator iClassGroup(this);
        while (++iClassGroup)
        {
            iClassGroup->SaveState();
            iClassGroup->SetOrder(i++);
        }
    }

    return GetDataModel()->GetPhaseSupport();
}//@CODE_29585


void MetaGroup::Update()
{//@CODE_29587
    if (GetAdded())
    {
        SetItemText(GetName());

        Gti::Update();
    }
}//@CODE_29587


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_28971
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MetaGroup::CleanupReferences()
{
    Group::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
}


/*@NOTE_28961
Method which must be called first in a constructor.
*/
void MetaGroup::ConstructorInclude(DataModel* pDataModel)
{
    INIT_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    INIT_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
}


/*@NOTE_28963
Method which must be called first in a destructor.
*/
void MetaGroup::DestructorInclude()
{
    EXIT_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    EXIT_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
}


/*@NOTE_28972
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MetaGroup::RemoveReferences()
{
    REMOVE_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    Group::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
}


/*@NOTE_28973
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MetaGroup::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MetaGroup* pMetaGroup = (MetaGroup*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    Group::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_28975
Save the state of the current object relations to pDataModelDocObject.
*/
void MetaGroup::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Group::SaveReferences(pDataModelDocObject);
    MetaGroup* pMetaGroup = (MetaGroup*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
}


/*@NOTE_28966
Serialize the members only to a CbObject object.
*/
void MetaGroup::Serialize(CbArchive& archive)
{
    Group::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_28965
Method which must be called first in a serialize constructor.
*/
void MetaGroup::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    INIT_MULTI_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)
}


/*@NOTE_28968
Serialize the relations to a CbObject object.
*/
void MetaGroup::SerializeRelations(CbArchive& archive,
                                   DataModelDocObject* pointerArray[])
{
    Group::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(MetaGroup)


// Methods for the relation(s) of the class
METHODS_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
METHODS_ITERATOR_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
METHODS_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
