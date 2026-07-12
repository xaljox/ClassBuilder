/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Gti.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Gti'
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
#include "ClassBuilderDoc.h"
// (ClassBuilderView.h include removed -- the MFC tree view is retired)
//@END_USER2


// Static members
Gti* Gti::_pGtiCopy = 0;


Gti::Gti(DataModelDoc* pDataModelDoc) //@INIT_680
    : DataModelDocObject(pDataModelDoc)
    , _added(false)
    , _itemText("")
    , _icon(ICON_FILE)
    , _initialVersion(pDataModelDoc->GetVersion() + 1)
    , _version(pDataModelDoc->GetVersion() + 1)
    , _order(0)
    , _addInString("")
    , _phase(None_Phase)
    , _state(0)
{//@CODE_680
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_680


/*@NOTE_1556
Constructor needed for putting a new object in the old one's context
*/
Gti::Gti(Gti* pOld) //@INIT_1556
    : DataModelDocObject(pOld)
{//@CODE_1556
    ReplaceConstructorInclude(pOld);

    _added = pOld->_added;
    _itemText = pOld->_itemText;
    _icon = pOld->_icon;
    _initialVersion = pOld->_initialVersion;
    _version = pOld->_version;
    _order = pOld->_order;
    _addInString = pOld->_addInString;
    _phase = pOld->_phase;

    // Put in your own code
}//@CODE_1556


/*@NOTE_61
Constructor needed for serialization, not meant to use for other purposes!
*/
Gti::Gti() //@INIT_61
    : DataModelDocObject()
    , _order(0)
    , _addInString("")
    , _phase(None_Phase)
    , _state(0)
{//@CODE_61
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_61


/*@NOTE_59
Destructor method
*/
Gti::~Gti()
{//@CODE_59
    DestructorInclude();

    // Put in your own code
    if (_pGtiCopy == this)
    {
        _pGtiCopy = 0;
    }

    // NOTE: closing a (sub) tree view open on this node is NOT done here -- by
    // the time ~Gti runs the object has long left the data structure (it sat on
    // the undo stack, and RemoveReferences already cut its doc backpointer, so
    // GetDataModelDoc() would be null here). The view is closed in
    // OnUndoRedoRemoving() instead, which runs while the node is still live.
}//@CODE_59


void Gti::Add()
{//@CODE_682
    if (!_added)
    {
        _added = true;
        if (!IsDataModel())
            SetVersion(GetDataModelDoc()->GetVersion() + 1);
        GetDataModelDoc()->NotifyStructureChanged();
        GetDataModelDoc()->SetModifiedFlag();

        if (GetParent() && GetPhase())
        {
            if (GetPhase() < GetParent()->GetPhase())
            {
                GetParent()->SetPhaseUpwards(GetPhase());
            }
        }
    }
}//@CODE_682


void Gti::AddRecursive()
{//@CODE_22930
    GetDataModelDoc()->NotifyStructureChanged();

    Gti::ChildIterator iChild(this);
    while (++iChild)
    {
        iChild->AddRecursive();
    }
}//@CODE_22930


int Gti::CompareTreeOrder(Gti* pGti1, Gti* pGti2)
{//@CODE_40609
    DataModel* pDataModel1             = dynamic_cast<DataModel*>(pGti1);
    ClassDiagram* pClassDiagram1       = dynamic_cast<ClassDiagram*>(pGti1);
    SequenceDiagram* pSequenceDiagram1 = dynamic_cast<SequenceDiagram*>(pGti1);
    ClassGroup* pClassGroup1           = dynamic_cast<ClassGroup*>(pGti1);
    MetaGroup* pMetaGroup1             = dynamic_cast<MetaGroup*>(pGti1);
    Group* pGroup1                     = dynamic_cast<Group*>(pGti1);
    Class* pClass1                     = dynamic_cast<Class*>(pGti1);
    Inherit* pInherit1                 = dynamic_cast<Inherit*>(pGti1);
    FromRelation* pFromRelation1       = dynamic_cast<FromRelation*>(pGti1);
    ToRelation* pToRelation1           = dynamic_cast<ToRelation*>(pGti1);
    Member* pMember1                   = dynamic_cast<Member*>(pGti1);
    Method* pMethod1                   = dynamic_cast<Method*>(pGti1);
    Type* pType1                       = dynamic_cast<Type*>(pGti1);
    Argument* pArgument1               = dynamic_cast<Argument*>(pGti1);
    Actors* pActors1                   = dynamic_cast<Actors*>(pGti1);
    Actor* pActor1                     = dynamic_cast<Actor*>(pGti1);

    DataModel* pDataModel2             = dynamic_cast<DataModel*>(pGti2);
    ClassDiagram* pClassDiagram2       = dynamic_cast<ClassDiagram*>(pGti2);
    SequenceDiagram* pSequenceDiagram2 = dynamic_cast<SequenceDiagram*>(pGti2);
    ClassGroup* pClassGroup2           = dynamic_cast<ClassGroup*>(pGti2);
    MetaGroup* pMetaGroup2             = dynamic_cast<MetaGroup*>(pGti2);
    Group* pGroup2                     = dynamic_cast<Group*>(pGti2);
    Class* pClass2                     = dynamic_cast<Class*>(pGti2);
    Inherit* pInherit2                 = dynamic_cast<Inherit*>(pGti2);
    FromRelation* pFromRelation2       = dynamic_cast<FromRelation*>(pGti2);
    ToRelation* pToRelation2           = dynamic_cast<ToRelation*>(pGti2);
    Member* pMember2                   = dynamic_cast<Member*>(pGti2);
    Method* pMethod2                   = dynamic_cast<Method*>(pGti2);
    Type* pType2                       = dynamic_cast<Type*>(pGti2);
    Argument* pArgument2               = dynamic_cast<Argument*>(pGti2);
    Actors* pActors2                   = dynamic_cast<Actors*>(pGti2);
    Actor* pActor2                     = dynamic_cast<Actor*>(pGti2);

    int order1 = 11; // Default extern, other types
    if (pDataModel1)
        order1 = 0;
    else if (pClassDiagram1 || pSequenceDiagram1)
        order1 = 1;
    else if (pClassGroup1 || pMetaGroup1)
        order1 = 2;
    else if (pInherit1 || pActors1)
        order1 = 3;
    else if (pToRelation1)
        order1 = 4;
    else if (pFromRelation1)
        order1 = 5;
    else if (pGroup1)
        order1 = 6;
    else if (pMethod1)
        order1 = 7;
    else if (pMember1)
        order1 = 8;
    else if (pClass1)
        order1 = 9;
    else if (pArgument1)
        order1 = 10;

    int order2 = 11; // Default extern, other types
    if (pDataModel2)
        order2 = 0;
    else if (pClassDiagram2 || pSequenceDiagram2)
        order2 = 1;
    else if (pClassGroup2 || pMetaGroup2)
        order2 = 2;
    else if (pInherit2 || pActors2)
        order2 = 3;
    else if (pToRelation2)
        order2 = 4;
    else if (pFromRelation2)
        order2 = 5;
    else if (pGroup2)
        order2 = 6;
    else if (pMethod2)
        order2 = 7;
    else if (pMember2)
        order2 = 8;
    else if (pClass2)
        order2 = 9;
    else if (pArgument2)
        order2 = 10;

    if (pGroup1 && pGroup2)
        return pGroup1->GetOrder()-pGroup2->GetOrder();
    else if (pClass1 && pClass2)
        return pClass1->GetOrder()-pClass2->GetOrder();
    else if ((pClassDiagram1 || pSequenceDiagram1) && (pClassDiagram2 || pSequenceDiagram2))
        return pGti1->GetOrder()-pGti2->GetOrder();
    else if (pType1 && pType2)
        return pType1->GetName().CompareNoCase(pType2->GetName());
    else if (pActor1 && pActor2)
        return pActor1->GetName().CompareNoCase(pActor2->GetName());
    else if (pInherit1 && pInherit2)
        return pInherit1->GetBaseName().CompareNoCase(pInherit2->GetBaseName());
    else if (pFromRelation1 && pFromRelation2)
        return pFromRelation1->GetItemText().CompareNoCase(pFromRelation2->GetItemText());
    else if (pToRelation1 && pToRelation2)
        return pToRelation1->GetItemText().CompareNoCase(pToRelation2->GetItemText());
    else if (pMember1 && pMember2)
    {
        return Member::CompareTree(pMember1, pMember2);
    }
    else if (pMethod1 && pMethod2)
    {
        return Method::CompareTree(pMethod1, pMethod2);
    }
    else if (pArgument1 && pArgument2)
    {
        if (pArgument1 == pArgument2)
            return 0;

        Argument* pArgument = pArgument1->GetMethod()->GetNextArgument(pArgument1);
        while (pArgument)
        {
            if (pArgument == pArgument2)
                return -1;

            pArgument = pArgument1->GetMethod()->GetNextArgument(pArgument);
        }

        return 1;
    }
    else
    {
        return order1-order2;
    }
}//@CODE_40609


Context* Gti::CreateContext(ContextDeclaration* pContextDeclaration)
{//@CODE_25483
    return 0;
}//@CODE_25483


bool Gti::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1311
    pGtiDropDefault = NULL;

    return false;
}//@CODE_1311


void Gti::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1317
}//@CODE_1317


