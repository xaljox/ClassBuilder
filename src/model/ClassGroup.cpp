/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassGroup.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassGroup'
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
#include <iostream>
using namespace std;

#include "ClassBuilderDoc.h"
#include "qt/QtContextDialog.h"
//@END_USER2


// Static members


ClassGroup::ClassGroup(DataModel* pDataModel) //@INIT_1022
    : Group(pDataModel->GetDataModelDoc())
{//@CODE_1022
    ConstructorInclude();

    // Put in your own code
    pDataModel->AddClassGroupLast(this);
}//@CODE_1022


/*@NOTE_29572
Constructor method.
*/
ClassGroup::ClassGroup(MetaGroup* pMetaGroup) //@INIT_29572
    : Group(pMetaGroup->GetDataModel()->GetDataModelDoc())
{//@CODE_29572
    ConstructorInclude();

    // Put in your own code
    pMetaGroup->AddClassGroupLast(this);
}//@CODE_29572


/*@NOTE_529
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassGroup::ClassGroup() //@INIT_529
    : Group()
{//@CODE_529
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_529


/*@NOTE_527
Destructor method
*/
ClassGroup::~ClassGroup()
{//@CODE_527
    DestructorInclude();

    // Put in your own code
}//@CODE_527


void ClassGroup::Add()
{//@CODE_1024
    if (!GetAdded())
    {
        SaveState(1);
        if (GetDataModel())
        {
            GetDataModel()->AddChildLast(this);
        }
        if (GetMetaGroup())
        {
            GetMetaGroup()->AddChildLast(this);
        }
    
        SetItemText(GetName());
        SetIcon(ICON_FILE);

        Gti::Add();

        Gti::ChildIterator iClassDiagram(this, &Gti::IsClassDiagram);
        while (++iClassDiagram)
            iClassDiagram->Add();

        Gti::ChildIterator iSequenceDiagram(this, &Gti::IsSequenceDiagram);
        while (++iSequenceDiagram)
            iSequenceDiagram->Add();
        
        ClassIterator iClass(this);
        while (++iClass)
            iClass->Add();
    }
}//@CODE_1024


int ClassGroup::CompareName(ClassGroup* pA, ClassGroup* pB)
{//@CODE_1832
    int result = pA->GetName().CompareNoCase(pB->GetName());

    if (result > 0)
    {
        pB->SaveState();
        pB->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_1832


int ClassGroup::ComparePhase(ClassGroup* pA, ClassGroup* pB)
{//@CODE_23478
    int result = pA->GetPhase() - pB->GetPhase();

    if (result > 0)
    {
        pB->SaveState();
        pB->GetDataModelDoc()->MarkLastUndo(2);
    }

    return result;
}//@CODE_23478


Context* ClassGroup::CreateContext(ContextDeclaration* pContextDeclaration)
{//@CODE_27241
    return new ClassGroupContext(this, pContextDeclaration);
}//@CODE_27241


bool ClassGroup::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1478
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        DataModel* pDataModel = GetDataModel();
        if (pDataModel)
        {
            if (pDataModel->GetPrevClassGroup(this))
                pGtiDropDefault = pDataModel->GetPrevClassGroup(this);
            else
                pGtiDropDefault = pDataModel;
            
            SaveState(1);
            pDataModel->RemoveClassGroup(this);
        }
        
        MetaGroup* pMetaGroup = GetMetaGroup();
        if (pMetaGroup)
        {
            if (pMetaGroup->GetPrevClassGroup(this))
                pGtiDropDefault = pMetaGroup->GetPrevClassGroup(this);
            else
                pGtiDropDefault = pMetaGroup;
            
            SaveState(1);
            pMetaGroup->RemoveClassGroup(this);
        }

        Remove();
        value = true;
    }

    return value;
}//@CODE_1478


