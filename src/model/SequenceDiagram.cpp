/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SequenceDiagram.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SequenceDiagram'
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
#include <iostream>
using namespace std;
#include "ClassBuilderDoc.h"
#include "MainFrm.h"
#include "CbPainter.h"
#include "qt/QtSequenceDiagramDialog.h"
#include "qt/QtSequenceDiagramView.h"   // Qt_ShowSequenceDiagramView (open-diagram)
#include <algorithm>
//@END_USER2


// Static members
unsigned short SequenceDiagram::_activationWidth = 20;
unsigned short SequenceDiagram::_activationOffsetRecursive = 10;
unsigned short SequenceDiagram::_activationMinimalHeight = 40;
unsigned short SequenceDiagram::_activationSpaceRecursive = 20;
unsigned short SequenceDiagram::_activationSpaceBefore = 25;
unsigned short SequenceDiagram::_activationSpaceAfter = 20;
unsigned short SequenceDiagram::_classLifeLineHeight = 72;
unsigned short SequenceDiagram::_classLifeLineOffset = 272;
unsigned short SequenceDiagram::_signalLengthRecursive = 60;


/*@NOTE_29944
Constructor method.
*/
SequenceDiagram::SequenceDiagram(Gti* pGti) //@INIT_29944
    : Gti(pGti->GetDataModelDoc())
    , _height(2970)
    , _multiPage(0)
    , _name()
    , _note()
    , _width(2100)
    , _scale(80)
    , _numbering(SEQ_NONE)
    , _arguments(false)
    , _scope(false)
    , _argumentNames(false)
    , _caption()
    , _moveOnce(true)
{//@CODE_29944
    ConstructorInclude(pGti->GetDataModelDoc());

    // Put in your own code
    Gti::ChildIterator iChild(pGti);
    while (--iChild)
    {
        if (iChild->IsClassDiagram() || iChild->IsSequenceDiagram())
        {
            SetOrder(iChild->GetOrder()+1);
            break;
        }
    }
    
    pGti->AddChildLast(this);

    if (pGti->GetPhase() > Design_Phase)
    {
        SetPhase(Implementation_Phase);
    }
    else
    {
        SetPhase(pGti->GetPhase());
    }

    (void)new RootActivationShape(this);
}//@CODE_29944


/*@NOTE_35086
Constructor method.
*/
SequenceDiagram::SequenceDiagram(Gti* pGti,
                                 SequenceDiagram* pSequenceDiagram) //@INIT_35086
    : Gti(pGti->GetDataModelDoc())
    , _height(2970)
    , _multiPage(0)
    , _name()
    , _note()
    , _width(2100)
    , _scale(80)
    , _numbering(SEQ_NONE)
    , _arguments(false)
    , _scope(false)
    , _argumentNames(false)
    , _caption()
    , _moveOnce(true)
{//@CODE_35086
    ConstructorInclude(pGti->GetDataModelDoc());

    // Put in your own code
    Gti::ChildIterator iChild(pGti);
    while (--iChild)
    {
        if (iChild->IsClassDiagram() || iChild->IsSequenceDiagram())
        {
            SetOrder(iChild->GetOrder()+1);
            break;
        }
    }

    pGti->AddChildLast(this);

    CopyState(pSequenceDiagram);

    SetPhase(pSequenceDiagram->GetPhase());
    SetAdded(false);

    pSequenceDiagram->GetRootActivationShape()->CopyShape(this);
    LifeLineShapeIterator iLifeLineShape(pSequenceDiagram);
    while (++iLifeLineShape)
    {
        iLifeLineShape->CopyShape(this);
    }
    pSequenceDiagram->GetRootActivationShape()->ParentActivationShape::CopyShape(this);

    SequenceDiagramShapeIterator iSequenceDiagramShape(pSequenceDiagram, &SequenceDiagramShape::IsSDNoteShape);
    while (++iSequenceDiagramShape)
    {
        iSequenceDiagramShape->CopyShape(this);
    }
}//@CODE_35086


/*@NOTE_29649
Constructor needed for serialization, not meant to use for other purposes!
*/
SequenceDiagram::SequenceDiagram() //@INIT_29649
    : Gti()
    , _scale(80)
    , _numbering(SEQ_a)
    , _arguments(false)
    , _scope(false)
    , _argumentNames(false)
    , _caption()
    , _moveOnce(true)
{//@CODE_29649
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_29649


/*@NOTE_29612
Destructor method.
*/
SequenceDiagram::~SequenceDiagram()
{//@CODE_29612
    DestructorInclude();

    // Put in your own code
}//@CODE_29612


void SequenceDiagram::Add()
{//@CODE_30102
    if (!GetAdded())
    {
        SetItemText(GetName());
        SetIcon(ICON_SEQUENCEDIAGRAM);

        Gti::Add();
    }
}//@CODE_30102


void SequenceDiagram::CheckAndUpdatePhase()
{//@CODE_35990
    if (GetDataModelDoc()->GetDataModel()->GetPhaseSupport())
    {
        Phase minPhase = Complete_Phase;
    
        SequenceDiagramShapeIterator iSequenceDiagramShape(this);
        while (++iSequenceDiagramShape)
        {
            ChildActivationShape* pChildActivationShape = 
                dynamic_cast<ChildActivationShape*>(iSequenceDiagramShape.Get());
            if (pChildActivationShape && pChildActivationShape->GetMethod() &&
                pChildActivationShape->GetMethod()->GetPhase() && 
                pChildActivationShape->GetMethod()->GetPhase() < minPhase)
            {
                minPhase = pChildActivationShape->GetMethod()->GetPhase();
            }
        }
    
        if (minPhase != GetPhase())
        {
            SetPhaseUpwards(minPhase);
        }
    }
}//@CODE_35990


bool SequenceDiagram::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_30093
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        pGtiDropDefault = GetParent();
        Gti::ChildIterator iChild(pGtiDropDefault, NULL, this);
        while (--iChild)
        {
            if (iChild->IsClassDiagram() || iChild->IsSequenceDiagram())
            {
                pGtiDropDefault = iChild;
                break;
            }
        }

        Remove();
        value = true;
    }

    return value;
}//@CODE_30093