bool Gti::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1314
    return false;
}//@CODE_1314


Gti* Gti::FindStringFiltered(CbString str, bool matchCase, bool wholeName,
                             bool searchTypes, bool searchMethods,
                             bool searchArguments, bool searchMembers)
{//@CODE_36819
    if (str.IsEmpty())
    {
        return 0;
    }

    if (!matchCase)
    {
        str.MakeUpper();
    }

    Gti* pGti = GetNext();
    while (pGti)
    {
        bool kindOk =
            (searchTypes      && pGti->IsType())     ||
            (searchMethods    && pGti->IsMethod())   ||
            (searchArguments  && pGti->IsArgument()) ||
            (searchMembers    && pGti->IsMember());

        if (kindOk)
        {
            CbString text = pGti->GetItemText();

            int index = text.Find("(");
            if (index != -1)
            {
                text = text.Left(index);
            }

            if (wholeName)
            {
                // Strip any leading "return-type " / modifier prefix (Method
                // item text is e.g. "void Serialize") so the bare name remains.
                int sp = text.ReverseFind(' ');
                if (sp != -1)
                {
                    text = text.Mid(sp + 1);
                }
            }

            if (!matchCase)
            {
                text.MakeUpper();
            }

            bool hit = wholeName ? (text == str) : (text.Find(str) != -1);
            if (hit)
            {
                return pGti;
            }
        }

        pGti = pGti->GetNext();
    }

    return 0;
}//@CODE_36819


Context* Gti::GetFirstContext()
{//@CODE_26209
    return 0;
}//@CODE_26209


Gti* Gti::GetNext(Gti* pGti)
{//@CODE_35365
    if (!pGti && GetFirstChild())
    {
        return GetFirstChild();
    }
    
    if (pGti && GetNextChild(pGti))
    {
        return GetNextChild(pGti);
    }
    
    if (GetParent())
    {
        return GetParent()->GetNext(this);
    }
    
    return 0;
}//@CODE_35365


Context* Gti::GetNextContext(Context* pContextPos)
{//@CODE_26226
    return 0;
}//@CODE_26226