void ClassGroup::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1481
    if (ctrlKeyDown)
    {
    }
    else
    {
        CbViewLock lock(GetDataModelDoc());

        DataModel* pDropDataModel = dynamic_cast<DataModel*>(pGtiDrop);
        MetaGroup* pDropMetaGroup = dynamic_cast<MetaGroup*>(pGtiDrop);
        ClassGroup* pDropClassGroup = dynamic_cast<ClassGroup*>(pGtiDrop);
        if (pDropDataModel)
            pDropDataModel->AddClassGroupFirst(this);
        else if (pDropMetaGroup)
            pDropMetaGroup->AddClassGroupFirst(this);
        else if (pDropClassGroup)
        {
            if (pDropClassGroup->GetDataModel())
                pDropClassGroup->GetDataModel()->AddClassGroupAfter(this, pDropClassGroup);
            else if (pDropClassGroup->GetMetaGroup())
                pDropClassGroup->GetMetaGroup()->AddClassGroupAfter(this, pDropClassGroup);
        }

        if (GetDataModel())
        {
            int i = 0;
            DataModel::ClassGroupIterator iClassGroup(GetDataModel());
            while (++iClassGroup)
            {
                iClassGroup->SaveState(1);
                iClassGroup->SetOrder(i++);
            }
        }
        else if (GetMetaGroup())
        {
            int i = 0;
            MetaGroup::ClassGroupIterator iClassGroup(GetMetaGroup());
            while (++iClassGroup)
            {
                iClassGroup->SaveState(1);
                iClassGroup->SetOrder(i++);
            }
        }
        
        Add();

    }
}//@CODE_1481


bool ClassGroup::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1484
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
    }
    else
    {
        if (pGtiDrop->IsDataModel() || pGtiDrop->IsMetaGroup() || 
            pGtiDrop->IsClassGroup())
        {
            value = true;
        }
    }

    return value;
}//@CODE_1484


Context* ClassGroup::GetFirstContext()
{//@CODE_27243
    return GetFirstClassGroupContext();
}//@CODE_27243


Context* ClassGroup::GetNextContext(Context* pContextPos)
{//@CODE_27244
    return GetNextClassGroupContext((ClassGroupContext*)pContextPos);
}//@CODE_27244


int ClassGroup::OnAddClass(bool checkOnly)
{//@CODE_1027
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Class* pClass = new Class(GetDataModelDoc()->GetDataModel());
        pClass->SetOrder(GetClassCount()); // Make it the last in the tree view
        AddClassLast(pClass);

        if (pClass->GetSerialize())
        {
            Inherit* pInherit = new Inherit(pClass, pClass->GetDataModel()->GetDocumentObject());
            pInherit->SetVirtual(1);
        }

        if (pClass->OnEditAttributes())
        {
            // Post-dialog: coalesce the Add cascade (class + its seeded methods).
            CbViewLock lock(GetDataModelDoc());
            pClass->Add();
        }
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_1027


int ClassGroup::OnAddClassDiagram(bool checkOnly)
{//@CODE_3900
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
}//@CODE_3900


int ClassGroup::OnAddSequenceDiagram(bool checkOnly)
{//@CODE_30485
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
}//@CODE_30485


int ClassGroup::OnDelete(bool checkOnly)
{//@CODE_1026
    if (!checkOnly)
    {
        bool deleteGroup = false;
        
        CbString str;
        str.Format("Are you sure you want to delete group '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            if (GetChildCount())
            {
                str.Format("Group '%s' isn't empty, do you want to delete all in it", GetName().c_str());
                if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO|CBMB_DEFBUTTON2) == CBMB_IDYES)
                {
                    CbViewLock lock(GetDataModelDoc());
                    
                    Gti::ChildIterator iChild(this);
                    while (++iChild)
                        iChild->Delete();
                    
                }
                else
                {
                    CbViewLock lock(GetDataModelDoc());
                    
                    DataModel* pDataModel = GetDataModelDoc()->GetDataModel();
                    
                    ClassIterator iClass(this);
                    while (++iClass)
                    {
                        Class* pClass = iClass;
                        
                        // Remove from tree
                        pClass->Remove();
                        
                        // Relocate
                        RemoveClass(pClass);
                        pDataModel->MoveClassLast(pClass);
                        
                        // Add to tree
                        pClass->Add();
                    }
                    
                    // Put in correct order in tree
                    int i = 0;
                    DataModel::ClassIterator iClassOrder(pDataModel);
                    while (++iClassOrder)
                    {
                        iClassOrder->SaveState();
                        iClassOrder->SetOrder(i++);
                    }
                    
                    // Process leftover, this must be classdiagrams, which must be attached to the tree manually
                    Gti::ChildIterator iChild(this);
                    while (++iChild)
                    {
                        Gti* pGti = iChild;
                        
                        pGti->Remove();
                        GetParent()->AddChildLast(pGti);
                        pGti->Add();
                    }
                    
                }
            }
            
            Delete();
        }
    }
    
    return 1;
}//@CODE_1026