void SequenceDiagram::Draw(CbPainter& painter,
                           SequenceDiagramViewModel* pSequenceDiagramViewModel)
{//@CODE_40441
    // Paint is read-only (see ClassDiagram::Draw) -- no draw guard; the _isDrawing
    // scaffold retired once the note-height recompute moved to the edit boundary.
    LifeLineShapeIterator iLifeLineShape(this);
    while (++iLifeLineShape)
    {
        iLifeLineShape->Draw(painter, pSequenceDiagramViewModel,
            iLifeLineShape->IsSelectedIn(pSequenceDiagramViewModel));
    }

    ParentActivationShape::ChildActivationShapeIterator
        iChildActivationShape(GetRootActivationShape());
    while (++iChildActivationShape)
    {
        iChildActivationShape->Draw(painter, pSequenceDiagramViewModel,
            iChildActivationShape->IsSelectedIn(pSequenceDiagramViewModel));
    }

    SequenceDiagramShapeIterator iSequenceDiagramShape(this, &SequenceDiagramShape::IsSDNoteShape);
    while (++iSequenceDiagramShape)
    {
        iSequenceDiagramShape->Draw(painter, pSequenceDiagramViewModel,
            iSequenceDiagramShape->IsSelectedIn(pSequenceDiagramViewModel));
    }
}//@CODE_40441


void SequenceDiagram::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_30096
    if (ctrlKeyDown)
    {
    }
    else
    {
        if (pGtiDrop)
        {
            SaveState(1);
            if (pGtiDrop->IsClassDiagram() || pGtiDrop->IsSequenceDiagram())
            {
                pGtiDrop->GetParent()->AddChildAfter(this, pGtiDrop);
            }
            else
            {
                pGtiDrop->AddChildFirst(this);
            }

            int i = 0;
            Gti::ChildIterator iChild(GetParent());
            while (++iChild)
            {
                if (iChild->IsClassDiagram() || iChild->IsSequenceDiagram())
                {
                    iChild->SaveState(1);
                    iChild->SetOrder(i++);
                }
            }
        }
        Add();
    }
}//@CODE_30096


bool SequenceDiagram::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_30099
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
    }
    else
    {
        if (pGtiDrop->IsDataModel() || pGtiDrop->IsMetaGroup() || 
            pGtiDrop->IsClassGroup() || pGtiDrop->IsBaseClass() ||
            pGtiDrop->IsClassDiagram() || pGtiDrop->IsSequenceDiagram())
        {
            value = true;
        }
    }

    return value;
}//@CODE_30099


CbRect SequenceDiagram::GetBoundingRect()
{//@CODE_34113
    RecalculateDiagram();
    CbRect rect(0, 0, 0, 0);
    
    SequenceDiagramShapeIterator iSequenceDiagramShape(this);
    while (++iSequenceDiagramShape)
    {
        rect *= iSequenceDiagramShape->GetBoundingRect();
    }

    return rect;
}//@CODE_34113


SequenceDiagramShape* SequenceDiagram::GetHitShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                                   CbPoint pointLP, bool nested)
{//@CODE_40893
    SequenceDiagramShape* pSequenceDiagramShape = 0;

    SequenceDiagramShapeIterator iSequenceDiagramShape(this);
    while (!pSequenceDiagramShape && --iSequenceDiagramShape)
    {
        if (iSequenceDiagramShape->PointInShape(pSequenceDiagramViewModel, pointLP))
        {
            pSequenceDiagramShape = iSequenceDiagramShape->GetHitShape(pSequenceDiagramViewModel, pointLP, nested);
        }
    }

    return pSequenceDiagramShape;
}//@CODE_40893


unsigned short SequenceDiagram::GetLifeLineHeight()
{//@CODE_34919
    return SequenceDiagram::GetActivationSpaceBefore() + -2 +
           GetRootActivationShape()->GetHeight() +
           SequenceDiagram::GetActivationSpaceAfter();
}//@CODE_34919


/*@NOTE_34783
Move the noteshape points within 'rect' with 'offset'.
*/
void SequenceDiagram::MoveNoteShapePoints(const CbRect& rect,
                                          const CbSize& offset)
{//@CODE_34783
    SequenceDiagramShapeIterator iSequenceDiagramShape(this);
    while (++iSequenceDiagramShape)
    {
        SDNoteShape* pSDNoteShape = dynamic_cast<SDNoteShape*>(iSequenceDiagramShape.Get());
        if (pSDNoteShape)
        {
            pSDNoteShape->MoveNoteShapePoints(rect, offset);
        }
    }
}//@CODE_34783


/*@NOTE_34913
Move the noteshape points within 'oldRect' with 'newRect'.
*/
void SequenceDiagram::MoveNoteShapePoints(const CbRect& oldRect,
                                          const CbRect& newRect)
{//@CODE_34913
    SequenceDiagramShapeIterator iSequenceDiagramShape(this);
    while (++iSequenceDiagramShape)
    {
        SDNoteShape* pSDNoteShape = dynamic_cast<SDNoteShape*>(iSequenceDiagramShape.Get());
        if (pSDNoteShape)
        {
            pSDNoteShape->MoveNoteShapePoints(oldRect, newRect);
        }
    }
}//@CODE_34913


int SequenceDiagram::OnAddClassDiagram(bool checkOnly)
{//@CODE_34525
    if (!checkOnly)
    {
        UndoBase* pLastUndo = GetDataModelDoc()->MarkLastUndo();
        ClassDiagram* pClassDiagram = new ClassDiagram(GetParent());
        pClassDiagram->SetName(GetName());
             
        if (pClassDiagram->OnEditAttributes())
        {
            pClassDiagram->Add();
            pClassDiagram->OnOpen(false);
            
            LifeLineShapeIterator iLifeLineShape(this);
            while (++iLifeLineShape)
            {
                ClassLifeLineShape* pClassLifeLineShape = 
                    iLifeLineShape->GetClassLifeLine();
                
                if (pClassLifeLineShape)
                {
                    if (!pClassLifeLineShape->GetBaseClass()->FindClassShape(pClassDiagram))
                    {
                        (void)new ClassShape(pClassDiagram, 
                            pClassLifeLineShape->GetBaseClass(), CbPoint(0, 0));
                    }
                    
                    LifeLineShape::ChildActivationShapeIterator 
                        iChildActivationShape(pClassLifeLineShape);
                    while (++iChildActivationShape)
                    {
                        Method* pMethod = iChildActivationShape->GetMethod();
                        if (pMethod)
                        {
                            ClassShape* pClassShape = 
                                pMethod->GetBaseClass()->FindClassShape(pClassDiagram);
                            if (!pClassShape)
                            {
                                pClassShape = new ClassShape(pClassDiagram, 
                                    pMethod->GetBaseClass(), CbPoint(0, 0));
                            }
                            
                            if (!pClassShape->FindMethodShape(pMethod))
                            {
                                (void)new MethodShape(pClassShape, pMethod);
                            
                                MemberMethod* pMemberMethod = 
                                    dynamic_cast<MemberMethod*>(pMethod);
                                if (pMemberMethod && 
                                    !pClassShape->FindMemberShape(pMemberMethod->GetMember()))
                                {
                                    (void)new MemberShape(pClassShape, pMemberMethod->GetMember());
                                }
                            }

                            pClassShape->RecalculateRect();
                        }
                    }
                }
            }
            
            Grid grid(pClassDiagram);
            grid.Place();
        }
        else
            GetDataModelDoc()->RollBack(pLastUndo);
    }

    return 1;
}//@CODE_34525