/*@NOTE_23449
Return index to the state icon to be used.
*/
int Gti::GetStateIcon()
{//@CODE_23449
    int stateIcon = 0;

    if (GetDataModelDoc()->GetDataModel()->GetPhaseSupport())
    {
        stateIcon = GetPhase();
    }
    
    return stateIcon;
}//@CODE_23449


/*@NOTE_23456
Returns true if it is allowed to edit the phase of this node.
*/
int Gti::IsAllowedToEditPhase(Phase phase) const
{//@CODE_23456
    if (!GetDataModelDoc()->GetDataModel()->GetPhaseSupport() ||
        _phase == None_Phase || IsMacroMethods() || IsToRelation())
    {
        return 0;
    }

    const Method* pMethod = dynamic_cast<const Method*>(this);
    if (pMethod && pMethod->IsFixed())
    {
        return 0;
    }

    if (IsFromRelation() || IsMemberAndMethodGroup())
    {
        int minPhase = Complete_Phase;
        ChildIterator iChild(this);
        while (++iChild)
        {
            if (iChild->GetPhase() < minPhase)
            {
                minPhase = iChild->GetPhase();
            }
        }

        if (minPhase == Complete_Phase)
        {
            return 0;
        }
        else
        {
            return (minPhase < phase);
        }
    }

    return 1;
}//@CODE_23456


