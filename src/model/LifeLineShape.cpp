/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          LifeLineShape.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'LifeLineShape'
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
#include "MainFrm.h"
#include "qt/QtLifeLineDialog.h"
//@END_USER2


// Static members
bool LifeLineShape::_tracking = false;


/*@NOTE_32481
Constructor method.
*/
LifeLineShape::LifeLineShape(SequenceDiagram* pSequenceDiagram,
                             const CbPoint& point) //@INIT_32481
    : SequenceDiagramShape(pSequenceDiagram, point, 
        pSequenceDiagram->GetDataModelDoc()->GetLifeLinePenColor(), 
        pSequenceDiagram->GetDataModelDoc()->GetLifeLineTextColor())
    , _name("")
    , _note("")
    , _showActivations(true)
    , _order(0)
    , _orderWeight(0)
{//@CODE_32481
    ConstructorInclude(pSequenceDiagram);

    // Put in your own code
}//@CODE_32481


/*@NOTE_30619
Constructor needed for serialization, not meant to use for other purposes!
*/
LifeLineShape::LifeLineShape() //@INIT_30619
    : SequenceDiagramShape()
    , _name("")
    , _note("")
    , _showActivations(true)
    , _order(0)
    , _orderWeight(0)
{//@CODE_30619
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_30619


/*@NOTE_30617
Destructor method.
*/
LifeLineShape::~LifeLineShape()
{//@CODE_30617
    DestructorInclude();

    // Put in your own code
}//@CODE_30617


int LifeLineShape::Compare(LifeLineShape* pLifeLineShape1,
                           LifeLineShape* pLifeLineShape2)
{//@CODE_34109
    int result = pLifeLineShape1->GetLeftActivation() - pLifeLineShape2->GetLeftActivation();

    // The sort using this comparator runs only inside the paint recompute
    // (SortLifeLineShape(Compare) @ SetRect:291); the lifeline order is derived
    // from GetLeftActivation (x-position) and re-derives on undo, so SaveState-ing
    // each swap here only recorded undo while deriving -- removed. (CompareOrderWeight
    // below keeps its per-swap save: that sort is the OptimizePlacement edit.)
    return result;
}//@CODE_34109


int LifeLineShape::CompareOrderWeight(LifeLineShape* pLifeLineShape1,
                                      LifeLineShape* pLifeLineShape2)
{//@CODE_38581
    // Same undo discipline as Compare: snapshot the right operand and
    // emit a level-2 sub-batch boundary on each swap-needed comparison
    // (diff > 0). METHOD_MULTI_SORT is a bubble sort so every diff > 0
    // corresponds to exactly one adjacent-pair flip; the resulting
    // sequence of single-pair sub-batches is reversible by
    // RESTORE_MULTI_PASSIVE. Don't pair with MergeSortLifeLineShape.
    float diff = pLifeLineShape1->GetOrderWeight() - pLifeLineShape2->GetOrderWeight();

    if (diff > 0.0f)
    {
        pLifeLineShape2->SaveState();
        pLifeLineShape2->GetDataModelDoc()->MarkLastUndo(2);
        return 1;
    }
    if (diff < 0.0f)
        return -1;
    return 0;
}//@CODE_38581


void LifeLineShape::Draw(CbPainter& painter,
                         SequenceDiagramViewModel* pSequenceDiagramViewModel,
                         bool selected)
{//@CODE_40413
}//@CODE_40413


BaseClass* LifeLineShape::GetBaseClass() const
{//@CODE_35347
    return 0;
}//@CODE_35347


ChildActivationShape* LifeLineShape::GetBottomChildActivationShape()
{//@CODE_35275
    ChildActivationShape* pBottomChildActivationShape = 
        GetLastChildActivationShape();

    ChildActivationShapeIterator iChildActivationShape(this);
    while (--iChildActivationShape)
    {
        if (iChildActivationShape->GetRect().top < 
            pBottomChildActivationShape->GetRect().top)
        {
            pBottomChildActivationShape = iChildActivationShape;
        }
    }
        
    return pBottomChildActivationShape;
}//@CODE_35275


Class* LifeLineShape::GetClass() const
{//@CODE_35349
    return dynamic_cast<Class*>(GetBaseClass());
}//@CODE_35349


ChildActivationShape* LifeLineShape::GetDestructionChildActivationShape()
{//@CODE_35277
    ChildActivationShape* pDestructionChildActivationShape = 
        GetLastChildActivationShape();

    ChildActivationShapeIterator iChildActivationShape(this);
    while (--iChildActivationShape)
    {
        if (iChildActivationShape->GetRect().top < 
            pDestructionChildActivationShape->GetRect().top &&
            iChildActivationShape->GetDestruction())
        {
            pDestructionChildActivationShape = iChildActivationShape;
        }
    }

    if (pDestructionChildActivationShape && 
        !pDestructionChildActivationShape->GetDestruction())
    {
        pDestructionChildActivationShape = 0;
    }
        
    return pDestructionChildActivationShape;
}//@CODE_35277


CbPoint LifeLineShape::GetEndPoint()
{//@CODE_34343
    return GetStartPoint() - CbSize(0, GetLifeLineLength());
}//@CODE_34343


int LifeLineShape::GetLeftActivation()
{//@CODE_33769
    return GetStartPoint().x - SequenceDiagram::GetActivationWidth()/2;
}//@CODE_33769


LifeLineShape* LifeLineShape::GetLifeLine()
{//@CODE_33552
    return this;
}//@CODE_33552


int LifeLineShape::GetLifeLineLength()
{//@CODE_34134
    return GetSequenceDiagram()->GetLifeLineHeight();
}//@CODE_34134


CbRect LifeLineShape::GetLifeLineRect()
{//@CODE_34118
    CbRect lifeLineRect;
    lifeLineRect.left = GetLeftActivation();
    lifeLineRect.right = lifeLineRect.left + SequenceDiagram::GetActivationWidth();
    lifeLineRect.bottom = _rect.top;
    lifeLineRect.top = _rect.top - GetLifeLineLength();
    
    return lifeLineRect;
}//@CODE_34118


CbPoint LifeLineShape::GetStartPoint()
{//@CODE_34344
    CbPoint startPoint = _rect.CenterPoint();
    Shape::Round(startPoint);
    
    startPoint.y = _rect.top;

    return startPoint;
}//@CODE_34344


int LifeLineShape::OnDelete(bool checkOnly)
{//@CODE_33770
    if (!checkOnly)
    {
        Delete();
    }

    return 1;
}//@CODE_33770


int LifeLineShape::OnOpen(bool checkOnly)
{//@CODE_33888
    if (!checkOnly)
    {
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowLifeLineDialog(this, ownerHwnd);
    }

    return 1;
}//@CODE_33888


/*@NOTE_33772
Set this lifeline's _rect for a SINGLE, derived re-position: re-sorts the lifelines
(by x-position) and repaints, but does NOT SaveState -- the move's undo is already
recorded at the edit boundary (SequenceDiagram::RecalculateAfterEdit). The inverse of
SetRectNoSort (which SaveState's but leaves sort + repaint to the caller). For a bulk
op that moves many lifelines, use SetRectNoSort instead.
*/
void LifeLineShape::SetRect(const CbRect& rRect)
{//@CODE_33772
    CbRect oldRect = _rect;

    if (oldRect != rRect)
    {
        // Note-follow is no longer queued here: the edit chokepoint
        // (SequenceDiagram::RecalculateAfterEdit) records the lifeline's move and
        // carries the attached note points. SetRect just stores the rect.
        _rect = rRect;

        GetSequenceDiagram()->UpdateSequenceDiagramViews();
        
        GetSequenceDiagram()->SortLifeLineShape(LifeLineShape::Compare);
    }
}//@CODE_33772


/*@NOTE_38302
Set this lifeline's _rect as part of a BULK layout op: SaveState's the lifeline (so the
move is undoable) but does NOT sort or repaint -- the caller does ONE SortLifeLineShape
+ repaint after moving all the lifelines (e.g. SpaceLifeLines), avoiding O(n^2) per-call
sorts and repaint thrash. The inverse of SetRect (which sorts + repaints but does NOT
SaveState).
*/
void LifeLineShape::SetRectNoSort(const CbRect& rRect)
{//@CODE_38302
    CbRect oldRect = _rect;
    
    if (oldRect != rRect)
    {
        SaveState();

        // Note-follow is no longer queued here (see RecalculateAfterEdit).
        _rect = rRect;
    }
}//@CODE_38302


/*@NOTE_33532
Returns the value of member '_note'.
*/
const CbString& LifeLineShape::GetNote()
{//@CODE_33532
    return _note;
}//@CODE_33532


/*@NOTE_33533
Set the value of member '_note' to 'rNote'.
*/
void LifeLineShape::SetNote(const CbString& rNote)
{//@CODE_33533
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_33533


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_30626
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void LifeLineShape::CleanupReferences()
{
    SequenceDiagramShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
}


/*@NOTE_30616
Method which must be called first in a constructor.
*/
void LifeLineShape::ConstructorInclude(SequenceDiagram* pSequenceDiagram)
{
    INIT_MULTI_OWNED_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
}


/*@NOTE_30618
Method which must be called first in a destructor.
*/
void LifeLineShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    EXIT_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
}


/*@NOTE_30627
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void LifeLineShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    SequenceDiagramShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
}


/*@NOTE_30628
Bring the current object relations into the same state as pDataModelDocObject.
*/
void LifeLineShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    LifeLineShape* pLifeLineShape = (LifeLineShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    SequenceDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_30630
Save the state of the current object relations to pDataModelDocObject.
*/
void LifeLineShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape::SaveReferences(pDataModelDocObject);
    LifeLineShape* pLifeLineShape = (LifeLineShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
}


/*@NOTE_30621
Serialize the members only to a CbObject object.
*/
void LifeLineShape::Serialize(CbArchive& archive)
{
    SequenceDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
        archive << _note;
        archive << _showActivations;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
            archive >> _note;
            archive >> _showActivations;
        }
    }
}


/*@NOTE_30620
Method which must be called first in a serialize constructor.
*/
void LifeLineShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
}


/*@NOTE_30623
Serialize the relations to a CbObject object.
*/
void LifeLineShape::SerializeRelations(CbArchive& archive,
                                       DataModelDocObject* pointerArray[])
{
    SequenceDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
METHODS_ITERATOR_MULTI_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
METHODS_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