int SequenceDiagram::OnDelete(bool checkOnly)
{//@CODE_30103
    if (!checkOnly)
    {
        if (GetSequenceDiagramViewModelCount() == 0)
        {
            CbString str;
            str.Format("Are you sure you want to delete sequence diagram '%s'",
                GetName().c_str());
            if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_OKCANCEL) == CBMB_IDOK)
            {
                Delete();
            }
        }
        else
        {
            CbString str;
            str.Format("Can not delete sequence diagram '%s' views on it are still open, close them first.",
                GetName().c_str());
            CbMessageBox(str, CBMB_ICONEXCLAMATION);
        }
    }

    return (GetSequenceDiagramViewModelCount() ? 0: 1);
}//@CODE_30103


int SequenceDiagram::OnEditAttributes(bool checkOnly)
{//@CODE_30105
    if (checkOnly)
        return 1;
    
    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    bool sizeChanged = false;
    if (Qt_ShowSequenceDiagramDialog(this, modelChanged, sizeChanged, ownerHwnd))
    {
        // Coalesce the size-repaint and Update()'s rebuild into one refresh
        // (CbViewLock also shows the wait cursor).
        CbViewLock lock(GetDataModelDoc());

        // The Qt dialog applied the page width/height to the model; repaint
        // the open Qt canvases so they pick up the new page size.
        if (sizeChanged)
            UpdateSequenceDiagramViews();

        if (modelChanged)
            Update();

        return 1;
    }

    return 0;
}//@CODE_30105


int SequenceDiagram::OnOpen(bool checkOnly)
{//@CODE_30107
    if (!checkOnly)
    {
        // Open the diagram as a Qt canvas (the MFC MDI view was removed).
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowSequenceDiagramView(this, ownerHwnd);
    }

    return 1;
}//@CODE_30107