int Gti::OnAddActor(bool checkOnly)
{//@CODE_33648
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Actor* pActor = new Actor(GetDataModelDoc());
        if (pActor->OnEditAttributes())
            pActor->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_33648


int Gti::OnAddArgument(bool checkOnly)
{//@CODE_695
    if (GetParent())
        return GetParent()->OnAddArgument(checkOnly);
    
    return 0;
}//@CODE_695


int Gti::OnAddClass(bool checkOnly)
{//@CODE_687
    if (GetParent())
        return GetParent()->OnAddClass(checkOnly);

    return 0;
}//@CODE_687


int Gti::OnAddClassDiagram(bool checkOnly)
{//@CODE_3904
    if (GetParent())
        return GetParent()->OnAddClassDiagram(checkOnly);
    
    return 0;
}//@CODE_3904


int Gti::OnAddConstructor(bool checkOnly)
{//@CODE_694
    if (GetParent())
        return GetParent()->OnAddConstructor(checkOnly);
    
    return 0;
}//@CODE_694


int Gti::OnAddGroup(bool checkOnly)
{//@CODE_689
    if (GetParent())
        return GetParent()->OnAddGroup(checkOnly);
    
    return 0;
}//@CODE_689


int Gti::OnAddInherit(bool checkOnly)
{//@CODE_690
    if (GetParent())
        return GetParent()->OnAddInherit(checkOnly);
    
    return 0;
}//@CODE_690


int Gti::OnAddIsClassMethods(bool checkOnly)
{//@CODE_697
    if (GetParent())
        return GetParent()->OnAddIsClassMethods(checkOnly);
    
    return 0;
}//@CODE_697


int Gti::OnAddMember(bool checkOnly)
{//@CODE_692
    if (GetParent())
        return GetParent()->OnAddMember(checkOnly);
    
    return 0;
}//@CODE_692


int Gti::OnAddMetaGroup(bool checkOnly)
{//@CODE_29600
    if (GetParent())
        return GetParent()->OnAddMetaGroup(checkOnly);
    
    return 0;
}//@CODE_29600


int Gti::OnAddMethod(bool checkOnly)
{//@CODE_693
    if (GetParent())
        return GetParent()->OnAddMethod(checkOnly);
    
    return 0;
}//@CODE_693


int Gti::OnAddRelation(bool checkOnly)
{//@CODE_691
    if (GetParent())
        return GetParent()->OnAddRelation(checkOnly);
    
    return 0;
}//@CODE_691


int Gti::OnAddSequenceDiagram(bool checkOnly)
{//@CODE_30477
    if (GetParent())
        return GetParent()->OnAddSequenceDiagram(checkOnly);
    
    return 0;
}//@CODE_30477


int Gti::OnAddType(bool checkOnly)
{//@CODE_688
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        OtherType* pOtherType = new OtherType(GetDataModelDoc());
        if (pOtherType->OnEditAttributes())
            pOtherType->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_688


int Gti::OnAddVirtuals(bool checkOnly)
{//@CODE_696
    if (GetParent())
        return GetParent()->OnAddVirtuals(checkOnly);
    
    return 0;
}//@CODE_696


int Gti::OnCopy(bool checkOnly)
{//@CODE_40789
    bool enable = (IsExternClass() || IsDataModel() ||
        IsExternClasses() || IsMember() ||
        IsClassDiagram() || IsSequenceDiagram());

    if (enable && !checkOnly)
    {
        SetGtiCopy(this);
    }

    return enable;
}//@CODE_40789


int Gti::OnDelete(bool checkOnly)
{//@CODE_686
    return 0;
}//@CODE_686


int Gti::OnEditAttributes(bool checkOnly)
{//@CODE_684
    return 0; 
}//@CODE_684


int Gti::OnEditContext(bool checkOnly)
{//@CODE_25741
    /*
    if (GetParent())
        return GetParent()->OnEditContext(checkOnly);
    */
    
    return 0;
}//@CODE_25741


int Gti::OnEditExceptionSpecification(bool checkOnly)
{//@CODE_22708
    if (GetParent())
        return GetParent()->OnEditExceptionSpecification(checkOnly);
    
    return 0;
}//@CODE_22708


int Gti::OnOpen(bool checkOnly)
{//@CODE_685
    if (!checkOnly)
    {
        OnEditAttributes();
    }

    return 0;
}//@CODE_685


int Gti::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_1590
    if (GetParent())
        return GetParent()->OnPaste(pGti, checkOnly);
    
    if (!checkOnly)
    {
        CbMessageBox("Incorrect place selected for paste paste", CBMB_ICONEXCLAMATION);
    }
    
    return 0;
}//@CODE_1590


/*@NOTE_22897
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.
Returns optional an object to update.
*/
void Gti::OnUndoRedoAdded()
{//@CODE_22897
    if (GetAdded())
    {
        GetDataModelDoc()->NotifyStructureChanged();
    }
}//@CODE_22897


/*@NOTE_22906
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Gti::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22906
    Gti* pGti = (Gti*)pOldState;
    
    if (GetAdded())
    {
        if (pGti && (!pGti->GetAdded() || GetParent() != pGti->GetParent()))
        {
            AddRecursive();
        }
        else
        {
			GetDataModelDoc()->NotifyStructureChanged();
        }
    }
}//@CODE_22906


/*@NOTE_22908
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called before the object changes state. This method calls
OnUndoRedoRemoving(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour.
*/
void Gti::OnUndoRedoChanging(DataModelDocObject* pNewState)
{//@CODE_22908
    Gti* pGti = (Gti*)pNewState;

    if (GetAdded() && (!pGti->GetAdded() || GetParent() != pGti->GetParent()))
    {
        GetDataModelDoc()->NotifyStructureChanged();
    }
}//@CODE_22908


/*@NOTE_22890
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called before the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void Gti::OnUndoRedoRemoving()
{//@CODE_22890
    // Close any (sub) tree view rooted on this node before it leaves the data
    // structure. This hook fires per node on every removal -- the directly
    // deleted node (UndoDelete/RedoNew) AND each cascade-removed descendant
    // (UndoSubDelete) -- so a sub-window open on ANY node type (class, group,
    // method, inheritance, relation half, ...) is caught. It runs while
    // GetDataModelDoc() is still valid (RemoveReferences, which cuts the doc
    // backpointer, runs AFTER this -- which is why the destructor is too late).
    // No CbViewLock needed: deleting a TreeViewModel only tears down its Qt
    // window, it does not refresh the model; the surrounding op's lock / the
    // removal's own NotifyStructureChanged coalesces the repaint.
    TreeViewModel* pTreeViewModel;
    while ((pTreeViewModel = GetDataModelDoc()->FindTreeViewModel(this)) != 0)
    {
        delete pTreeViewModel;
    }

    if (GetAdded())
    {
        if (GetParent())
        {
            if (GetPhase() == GetParent()->GetPhase())
            {
                Phase minPhase = Complete_Phase;
                ChildIterator iGti(GetParent());
                while (++iGti)
                {
                    if (iGti.Get() != this && iGti->GetPhase() &&
                        minPhase > iGti->GetPhase())
                    {
                        minPhase = iGti->GetPhase();
                    }
                }

                if (minPhase != GetParent()->GetPhase())
                {
                    GetParent()->SetPhaseUpwards(minPhase);
                }
            }
        }

        // No view update HERE: this hook runs BEFORE the object leaves the
        // data structure, and the Qt trees rebuild from the model -- an
        // update now still shows the object. The UndoDelete/RedoNew ctors
        // fire the update after RemoveReferences instead.
    }
}//@CODE_22890


void Gti::Remove()
{//@CODE_698
    if (_added)
    {
        ChildIterator iGti(this);
        while (++iGti)
            iGti->RemoveRecursive();

        SaveState(1);
        _added = false;

        if (GetParent())
        {
            if (GetPhase() == GetParent()->GetPhase())
            {
                Phase minPhase = Complete_Phase;
                ChildIterator iGti(GetParent());
                while (++iGti)
                {
                    if (iGti.Get() != this && iGti->GetPhase() &&
                        minPhase > iGti->GetPhase())
                    {
                        minPhase = iGti->GetPhase();
                    }
                }

                if (minPhase != GetParent()->GetPhase())
                {
                    GetParent()->SetPhaseUpwards(minPhase);
                }
            }
            GetParent()->RemoveChild(this);
        }

        // Notify AFTER the unlink: the Qt trees rebuild from the model on this
        // call, so firing it first (the old MFC MOD_REMOVE hint order) rebuilt
        // WITH the item still present -- a leaf delete then looked like a
        // missing tree update. No consumer needs the pre-removal hint anymore.
        GetDataModelDoc()->NotifyStructureChanged();
        GetDataModelDoc()->SetModifiedFlag();
    }
}//@CODE_698


void Gti::RemoveRecursive()
{//@CODE_679
    if (_added)
    {
        ChildIterator gti(this);
        while (++gti)
            gti->RemoveRecursive();

        SaveState(1);
        _added = false;

        if (GetParent() && !IsClassDiagram() && !IsSequenceDiagram())
        {
            GetParent()->RemoveChild(this);
        }
    }
}//@CODE_679


void Gti::SetPhaseDownAndUpwards(Phase phase)
{//@CODE_23464
    if (IsAllowedToEditPhase(phase) && phase != None_Phase && _phase != phase)
    {
        SaveState();
        
        if (_phase < phase)
        {
            _phase = phase;
        
            ChildIterator iChild(this);
            while (++iChild)
            {
                if (iChild->GetPhase() && iChild->GetPhase() < phase)
                {
                    iChild->SetPhaseDownAndUpwards(phase);
                }
            }
            
            if (GetParent() && GetParent()->GetPhase() < phase)
            {
                Phase minPhase = phase;
               
                ChildIterator iChild(GetParent());
                while (++iChild)
                {
                    if (iChild->GetPhase() && iChild->GetPhase() < minPhase)
                    {
                        minPhase = iChild->GetPhase();

                        if (minPhase == GetParent()->GetPhase())
                        {
                            break;
                        }
                    }
                }
                
                if (minPhase != GetParent()->GetPhase())
                {
                    GetParent()->SetPhaseUpwards(minPhase);
                }
            }
        }
        else
        {
            _phase = phase;
            
            if (GetParent() && GetParent()->GetPhase() > phase)
            {
                GetParent()->SetPhaseUpwards(phase);
            }
        }
        
        Gti::Update();
    }
}//@CODE_23464


/*@NOTE_23454
Set _phase to phase and propegate it upwards.
*/
void Gti::SetPhaseUpwards(Phase phase)
{//@CODE_23454
    if (_phase != None_Phase && phase != None_Phase && _phase != phase)
    {
        SaveState();
        
        if (_phase < phase)
        {
            _phase = phase;
        
            if (GetParent() && GetParent()->GetPhase() < phase)
            {
                Phase minPhase = phase;
               
                ChildIterator iChild(GetParent());
                while (++iChild)
                {
                    if (iChild->GetPhase() && iChild->GetPhase() < minPhase)
                    {
                        minPhase = iChild->GetPhase();

                        if (minPhase == GetParent()->GetPhase())
                        {
                            break;
                        }
                    }
                }
                
                if (minPhase != GetParent()->GetPhase())
                {
                    GetParent()->SetPhaseUpwards(minPhase);
                }
            }
        }
        else
        {
            _phase = phase;
            
            if (GetParent() && GetParent()->GetPhase() > phase)
            {
                GetParent()->SetPhaseUpwards(phase);
            }
        }
        
        Gti::Update();
    }
}//@CODE_23454


bool Gti::ShownByFilter(TreeViewModel* pTreeViewModel)
{//@CODE_40776
    if (GetDataModelDoc()->GetDataModel()->GetPhaseSupport())
    {
        Phase phase = GetPhase();
        bool show = phase == None_Phase
                 || (phase == Analysis_Phase       && pTreeViewModel->GetShowAnalysisPhase())
                 || (phase == Design_Phase         && pTreeViewModel->GetShowDesignPhase())
                 || (phase == Implementation_Phase && pTreeViewModel->GetShowImplementationPhase())
                 || (phase == Test_Phase           && pTreeViewModel->GetShowTestPhase())
                 || (phase == Complete_Phase       && pTreeViewModel->GetShowCompletePhase());
        if (!show)
        {
            return false;
        }
    }

    return true;
}//@CODE_40776


/*@NOTE_1525
Sort items alphabetically on their name
*/
int Gti::SortOnName(bool checkOnly)
{//@CODE_1525
    if (!checkOnly)
    {
        CbMessageBox("Incorrect place selected to sort on name", CBMB_ICONEXCLAMATION);
    }

    return 0;
}//@CODE_1525


int Gti::SortOnPhase(bool checkOnly)
{//@CODE_23467
    if (!checkOnly)
    {
        CbMessageBox("Incorrect place selected to sort on phase", CBMB_ICONEXCLAMATION);
    }

    return 0;
}//@CODE_23467


void Gti::Update()
{//@CODE_683
    if (_added)
    {
        if (!IsDataModel())
            SetVersion(GetDataModelDoc()->GetVersion() + 1);
        // No SetModifiedFlag here: Update re-displays an already-recorded
        // change -- the mutation itself dirtied the doc via SaveState (or
        // Add/Delete). Two-place dirty rule, see docs/ViewRefresh_Audit.md.
        GetDataModelDoc()->NotifyStructureChanged();
    }
}//@CODE_683


int Gti::GetIcon()
{//@CODE_1187
    return _icon;
}//@CODE_1187


void Gti::SetIcon(int icon)
{//@CODE_1188
    _icon = icon;
}//@CODE_1188


int Gti::GetInitialVersion()
{//@CODE_1181
    return _initialVersion;
}//@CODE_1181


void Gti::SetInitialVersion(int initialVersion)
{//@CODE_1182
    _initialVersion = initialVersion;
    _version = initialVersion;
}//@CODE_1182


CbString Gti::GetItemText()
{//@CODE_1190
    return DataModel::ConvertToHtmlStringIfNeeded(_itemText);
}//@CODE_1190


void Gti::SetItemText(const CbString& rItemText)
{//@CODE_1191
    _itemText = rItemText;
}//@CODE_1191


/*@NOTE_1498
Returns the value of member '_order'.
*/
int Gti::GetOrder()
{//@CODE_1498
    return _order;
}//@CODE_1498


/*@NOTE_1499
Set the value of member '_order' to 'order'.
*/
void Gti::SetOrder(int order)
{//@CODE_1499
    _order = order;
}//@CODE_1499


/*@NOTE_23447
Set the value of member '_phase' to 'phase'.
*/
void Gti::SetPhase(Phase phase)
{//@CODE_23447
    _phase = phase;
}//@CODE_23447


int Gti::GetVersion()
{//@CODE_1184
    return _version;
}//@CODE_1184


void Gti::SetVersion(int version)
{//@CODE_1185
    _version = version;
    if (GetParent() && 
        !GetParent()->IsDataModel() && 
        GetParent()->GetVersion() < version)
    {
        Method* pMethod = dynamic_cast<Method*>(this);
        MacroMethod* pMacroMethod = dynamic_cast<MacroMethod*>(this);
        MemberMethod* pMemberMethod = dynamic_cast<MemberMethod*>(this);
        FromRelationMethod* pFromRelationMethod = dynamic_cast<FromRelationMethod*>(this);
        if (pMacroMethod || pMemberMethod || pFromRelationMethod)
        {
            if (pMethod->GetBaseClass()->GetVersion() < version)
            {
                pMethod->GetBaseClass()->SetVersion(version);
            }
        }
        else
        {
            GetParent()->SetVersion(version);
        }
    }
}//@CODE_1185


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5450
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Gti::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    CLEANUP_MULTI_PASSIVE(Gti, Parent, Gti, Child)
}