int ClassGroup::OnEditContext(bool checkOnly)
{//@CODE_27246
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
}//@CODE_27246


int ClassGroup::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_35080
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
}//@CODE_35080


/*@NOTE_1528
Sort items alphabetically on their name
*/
int ClassGroup::SortOnName(bool checkOnly)
{//@CODE_1528
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortClass(Class::CompareName);

        int i = 0;
        ClassIterator iClass(this);
        while (++iClass)
        {
            iClass->SaveState();
            iClass->SetOrder(i++);
        }
    }

    return 1;
}//@CODE_1528


int ClassGroup::SortOnPhase(bool checkOnly)
{//@CODE_23476
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortClass(Class::ComparePhase);

        int i = 0;
        ClassIterator iClass(this);
        while (++iClass)
        {
            iClass->SaveState();
            iClass->SetOrder(i++);
        }
    }

    return GetDataModelDoc()->GetDataModel()->GetPhaseSupport();
}//@CODE_23476


void ClassGroup::Update()
{//@CODE_1025
    if (GetAdded())
    {
        SetItemText(GetName());

        Gti::Update();
    }
}//@CODE_1025


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5288
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassGroup::CleanupReferences()
{
    Group::CleanupReferences();
    CLEANUP_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    CLEANUP_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
}


/*@NOTE_526
Method which must be called first in a constructor
*/
void ClassGroup::ConstructorInclude()
{
    INIT_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
    INIT_MULTI_OWNED_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    INIT_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    INIT_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
}


/*@NOTE_528
Method which must be called first in a destructor
*/
void ClassGroup::DestructorInclude()
{
    EXIT_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
    EXIT_MULTI_OWNED_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    EXIT_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    EXIT_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
}


/*@NOTE_5289
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassGroup::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    REMOVE_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
    Group::RemoveReferences();
    REMOVE_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    REMOVE_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
}


/*@NOTE_5290
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassGroup::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassGroup* pClassGroup = (ClassGroup*)pDataModelDocObject;
    RESTORE_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    RESTORE_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    Group::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5292
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassGroup::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Group::SaveReferences(pDataModelDocObject);
    ClassGroup* pClassGroup = (ClassGroup*)pDataModelDocObject;
    SAVE_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    SAVE_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
}


/*@NOTE_531
Serialize the members only to a CbObject object
*/
void ClassGroup::Serialize(CbArchive& archive)
{
    Group::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_530
Method which must be called first in a serialize constructor
*/
void ClassGroup::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
    INIT_MULTI_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    INIT_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    INIT_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
}


/*@NOTE_533
Serialize the relations to a CbObject object
*/
void ClassGroup::SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[])
{
    Group::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
        WRITE_MULTI_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
            READ_MULTI_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ClassGroup)


// Methods for the relation(s) of the class
METHODS_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
METHODS_ITERATOR_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
METHODS_MULTI_OWNED_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
METHODS_ITERATOR_MULTI_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
METHODS_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
METHODS_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