/*@NOTE_30077
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void SequenceDiagram::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_30077
    Gti::OnUndoRedoChanged(pOldState);

    SequenceDiagram* pSequenceDiagram = (SequenceDiagram*)pOldState;
    
    if (pSequenceDiagram &&
        GetWidth() != pSequenceDiagram->GetWidth() && 
        GetHeight() != pSequenceDiagram->GetHeight())
    {
        UpdateSequenceDiagramViews();
    }
    
}//@CODE_30077


void SequenceDiagram::OptimizePlacement()
{//@CODE_38584
    // Undo coverage: full. The algorithm runs entirely on _order (a
    // non-serialized helper) while the physical list stays in its
    // original order. Once the target _order is settled, we apply it
    // via single-pair adjacent swaps using CB's MoveLifeLineShapeBefore
    // primitive; each swap is its own sub-batch (SaveState on the moved
    // LL + MarkLastUndo(2)) so RESTORE_MULTI_PASSIVE only ever sees one
    // pair at a time on undo — the case it handles correctly. The
    // view's MarkLastUndo() at the end of OnOptimizeplacement upgrades
    // the last sub-marker to a level-1 marker, closing the user-visible
    // undo step.

    // Gather pass: build one Contrib per unordered (a,b) lifeline pair,
    // with _needed accumulating the count of signals exchanged. Self-loops
    // (sender==receiver) are skipped by SignalShape::AddContribution().
    Contrib::DeleteAllContrib();

    LifeLineShapeIterator iSndLL(this);
    while (++iSndLL)
    {
        LifeLineShape::ChildActivationShapeIterator iAct(iSndLL);
        while (++iAct)
        {
            ChildActivationShape::ReceiverIterator iSig(iAct);
            while (++iSig)
            {
                iSig->AddContribution();
            }
        }
    }

    // Seed _order from the current left-to-right list position.
    int order = 1;
    iSndLL.Reset();
    while (++iSndLL)
    {
        iSndLL->SetOrder(order++);
        iSndLL->SetOrderWeight((float)iSndLL->GetOrder());
    }

    // Weighted-median barycenter — entirely on _order, no physical sort.
    // Each sweep computes a new _orderWeight per lifeline from the
    // weighted median of its connected neighbours' current _order values
    // (weights = Contrib::GetNeeded()), then re-ranks _order from the
    // new weights. Disconnected lifelines stay put.
    //
    // Median is used (not mean) — mean is much more sensitive to seed
    // order; median converges to the same result regardless of start.
    //
    // Neighbours are insertion-sorted by _order into a small stack
    // array, so we don't have to (and don't) physically reorder the
    // lifeline list. Keeping the list untouched is what makes the final
    // single-pair-swap apply step undoable.
    const int MAX_NBRS = 64;
    struct NbrEntry { int order; int weight; };
    const int MAX_LLS = 256;
    struct RankEntry { LifeLineShape* pLL; int newOrder; };

    const int MAX_SWEEPS = 24;
    for (int sweep = 0; sweep < MAX_SWEEPS; ++sweep)
    {
        // Pass 1: weighted-median orderWeight per LL.
        LifeLineShapeIterator iLL(this);
        while (++iLL)
        {
            LifeLineShape* pLL = iLL.Get();

            NbrEntry nbrs[MAX_NBRS];
            int nbrCount = 0;
            Contrib::ContribIterator iC;
            while (++iC)
            {
                LifeLineShape* pOther = NULL;
                if (iC->GetLeft() == pLL)        pOther = iC->GetRight();
                else if (iC->GetRight() == pLL)  pOther = iC->GetLeft();
                if (!pOther || nbrCount >= MAX_NBRS) continue;

                int o = pOther->GetOrder();
                int w = iC->GetNeeded();
                int k = nbrCount;
                while (k > 0 && nbrs[k - 1].order > o)
                {
                    nbrs[k] = nbrs[k - 1];
                    k--;
                }
                nbrs[k].order = o;
                nbrs[k].weight = w;
                nbrCount++;
            }

            if (nbrCount == 0)
            {
                pLL->SetOrderWeight((float)pLL->GetOrder());
                continue;
            }

            int total = 0;
            for (int k = 0; k < nbrCount; ++k) total += nbrs[k].weight;

            int targetHalf = (total + 1) / 2;
            int cumulative = 0;
            float median = (float)nbrs[nbrCount - 1].order;
            for (int k = 0; k < nbrCount; ++k)
            {
                cumulative += nbrs[k].weight;
                if (cumulative >= targetHalf)
                {
                    median = (float)nbrs[k].order;
                    if (cumulative * 2 == total && k + 1 < nbrCount)
                        median = (median + (float)nbrs[k + 1].order) / 2.0f;
                    break;
                }
            }
            pLL->SetOrderWeight(median);
        }

        // Pass 2: rank by (_orderWeight, old _order). Compute into a
        // temp array first so other LLs still see the *old* _order
        // during ranking.
        RankEntry ranks[MAX_LLS];
        int rankCount = 0;
        LifeLineShapeIterator iA(this);
        while (++iA && rankCount < MAX_LLS)
        {
            LifeLineShape* pA = iA.Get();
            int rank = 1;
            LifeLineShapeIterator iB(this);
            while (++iB)
            {
                LifeLineShape* pB = iB.Get();
                if (pB == pA) continue;
                if (pB->GetOrderWeight() < pA->GetOrderWeight() ||
                    (pB->GetOrderWeight() == pA->GetOrderWeight() &&
                     pB->GetOrder() < pA->GetOrder()))
                    rank++;
            }
            ranks[rankCount].pLL = pA;
            ranks[rankCount].newOrder = rank;
            rankCount++;
        }

        bool changed = false;
        for (int i = 0; i < rankCount; ++i)
        {
            if (ranks[i].pLL->GetOrder() != ranks[i].newOrder)
                changed = true;
            ranks[i].pLL->SetOrder(ranks[i].newOrder);
        }

        if (!changed)
            break;
    }

    // Local-search refinement on top of the barycenter result. Cost is
    // total signal span: Sum(Contrib.GetNeeded() * |right._order -
    // left._order|) — i.e. how far each message has to reach, weighted
    // by how often it's sent. Reducing span directly reduces the number
    // of intermediate lifelines that long-distance arrows cross. We
    // try every adjacent pair; if swapping their _order values lowers
    // total span we accept the swap, else revert. Sweep until a full
    // pass produces no improvement (a local minimum).
    //
    // Anchor bias: the lifeline that hosts the first child activation
    // under the root (= where the first signal lands, often the actor)
    // gets a small "prefer leftward" penalty added to the cost. The
    // weight is modest, so it acts as a tie-breaker plus a gentle nudge
    // — it won't override a real crossing reduction, but in equally-
    // good layouts it keeps the kick-off lifeline on the left where
    // readers expect it.
    //
    // Operates on _order values, not list position — the physical list
    // re-sort happens once at the end. O(N^2 * C) per sweep where N is
    // lifeline count and C is Contrib count; fine for the small N
    // sequence diagrams usually have.
    LifeLineShape* pAnchor = NULL;
    if (GetRootActivationShape())
    {
        ChildActivationShape* pFirst = GetRootActivationShape()->GetFirstChildActivationShape();
        if (pFirst)
            pAnchor = pFirst->GetLifeLineShape();
    }
    const int ANCHOR_WEIGHT = 2;

    const int MAX_REFINE_SWEEPS = 16;
    for (int rs = 0; rs < MAX_REFINE_SWEEPS; ++rs)
    {
        bool improved = false;
        LifeLineShape* pPrev = NULL;
        LifeLineShapeIterator iCurr(this);
        while (++iCurr)
        {
            LifeLineShape* pCurr = iCurr.Get();
            if (pPrev)
            {
                int oldCost = 0, newCost = 0;
                Contrib::ContribIterator iC;
                while (++iC)
                {
                    int la = iC->GetLeft()->GetOrder();
                    int ra = iC->GetRight()->GetOrder();
                    oldCost += iC->GetNeeded() * (la > ra ? la - ra : ra - la);
                }
                if (pAnchor)
                    oldCost += ANCHOR_WEIGHT * (pAnchor->GetOrder() - 1);

                int aOrd = pPrev->GetOrder();
                int bOrd = pCurr->GetOrder();
                pPrev->SetOrder(bOrd);
                pCurr->SetOrder(aOrd);

                iC.Reset();
                while (++iC)
                {
                    int la = iC->GetLeft()->GetOrder();
                    int ra = iC->GetRight()->GetOrder();
                    newCost += iC->GetNeeded() * (la > ra ? la - ra : ra - la);
                }
                if (pAnchor)
                    newCost += ANCHOR_WEIGHT * (pAnchor->GetOrder() - 1);

                if (newCost < oldCost)
                {
                    improved = true;
                }
                else
                {
                    pPrev->SetOrder(aOrd);
                    pCurr->SetOrder(bOrd);
                }
            }
            pPrev = pCurr;
        }
        if (!improved)
            break;
    }

    // Apply the target _order to the physical list. SortLifeLineShape
    // (METHOD_MULTI_SORT) is a bubble sort; CompareOrderWeight emits
    // SaveState + MarkLastUndo(2) on each swap-needed comparison, so
    // each adjacent flip is its own undo sub-batch and the whole
    // reorder is reversible.
    {
        LifeLineShapeIterator iApply(this);
        while (++iApply)
            iApply->SetOrderWeight((float)iApply->GetOrder());
        SortLifeLineShape(LifeLineShape::CompareOrderWeight);
    }

    Contrib::DeleteAllContrib();

    // Beautify: vertical activation offsets and horizontal spacing
    // based on the freshly chosen left-to-right order.
    ResetActivationOffsets();
    SpaceLifeLines();

    // Snap horizontally to a canonical leftmost position so the diagram
    // doesn't wander right when the user shifted a lifeline rightward
    // before running OptimizePlacement to perturb the seed order.
    // SpaceLifeLines anchors at the (post-reorder) first lifeline's
    // current x; without this snap, the entire pack inherits that drift.
    // Follow the canonical-leftmost snap below: snapshot lifeline rects now
    // (after the reorder + ResetActivationOffsets + SpaceLifeLines, which each
    // already followed their own moves), so the diff after the dx shift carries
    // the notes by that shift.
    std::vector<std::pair<LifeLineShape*, CbRect> > oldHeaders;
    std::vector<std::pair<LifeLineShape*, CbRect> > oldBodies;
    {
        LifeLineShapeIterator iSnap(this);
        while (++iSnap)
        {
            oldHeaders.push_back(std::make_pair(iSnap.Get(), iSnap->GetRect()));
            oldBodies.push_back(std::make_pair(iSnap.Get(), iSnap->GetLifeLineRect()));
        }
    }

    static const int CANONICAL_LEFTMOST = 100;
    int currentLeftmost = INT_MAX;
    {
        LifeLineShapeIterator iLB(this);
        while (++iLB)
        {
            int br = iLB->GetBoundingRect().left;
            if (br < currentLeftmost)
                currentLeftmost = br;
        }
    }
    int dx = CANONICAL_LEFTMOST - currentLeftmost;
    if (dx != 0 && currentLeftmost != INT_MAX)
    {
        LifeLineShapeIterator iLS(this);
        while (++iLS)
        {
            ClassLifeLineShape* pCLL = dynamic_cast<ClassLifeLineShape*>(iLS.Get());
            iLS->SetRectNoSort(iLS->GetRect() + CbSize(dx, 0));
            if (pCLL)
                pCLL->SetTemplateRect(pCLL->GetTemplateRect() + CbSize(dx, 0));
        }
    }

    // Diff the snap's lifeline moves, recompute, then carry the notes (the reorder +
    // ResetActivationOffsets + SpaceLifeLines above already followed their own steps;
    // this carries the notes by the canonical-leftmost shift).
    std::vector<std::pair<CbRect, CbRect> > moved;
    for (std::vector<std::pair<LifeLineShape*, CbRect> >::iterator it = oldHeaders.begin();
         it != oldHeaders.end(); ++it)
        if (it->first->GetRect() != it->second)
            moved.push_back(std::make_pair(it->second, it->first->GetRect()));
    for (std::vector<std::pair<LifeLineShape*, CbRect> >::iterator it = oldBodies.begin();
         it != oldBodies.end(); ++it)
        if (it->first->GetLifeLineRect() != it->second)
            moved.push_back(std::make_pair(it->second, it->first->GetLifeLineRect()));
    RecalculateDiagram(&moved);
    ResolveNoteFollows(moved);
    UpdateSequenceDiagramViews();
}//@CODE_38584


void SequenceDiagram::RecalculateAfterEdit(SequenceDiagramShape* pSDS,
                                           const CbSize& offset,
                                           const CbPoint& noteEndPoint,
                                           const CbRect& width)
{//@CODE_41349
    std::vector<std::pair<CbRect, CbRect> > moved;

    // --- apply the reported edit (formerly scattered across the Qt commit handlers).
    // A zero-offset / no-width / no-point call applies nothing -- it's a pure
    // recompute-and-follow, used after a structural reorder has moved shapes. ---
    if (pSDS)
    {
        const CbPoint noPoint(INT_MIN, INT_MIN);
        const bool isMove   = (offset != CbSize(0, 0));
        const bool hasWidth = (width != CbRect(0, 0, 0, 0));
        if (SDNoteShape* pNote = pSDS->GetNoteShape())
        {
            if (noteEndPoint != noPoint)            // attach-line end-point drag
            {
                SDNoteShape::SDNoteShapePointIterator iPt(pNote);
                while (++iPt)
                    if (iPt->GetPoint() == noteEndPoint)
                    {
                        iPt->SetPoint(noteEndPoint + offset);   // SetPoint SaveStates
                        break;
                    }
            }
            else if (isMove)                        // body move
                pNote->SetRect(pNote->GetRect() + offset);
            else if (hasWidth)                      // width change
                pNote->SetRect(width);
        }
        else if (pSDS->IsLifeLineShape())
        {
            if (isMove || hasWidth)
            {
                // Resize turns off auto-width; then both move and resize just throw
                // the old + new header (and body line) rects into the move-list and
                // let ResolveNoteFollows carry the notes -- no edge special-casing.
                if (hasWidth)
                    if (ClassLifeLineShape* pCLL = dynamic_cast<ClassLifeLineShape*>(pSDS))
                        pCLL->SetAutoWidth(false);
                pSDS->SaveState(1);
                LifeLineShape* pLL = pSDS->GetLifeLine();
                CbRect oldHeader = pSDS->GetRect();
                CbRect oldBody   = pLL ? pLL->GetLifeLineRect() : CbRect(0, 0, 0, 0);
                pSDS->SetRect(isMove ? oldHeader + offset : width);
                if (pSDS->GetRect() != oldHeader)
                    moved.push_back(std::make_pair(oldHeader, pSDS->GetRect()));
                if (pLL && pLL->GetLifeLineRect() != oldBody)
                    moved.push_back(std::make_pair(oldBody, pLL->GetLifeLineRect()));
            }
        }
        else if (ChildActivationShape* pCA = pSDS->GetChildActivation())
        {
            if (offset.cy != 0)
            {
                pCA->SaveState(1);
                pCA->SetOffset(pCA->GetOffset() - offset.cy);   // signal move (vertical)
            }
        }
    }

    // Re-derive geometry (RecalculateDiagram records the activation/creation/signal moves
    // on top of the apply above), carry the attached note end-points by everything in
    // `moved`, then repaint. Always follows -- this entry is only reached for an edit or
    // a delete.
    RecalculateDiagram(&moved);
    ResolveNoteFollows(moved);
    UpdateSequenceDiagramViews();
}//@CODE_41349


void SequenceDiagram::RecalculateDiagram(std::vector<std::pair<CbRect,CbRect>>* moved)
{//@CODE_34267
    // The recompute helper. Re-derives ALL the diagram's derived geometry and, when
    // `moved` is given, records each changed rect (old -> new) into it: the activation
    // rects, the creation-lifeline tops, and the signal active areas. That list is what
    // ResolveNoteFollows then carries the attached note end-points by. `moved` == nullptr
    // recomputes without recording (GetBoundingRect, load / undo / redo). Builds geometry
    // only -- it does NOT follow notes and does NOT repaint (those are the caller's job).
    int sequenceNumber = 1;
    GetRootActivationShape()->RecalculateRect(
        GetClassLifeLineOffset()+GetActivationSpaceBefore()-2,
        sequenceNumber, moved);

    // A creation lifeline's top tracks its creation activation's bottom (the compute
    // used to live only in ClassLifeLineShape::Draw, leaving a stale top at the edit).
    LifeLineShapeIterator iCreationLifeLine(this);
    while (++iCreationLifeLine)
    {
        ClassLifeLineShape* pCLL = dynamic_cast<ClassLifeLineShape*>(iCreationLifeLine.Get());
        if (pCLL && pCLL->GetFirstChildActivationShape() &&
            pCLL->GetFirstChildActivationShape()->GetCreation())
        {
            CbRect oldRect = pCLL->GetRect();
            CbRect rect = oldRect;
            rect.top = pCLL->GetFirstChildActivationShape()->GetRect().bottom;
            rect.bottom = rect.top + GetClassLifeLineHeight();
            pCLL->SetRect(rect);
            if (moved && rect != oldRect)
                moved->push_back(std::make_pair(oldRect, rect));
        }
    }

    // Signals: stored active area (old) vs recomputed (new).
    SequenceDiagramShapeIterator iSequenceDiagramShape(this);
    while (++iSequenceDiagramShape)
    {
        SignalShape* pSignalShape =
            dynamic_cast<SignalShape*>(iSequenceDiagramShape.Get());
        if (!pSignalShape)
            continue;

        CbSize inflate(0, 10);
        CbRect activeAreaRect;
        if (pSignalShape->IsRecursiveActivation())
        {
            CbPoint point1 = pSignalShape->GetStartPoint()
                + CbSize(GetSignalLengthRecursive(), 0)
                + CbSize(0, -pSignalShape->GetDuration()/2);
            CbPoint point2 = point1 + CbSize(0, -GetActivationSpaceRecursive());
            activeAreaRect = CbRect(pSignalShape->GetStartPoint()+inflate, point2-inflate);
        }
        else
        {
            activeAreaRect = CbRect(pSignalShape->GetStartPoint()+inflate,
                                    pSignalShape->GetEndPoint()-inflate);
        }
        activeAreaRect.NormalizeRect();
        if (moved && pSignalShape->GetActiveAreaRect() != activeAreaRect)
            moved->push_back(std::make_pair(pSignalShape->GetActiveAreaRect(), activeAreaRect));
        pSignalShape->SetActiveAreaRect(activeAreaRect);

        if (pSignalShape->GetEnableReturn())
        {
            CbRect returnActiveAreaRect(pSignalShape->GetReturnStartPoint()+inflate,
                                        pSignalShape->GetReturnEndPoint()-inflate);
            returnActiveAreaRect.NormalizeRect();
            if (moved && pSignalShape->GetReturnActiveAreaRect() != returnActiveAreaRect)
                moved->push_back(std::make_pair(pSignalShape->GetReturnActiveAreaRect(), returnActiveAreaRect));
            pSignalShape->SetReturnActiveAreaRect(returnActiveAreaRect);
        }
    }
}//@CODE_34267


void SequenceDiagram::ResetActivationOffsets()
{//@CODE_38304
    LifeLineShapeIterator iLifeLineShape(this);
    while (++iLifeLineShape)
    {
        LifeLineShape::ChildActivationShapeIterator iChildActivationShape(iLifeLineShape.Get());
        while (++iChildActivationShape)
        {
            iChildActivationShape->SaveState();
            if (iChildActivationShape->GetSender() &&
                iChildActivationShape->GetSender()->GetDuration())
            {
                iChildActivationShape->SetOffset(
                    iChildActivationShape->GetSender()->GetDuration());
            }
            else
            {
                iChildActivationShape->SetOffset(0);
            }
        }
    }
    // Multi-shape op: only the activation offsets changed -- recompute re-derives their
    // rects (recording the moves) and ResolveNoteFollows carries the attached notes.
    std::vector<std::pair<CbRect, CbRect> > moved;
    RecalculateDiagram(&moved);
    ResolveNoteFollows(moved);
    UpdateSequenceDiagramViews();
}//@CODE_38304


void SequenceDiagram::ResetDriftedSignalTextOffsets()
{//@CODE_38311
    // Threshold is asymmetric: x is tight (text width drives horizontal
    // layout directly) and y is loose (vertical drift doesn't hurt
    // horizontal packing, and label-y often gets nudged to dodge nearby
    // arrows). It is applied to the deviation from the default position,
    // not the absolute offset, so it matches what the user feels:
    // 'I moved it ~PAD from where it started.'
    static const int X_THRESHOLD = 30;
    static const int Y_THRESHOLD = 60;
    static const CbSize NAME_DEFAULT(30, 0);
    static const CbSize LABEL_DEFAULT(30, -30);
    static const CbSize RETURN_DEFAULT(30, 0);

    SequenceDiagramShapeIterator iShape(this, &SequenceDiagramShape::IsSignalShape);
    while (++iShape)
    {
        SignalShape* pSig = static_cast<SignalShape*>(iShape.Get());
        CbSize d;
        d = pSig->GetNameOffset() - NAME_DEFAULT;
        if (d.cx > X_THRESHOLD || d.cx < -X_THRESHOLD ||
            d.cy > Y_THRESHOLD || d.cy < -Y_THRESHOLD)
            pSig->SetNameOffset(NAME_DEFAULT);
        d = pSig->GetLabelOffset() - LABEL_DEFAULT;
        if (d.cx > X_THRESHOLD || d.cx < -X_THRESHOLD ||
            d.cy > Y_THRESHOLD || d.cy < -Y_THRESHOLD)
            pSig->SetLabelOffset(LABEL_DEFAULT);
        d = pSig->GetReturnOffset() - RETURN_DEFAULT;
        if (d.cx > X_THRESHOLD || d.cx < -X_THRESHOLD ||
            d.cy > Y_THRESHOLD || d.cy < -Y_THRESHOLD)
            pSig->SetReturnOffset(RETURN_DEFAULT);
    }
}//@CODE_38311


void SequenceDiagram::ResolveNoteFollows(const std::vector<std::pair<CbRect,CbRect>>& moved)
{//@CODE_41345
    if (moved.empty())
        return;

    SequenceDiagramShapeIterator iNote(this);
    while (++iNote)
        if (SDNoteShape* pNote = iNote->GetNoteShape())
            pNote->ResolveNoteFollows(moved);
}//@CODE_41345


void SequenceDiagram::SetPhaseDownAndUpwards(Phase phase)
{//@CODE_35988
    if (IsAllowedToEditPhase(phase) && phase != None_Phase && GetPhase() != phase)
    {
        SaveState();
        
        if (GetPhase() < phase)
        {
            SetPhase(phase);
        
            ChildIterator iChild(this);
            while (++iChild)
            {
                if (iChild->GetPhase() && iChild->GetPhase() < phase)
                {
                    iChild->SetPhaseDownAndUpwards(phase);
                }
            }
            
            // On top of the propagation via the tree, also propagate it to all methods used.
            SequenceDiagram::SequenceDiagramShapeIterator iSequenceDiagramShape(this);
            while (++iSequenceDiagramShape)
            {
                ChildActivationShape* pChildActivationShape = 
                    dynamic_cast<ChildActivationShape*>(iSequenceDiagramShape.Get());
                if (pChildActivationShape && pChildActivationShape->GetMethod() &&
                    pChildActivationShape->GetMethod()->GetPhase() && 
                    pChildActivationShape->GetMethod()->GetPhase() < phase)
                {
                    pChildActivationShape->GetMethod()->SetPhaseDownAndUpwards(phase);
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
            SetPhase(phase);
            
            if (GetParent() && GetParent()->GetPhase() > phase)
            {
                GetParent()->SetPhaseUpwards(phase);
            }
        }
        
        Gti::Update();
    }
}//@CODE_35988


void SequenceDiagram::SpaceLifeLines()
{//@CODE_34273
    // We use SetRectNoSort below — without the per-call SortLifeLineShape
    // there's no Iterator::CheckAll wiping our outer iterator, so the
    // straightforward live-list iteration works fine.
    //
    // Layout model: each lifeline has a target left position computed as
    // the maximum of all relative-distance constraints anchored to
    // lifelines on its left. We process strictly left-to-right and place
    // each lifeline at its target. There is no notion of "shift from the
    // current position" — the new position is determined entirely by the
    // constraints, so removing a long signal text causes the receiver and
    // its right neighbours to move LEFT, just as adding one moves them
    // right.
    //
    // Constraint kinds:
    //   1. Adjacent-rect floor: each lifeline's rect.left must be at least
    //      the previous lifeline's GetBoundingRect().right + MIN_RECT_GAP,
    //      so lifeline boxes never visually touch.
    //   2. Per-signal text length: for a signal connecting lifelines a<b,
    //      centerline-to-centerline distance must accommodate the text
    //      drawn along the arrow. Normal signals lose one full activation
    //      width to the activation bars on each end; constructor signals
    //      (receiver activation has Creation flag) end at the receiver
    //      class's box left edge instead of the activation, losing
    //      actW/2 + receiver.width/2 on the receiver side. The per-signal
    //      computation lives on SignalShape::GetRequiredLifelineDistance.
    //
    // The leftmost lifeline is anchored at its current position; the rest
    // are computed from there.
    static const int MIN_RECT_GAP = 10;     // floor whitespace between rects

    // OptimizePlacement is now horizontal-only. Resetting child activation
    // offsets to sender durations is a separate command (Reset Activation
    // Offsets) — that lets users keep manual vertical tweaks while still
    // re-running horizontal layout.

    // Measure signal text through the app-wide headless painter
    // (CbPainter_QFontMetrics, installed by the Qt app). It reports sizes in
    // the same logical units the Qt painter draws with -- so they match the
    // lifeline rects directly, with none of the old MM_ISOTROPIC DC dance.
    // Null only before the Qt app is up (pre-view); nothing to optimise then.
    CbPainter* pMeasure = CbPainter::GetMeasurePainter();
    if (!pMeasure)
        return;

    int savedMeasure = pMeasure->Save();
    pMeasure->SetFont(CBF_SIGNAL);

    // Snap drifted signal text offsets back to default before computing
    // constraints, so layout uses canonical text positions and the result
    // stays uniform.
    ResetDriftedSignalTextOffsets();

    // Gather pass: walk lifelines left-to-right; for each, walk its
    // ChildActivationShapes and the SignalShapes those activations sent
    // out (each signal is owned by its sender activation, so each is
    // visited exactly once). AddContribution stores a Contrib instance
    // per unordered (left, right) lifeline pair (Contrib's constructor
    // normalises order, FindContrib dedups across swapped lookups).
    // Defensive cleanup first in case a prior failed run left entries.
    Contrib::DeleteAllContrib();

    {
        CbPainter& painter = *pMeasure;
        LifeLineShapeIterator iSndLL(this);
        while (++iSndLL)
        {
            LifeLineShape::ChildActivationShapeIterator iAct(iSndLL);
            while (++iAct)
            {
                ChildActivationShape::ReceiverIterator iSig(iAct);
                while (++iSig)
                {
                    iSig->AddContribution(painter);
                }
            }
        }
    }

    // Placement pass: walk lifelines left-to-right. For each, the target
    // left edge is the max over the previous lifeline's right edge plus
    // MIN_RECT_GAP and every Contrib whose right lifeline is this one.
    // Constraints only look leftward, so one pass suffices.
    // Snapshot lifeline header + body rects before placement -- lifelines are
    // directly positioned, so capture old here; the diff after the loop is the
    // note-follow move-list handed to the base recompute.
    std::vector<std::pair<LifeLineShape*, CbRect> > oldHeaders;
    std::vector<std::pair<LifeLineShape*, CbRect> > oldBodies;
    {
        LifeLineShapeIterator iSnap(this);
        while (++iSnap)
        {
            oldHeaders.push_back(std::make_pair(iSnap.Get(), iSnap->GetRect()));
            oldBodies.push_back(std::make_pair(iSnap.Get(), iSnap->GetLifeLineRect()));
        }
    }

    LifeLineShape* pPrev = NULL;
    LifeLineShapeIterator iCurr(this);
    while (++iCurr)
    {
        LifeLineShape* pCurr = iCurr;
        if (pPrev)
        {
            int width = pCurr->GetRect().Width();
            int targetLeft = pPrev->GetBoundingRect().right + MIN_RECT_GAP;
            Contrib* pContrib = Contrib::FindRightContrib(pCurr);
            while (pContrib)
            {
                int candidate = pContrib->GetLeft()->GetStartPoint().x +
                                pContrib->GetNeeded() - width/2;
                if (candidate > targetLeft) targetLeft = candidate;

                pContrib = Contrib::FindNextRightContrib(pCurr, pContrib);
            }

            CbRect curRect = pCurr->GetRect();
            CbSize delta(targetLeft - curRect.left, 0);
            if (delta.cx != 0)
            {
                // SetRectNoSort SaveState()'s the LL; SetTemplateRect
                // does not (SaveState(0) — explicit skip). Order matters:
                // call SetRectNoSort first so the snapshot captures the
                // *original* _templateRect alongside the original _rect,
                // otherwise undo restores _rect but leaves _templateRect
                // at the post-move position and the class header box
                // floats away from its lifeline.
                pCurr->SetRectNoSort(curRect + delta);
                ClassLifeLineShape* pCLL = dynamic_cast<ClassLifeLineShape*>(pCurr);
                if (pCLL)
                {
                    pCLL->SetTemplateRect(pCLL->GetTemplateRect() + delta);
                }
            }
        }
        pPrev = pCurr;
    }

    Contrib::DeleteAllContrib();

    pMeasure->Restore(savedMeasure);

    // Diff the lifeline moves into the move-list, recompute, then carry the notes
    // (RecalculateDiagram records the activation moves; attached notes ride along).
    std::vector<std::pair<CbRect, CbRect> > moved;
    for (std::vector<std::pair<LifeLineShape*, CbRect> >::iterator it = oldHeaders.begin();
         it != oldHeaders.end(); ++it)
        if (it->first->GetRect() != it->second)
            moved.push_back(std::make_pair(it->second, it->first->GetRect()));
    for (std::vector<std::pair<LifeLineShape*, CbRect> >::iterator it = oldBodies.begin();
         it != oldBodies.end(); ++it)
        if (it->first->GetLifeLineRect() != it->second)
            moved.push_back(std::make_pair(it->second, it->first->GetLifeLineRect()));
    RecalculateDiagram(&moved);
    ResolveNoteFollows(moved);
    UpdateSequenceDiagramViews();
}//@CODE_34273


void SequenceDiagram::Update()
{//@CODE_30109
    if (GetAdded())
    {
        SetItemText(GetName());

        Gti::Update();
    }
}//@CODE_30109


void SequenceDiagram::UpdateSequenceDiagramViews()
{//@CODE_34249
    // Repaint the open SD canvases by invoking each SequenceDiagramViewModel's
    // registered refresh callback -- a posted, idempotent QWidget::update(), so
    // it coalesces and is safe even mid-bulk-op. (The NotifyDiagramChanged
    // lock-gate was removed 2026-06-17 -- cross-canvas/lock coalescing is the
    // chokepoint path's job now via NotifySdViews; this is just the leaf repaint,
    // reached either directly by a setter or through RepaintSdViews at flush.)
    SequenceDiagramViewModelIterator iViewModel(this);
    while (++iViewModel)
    {
        iViewModel->Refresh();
    }
}//@CODE_34249


/*@NOTE_35072
Set the value of member '_argumentNames' to 'argumentNames'.
*/
void SequenceDiagram::SetArgumentNames(bool argumentNames)
{//@CODE_35072
    if (_argumentNames != argumentNames)
    {
        _argumentNames = argumentNames;
        
        SequenceDiagramShapeIterator iSequenceDiagramShape(this);
        while (++iSequenceDiagramShape)
        {
            SignalShape* pSignalShape = dynamic_cast<SignalShape*>(
                iSequenceDiagramShape.Get());
            
            if (pSignalShape && pSignalShape->GetArgumentNames() != argumentNames)
            {
                pSignalShape->SaveState(1);
                pSignalShape->SetArgumentNames(argumentNames);
            }
        }
    }
}//@CODE_35072