/*@NOTE_58
Method which must be called first in a constructor
*/
void Gti::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    INIT_MULTI_PASSIVE(Gti, Parent, Gti, Child)
}


/*@NOTE_60
Method which must be called first in a destructor
*/
void Gti::DestructorInclude()
{
    EXIT_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    EXIT_MULTI_PASSIVE(Gti, Parent, Gti, Child)
}


/*@NOTE_41448
Method that returns true if it is actually a Actor Object.
*/
bool Gti::IsActor() const
{
    return (dynamic_cast<const Actor*>(this) != nullptr);
}


/*@NOTE_41447
Method that returns true if it is actually a Actors Object.
*/
bool Gti::IsActors() const
{
    return (dynamic_cast<const Actors*>(this) != nullptr);
}


/*@NOTE_41392
Method that returns true if it is actually a Argument Object.
*/
bool Gti::IsArgument() const
{
    return (dynamic_cast<const Argument*>(this) != nullptr);
}


/*@NOTE_41388
Method that returns true if it is actually a BaseClass Object.
*/
bool Gti::IsBaseClass() const
{
    return (dynamic_cast<const BaseClass*>(this) != nullptr);
}


/*@NOTE_41390
Method that returns true if it is actually a Class Object.
*/
bool Gti::IsClass() const
{
    return (dynamic_cast<const Class*>(this) != nullptr);
}