/*@NOTE_34218
Set the value of member '_arguments' to 'arguments'.
*/
void SequenceDiagram::SetArguments(bool arguments)
{//@CODE_34218
    if (_arguments != arguments)
    {
        _arguments = arguments;
        
        SequenceDiagramShapeIterator iSequenceDiagramShape(this);
        while (++iSequenceDiagramShape)
        {
            SignalShape* pSignalShape = dynamic_cast<SignalShape*>(
                iSequenceDiagramShape.Get());
            
            if (pSignalShape && pSignalShape->GetArguments() != arguments)
            {
                pSignalShape->SaveState(1);
                pSignalShape->SetArguments(arguments);
            }
        }
    }
}//@CODE_34218


/*@NOTE_35167
Returns the value of member '_caption'.
*/
const CbString& SequenceDiagram::GetCaption()
{//@CODE_35167
    return _caption;
}//@CODE_35167


/*@NOTE_35168
Set the value of member '_caption' to 'rCaption'.
*/
void SequenceDiagram::SetCaption(const CbString& rCaption)
{//@CODE_35168
    _caption = rCaption;
    if (!rCaption.IsEmpty())
    {
        if (rCaption[rCaption.GetLength()-1] != '\n')
            _caption += NL;
    }
}//@CODE_35168


/*@NOTE_29676
Returns the value of member '_note'.
*/
const CbString& SequenceDiagram::GetNote()
{//@CODE_29676
    return _note;
}//@CODE_29676


/*@NOTE_29677
Set the value of member '_note' to 'rNote'.
*/
void SequenceDiagram::SetNote(const CbString& rNote)
{//@CODE_29677
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_29677


/*@NOTE_34412
Set the value of member '_scope' to 'scope'.
*/
void SequenceDiagram::SetScope(bool scope)
{//@CODE_34412
    if (_scope != scope)
    {
        _scope = scope;
        
        SequenceDiagramShapeIterator iSequenceDiagramShape(this);
        while (++iSequenceDiagramShape)
        {
            SignalShape* pSignalShape = dynamic_cast<SignalShape*>(
                iSequenceDiagramShape.Get());
            
            if (pSignalShape && pSignalShape->GetScope() != scope)
            {
                pSignalShape->SaveState(1);
                pSignalShape->SetScope(scope);
            }
        }
    }
}//@CODE_34412


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_29656
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SequenceDiagram::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
}


/*@NOTE_29611
Method which must be called first in a constructor.
*/
void SequenceDiagram::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
    INIT_SINGLE_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
    INIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    INIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
}


/*@NOTE_29613
Method which must be called first in a destructor.
*/
void SequenceDiagram::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
    EXIT_SINGLE_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
}


/*@NOTE_29657
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SequenceDiagram::RemoveReferences()
{
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
    REMOVE_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    REMOVE_SINGLE_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
    REMOVE_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
    Gti::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
}


/*@NOTE_29658
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SequenceDiagram::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagram* pSequenceDiagram = (SequenceDiagram*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_29660
Save the state of the current object relations to pDataModelDocObject.
*/
void SequenceDiagram::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    SequenceDiagram* pSequenceDiagram = (SequenceDiagram*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
}