/*@NOTE_41445
Method that returns true if it is actually a ClassDiagram Object.
*/
bool Gti::IsClassDiagram() const
{
    return (dynamic_cast<const ClassDiagram*>(this) != nullptr);
}


/*@NOTE_41433
Method that returns true if it is actually a ClassGroup Object.
*/
bool Gti::IsClassGroup() const
{
    return (dynamic_cast<const ClassGroup*>(this) != nullptr);
}


/*@NOTE_41420
Method that returns true if it is actually a CleanupReferencesMethod Object.
*/
bool Gti::IsCleanupReferencesMethod() const
{
    return (dynamic_cast<const CleanupReferencesMethod*>(this) != nullptr);
}


/*@NOTE_41408
Method that returns true if it is actually a Constructor Object.
*/
bool Gti::IsConstructor() const
{
    return (dynamic_cast<const Constructor*>(this) != nullptr);
}


/*@NOTE_41413
Method that returns true if it is actually a ConstructorIncludeMethod Object.
*/
bool Gti::IsConstructorIncludeMethod() const
{
    return (dynamic_cast<const ConstructorIncludeMethod*>(this) != nullptr);
}


/*@NOTE_41429
Method that returns true if it is actually a DataModel Object.
*/
bool Gti::IsDataModel() const
{
    return (dynamic_cast<const DataModel*>(this) != nullptr);
}


/*@NOTE_41411
Method that returns true if it is actually a Destructor Object.
*/
bool Gti::IsDestructor() const
{
    return (dynamic_cast<const Destructor*>(this) != nullptr);
}


/*@NOTE_41416
Method that returns true if it is actually a DestructorIncludeMethod Object.
*/
bool Gti::IsDestructorIncludeMethod() const
{
    return (dynamic_cast<const DestructorIncludeMethod*>(this) != nullptr);
}


/*@NOTE_41389
Method that returns true if it is actually a ExternClass Object.
*/
bool Gti::IsExternClass() const
{
    return (dynamic_cast<const ExternClass*>(this) != nullptr);
}


/*@NOTE_41430
Method that returns true if it is actually a ExternClasses Object.
*/
bool Gti::IsExternClasses() const
{
    return (dynamic_cast<const ExternClasses*>(this) != nullptr);
}


/*@NOTE_41404
Method that returns true if it is actually a FindAvlTreeMethod Object.
*/
bool Gti::IsFindAvlTreeMethod() const
{
    return (dynamic_cast<const FindAvlTreeMethod*>(this) != nullptr);
}


/*@NOTE_41407
Method that returns true if it is actually a FindEqualOrBiggerAvlTreeMethod Object.
*/
bool Gti::IsFindEqualOrBiggerAvlTreeMethod() const
{
    return (dynamic_cast<const FindEqualOrBiggerAvlTreeMethod*>(this) != nullptr);
}


/*@NOTE_41406
Method that returns true if it is actually a FindEqualOrSmallerAvlTreeMethod Object.
*/
bool Gti::IsFindEqualOrSmallerAvlTreeMethod() const
{
    return (dynamic_cast<const FindEqualOrSmallerAvlTreeMethod*>(this) != nullptr);
}


/*@NOTE_41400
Method that returns true if it is actually a FindMethod Object.
*/
bool Gti::IsFindMethod() const
{
    return (dynamic_cast<const FindMethod*>(this) != nullptr);
}


/*@NOTE_41405
Method that returns true if it is actually a FindReverseAvlTreeMethod Object.
*/
bool Gti::IsFindReverseAvlTreeMethod() const
{
    return (dynamic_cast<const FindReverseAvlTreeMethod*>(this) != nullptr);
}


/*@NOTE_41403
Method that returns true if it is actually a FindReverseValueTreeMethod Object.
*/
bool Gti::IsFindReverseValueTreeMethod() const
{
    return (dynamic_cast<const FindReverseValueTreeMethod*>(this) != nullptr);
}


/*@NOTE_41401
Method that returns true if it is actually a FindUniqueValueTreeMethod Object.
*/
bool Gti::IsFindUniqueValueTreeMethod() const
{
    return (dynamic_cast<const FindUniqueValueTreeMethod*>(this) != nullptr);
}


/*@NOTE_41402
Method that returns true if it is actually a FindValueTreeMethod Object.
*/
bool Gti::IsFindValueTreeMethod() const
{
    return (dynamic_cast<const FindValueTreeMethod*>(this) != nullptr);
}


/*@NOTE_41412
Method that returns true if it is actually a FixedMethod Object.
*/
bool Gti::IsFixedMethod() const
{
    return (dynamic_cast<const FixedMethod*>(this) != nullptr);
}


/*@NOTE_41426
Method that returns true if it is actually a FromRelation Object.
*/
bool Gti::IsFromRelation() const
{
    return (dynamic_cast<const FromRelation*>(this) != nullptr);
}


/*@NOTE_41437
Method that returns true if it is actually a FromRelationMacroMethods Object.
*/
bool Gti::IsFromRelationMacroMethods() const
{
    return (dynamic_cast<const FromRelationMacroMethods*>(this) != nullptr);
}


/*@NOTE_41399
Method that returns true if it is actually a FromRelationMethod Object.
*/
bool Gti::IsFromRelationMethod() const
{
    return (dynamic_cast<const FromRelationMethod*>(this) != nullptr);
}


/*@NOTE_41396
Method that returns true if it is actually a GetMemberMethod Object.
*/
bool Gti::IsGetMemberMethod() const
{
    return (dynamic_cast<const GetMemberMethod*>(this) != nullptr);
}


/*@NOTE_41432
Method that returns true if it is actually a Group Object.
*/
bool Gti::IsGroup() const
{
    return (dynamic_cast<const Group*>(this) != nullptr);
}


/*@NOTE_41428
Method that returns true if it is actually a Inherit Object.
*/
bool Gti::IsInherit() const
{
    return (dynamic_cast<const Inherit*>(this) != nullptr);
}


/*@NOTE_41419
Method that returns true if it is actually a IsClassMethod Object.
*/
bool Gti::IsIsClassMethod() const
{
    return (dynamic_cast<const IsClassMethod*>(this) != nullptr);
}


/*@NOTE_41424
Method that returns true if it is actually a MacroMethod Object.
*/
bool Gti::IsMacroMethod() const
{
    return (dynamic_cast<const MacroMethod*>(this) != nullptr);
}


/*@NOTE_41436
Method that returns true if it is actually a MacroMethods Object.
*/
bool Gti::IsMacroMethods() const
{
    return (dynamic_cast<const MacroMethods*>(this) != nullptr);
}


/*@NOTE_41425
Method that returns true if it is actually a Member Object.
*/
bool Gti::IsMember() const
{
    return (dynamic_cast<const Member*>(this) != nullptr);
}


/*@NOTE_41434
Method that returns true if it is actually a MemberAndMethodGroup Object.
*/
bool Gti::IsMemberAndMethodGroup() const
{
    return (dynamic_cast<const MemberAndMethodGroup*>(this) != nullptr);
}


/*@NOTE_41393
Method that returns true if it is actually a MemberArgument Object.
*/
bool Gti::IsMemberArgument() const
{
    return (dynamic_cast<const MemberArgument*>(this) != nullptr);
}


/*@NOTE_41395
Method that returns true if it is actually a MemberMethod Object.
*/
bool Gti::IsMemberMethod() const
{
    return (dynamic_cast<const MemberMethod*>(this) != nullptr);
}


/*@NOTE_41435
Method that returns true if it is actually a MetaGroup Object.
*/
bool Gti::IsMetaGroup() const
{
    return (dynamic_cast<const MetaGroup*>(this) != nullptr);
}


/*@NOTE_41394
Method that returns true if it is actually a Method Object.
*/
bool Gti::IsMethod() const
{
    return (dynamic_cast<const Method*>(this) != nullptr);
}


/*@NOTE_41438
Method that returns true if it is actually a MultiMacroMethods Object.
*/
bool Gti::IsMultiMacroMethods() const
{
    return (dynamic_cast<const MultiMacroMethods*>(this) != nullptr);
}


/*@NOTE_41439
Method that returns true if it is actually a MultiOwnedMacroMethods Object.
*/
bool Gti::IsMultiOwnedMacroMethods() const
{
    return (dynamic_cast<const MultiOwnedMacroMethods*>(this) != nullptr);
}


/*@NOTE_41387
Method that returns true if it is actually a OtherType Object.
*/
bool Gti::IsOtherType() const
{
    return (dynamic_cast<const OtherType*>(this) != nullptr);
}


/*@NOTE_41431
Method that returns true if it is actually a OtherTypes Object.
*/
bool Gti::IsOtherTypes() const
{
    return (dynamic_cast<const OtherTypes*>(this) != nullptr);
}


/*@NOTE_41421
Method that returns true if it is actually a RemoveReferencesMethod Object.
*/
bool Gti::IsRemoveReferencesMethod() const
{
    return (dynamic_cast<const RemoveReferencesMethod*>(this) != nullptr);
}


/*@NOTE_41410
Method that returns true if it is actually a ReplaceConstructor Object.
*/
bool Gti::IsReplaceConstructor() const
{
    return (dynamic_cast<const ReplaceConstructor*>(this) != nullptr);
}


/*@NOTE_41415
Method that returns true if it is actually a ReplaceConstructorIncludeMethod Object.
*/
bool Gti::IsReplaceConstructorIncludeMethod() const
{
    return (dynamic_cast<const ReplaceConstructorIncludeMethod*>(this) != nullptr);
}


/*@NOTE_41422
Method that returns true if it is actually a RestoreReferencesMethod Object.
*/
bool Gti::IsRestoreReferencesMethod() const
{
    return (dynamic_cast<const RestoreReferencesMethod*>(this) != nullptr);
}


/*@NOTE_41423
Method that returns true if it is actually a SaveReferencesMethod Object.
*/
bool Gti::IsSaveReferencesMethod() const
{
    return (dynamic_cast<const SaveReferencesMethod*>(this) != nullptr);
}