/*@NOTE_29651
Serialize the members only to a CbObject object.
*/
void SequenceDiagram::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _height;
        archive << _multiPage;
        archive << _name;
        archive << _note;
        archive << _width;
        archive << _scale;
        archive << int(_numbering);
        archive << _arguments;
        archive << _scope;
        archive << _argumentNames;
        archive << _caption;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _height;
            archive >> _multiPage;
            archive >> _name;
            archive >> _note;
            archive >> _width;
            archive >> _scale;
            int numberingTmp;
            archive >> numberingTmp;
            _numbering = (SeqType)numberingTmp;
            archive >> _arguments;
            archive >> _scope;
            archive >> _argumentNames;
            archive >> _caption;
        }
    }
}


/*@NOTE_29650
Method which must be called first in a serialize constructor.
*/
void SequenceDiagram::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
    INIT_SINGLE_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
    INIT_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    INIT_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
}


/*@NOTE_29653
Serialize the relations to a CbObject object.
*/
void SequenceDiagram::SerializeRelations(CbArchive& archive,
                                         DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
        WRITE_SINGLE_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
        WRITE_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
            READ_SINGLE_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
            READ_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(SequenceDiagram)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
METHODS_ITERATOR_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
METHODS_SINGLE_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
METHODS_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
METHODS_ITERATOR_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
METHODS_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
METHODS_ITERATOR_NOFILTER_MULTI_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