/*@NOTE_41446
Method that returns true if it is actually a SequenceDiagram Object.
*/
bool Gti::IsSequenceDiagram() const
{
    return (dynamic_cast<const SequenceDiagram*>(this) != nullptr);
}


/*@NOTE_41409
Method that returns true if it is actually a SerializeConstructor Object.
*/
bool Gti::IsSerializeConstructor() const
{
    return (dynamic_cast<const SerializeConstructor*>(this) != nullptr);
}


/*@NOTE_41414
Method that returns true if it is actually a SerializeConstructorIncludeMethod Object.
*/
bool Gti::IsSerializeConstructorIncludeMethod() const
{
    return (dynamic_cast<const SerializeConstructorIncludeMethod*>(this) != nullptr);
}


/*@NOTE_41417
Method that returns true if it is actually a SerializeMethod Object.
*/
bool Gti::IsSerializeMethod() const
{
    return (dynamic_cast<const SerializeMethod*>(this) != nullptr);
}


/*@NOTE_41418
Method that returns true if it is actually a SerializeRelationsMethod Object.
*/
bool Gti::IsSerializeRelationsMethod() const
{
    return (dynamic_cast<const SerializeRelationsMethod*>(this) != nullptr);
}


/*@NOTE_41397
Method that returns true if it is actually a SetMemberMethod Object.
*/
bool Gti::IsSetMemberMethod() const
{
    return (dynamic_cast<const SetMemberMethod*>(this) != nullptr);
}


/*@NOTE_41440
Method that returns true if it is actually a SingleMacroMethods Object.
*/
bool Gti::IsSingleMacroMethods() const
{
    return (dynamic_cast<const SingleMacroMethods*>(this) != nullptr);
}


/*@NOTE_41441
Method that returns true if it is actually a SingleOwnedMacroMethods Object.
*/
bool Gti::IsSingleOwnedMacroMethods() const
{
    return (dynamic_cast<const SingleOwnedMacroMethods*>(this) != nullptr);
}


/*@NOTE_41442
Method that returns true if it is actually a StaticMultiMacroMethods Object.
*/
bool Gti::IsStaticMultiMacroMethods() const
{
    return (dynamic_cast<const StaticMultiMacroMethods*>(this) != nullptr);
}


/*@NOTE_41443
Method that returns true if it is actually a StaticMultiOwnedMacroMethods Object.
*/
bool Gti::IsStaticMultiOwnedMacroMethods() const
{
    return (dynamic_cast<const StaticMultiOwnedMacroMethods*>(this) != nullptr);
}


/*@NOTE_41427
Method that returns true if it is actually a ToRelation Object.
*/
bool Gti::IsToRelation() const
{
    return (dynamic_cast<const ToRelation*>(this) != nullptr);
}


/*@NOTE_41444
Method that returns true if it is actually a ToRelationMacroMethods Object.
*/
bool Gti::IsToRelationMacroMethods() const
{
    return (dynamic_cast<const ToRelationMacroMethods*>(this) != nullptr);
}


/*@NOTE_41386
Method that returns true if it is actually a Type Object.
*/
bool Gti::IsType() const
{
    return (dynamic_cast<const Type*>(this) != nullptr);
}


/*@NOTE_41391
Method that returns true if it is actually a Variable Object.
*/
bool Gti::IsVariable() const
{
    return (dynamic_cast<const Variable*>(this) != nullptr);
}


/*@NOTE_41398
Method that returns true if it is actually a WrapMemberMethod Object.
*/
bool Gti::IsWrapMemberMethod() const
{
    return (dynamic_cast<const WrapMemberMethod*>(this) != nullptr);
}


/*@NOTE_5451
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Gti::RemoveReferences()
{
    REMOVE_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_PASSIVE(Gti, Parent, Gti, Child)
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
}


/*@NOTE_1558
Method which must be called first in a replace constructor
*/
void Gti::ReplaceConstructorInclude(Gti* pOld)
{
    REPLACE_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    REPLACE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    REPLACE_MULTI_PASSIVE(Gti, Parent, Gti, Child)
}


/*@NOTE_5452
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Gti::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti* pGti = (Gti*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    RESTORE_MULTI_PASSIVE(Gti, Parent, Gti, Child)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5454
Save the state of the current object relations to pDataModelDocObject.
*/
void Gti::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    Gti* pGti = (Gti*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    SAVE_MULTI_PASSIVE(Gti, Parent, Gti, Child)
}


/*@NOTE_63
Serialize the members only to a CbObject object
*/
void Gti::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _added;
        archive << _itemText;
        archive << _icon;
        archive << _initialVersion;
        archive << _version;
        archive << _order;
        archive << _addInString;
        archive << int(_phase);
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _added;
            archive >> _itemText;
            archive >> _icon;
            archive >> _initialVersion;
            archive >> _version;
            archive >> _order;
            archive >> _addInString;
            int phaseTmp;
            archive >> phaseTmp;
            _phase = (Phase)phaseTmp;
        }
    }
}


/*@NOTE_62
Method which must be called first in a serialize constructor
*/
void Gti::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    INIT_MULTI_PASSIVE(Gti, Parent, Gti, Child)
}


/*@NOTE_65
Serialize the relations to a CbObject object
*/
void Gti::SerializeRelations(CbArchive& archive,
                             DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Gti, Parent, Gti, Child)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Gti)


// Methods for the relation(s) of the class
METHODS_MULTI_ACTIVE(Gti, Parent, Gti, Child)
METHODS_ITERATOR_MULTI_ACTIVE(Gti, Parent, Gti, Child)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
METHODS_MULTI_PASSIVE(Gti, Parent, Gti, Child)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
