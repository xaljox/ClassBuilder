/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SDNoteShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SDNoteShape'
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
#include "qt/QtNoteShapeDialog.h"
#include "ClassBuilderDoc.h"
#include "MainFrm.h"
#include "CbPainter.h"
//@END_USER2


// Static members
bool SDNoteShape::_tracking = false;


/*@NOTE_34688
Constructor method.
*/
SDNoteShape::SDNoteShape(SequenceDiagram* pSequenceDiagram,
                         const CbPoint& point) //@INIT_34688
    : SequenceDiagramShape(pSequenceDiagram, point, 
        pSequenceDiagram->GetDataModelDoc()->GetSDNoteShapePenColor(), 
        pSequenceDiagram->GetDataModelDoc()->GetSDNoteShapeTextColor())
    , _note()
    , _fontHeight(28)
{//@CODE_34688
    ConstructorInclude();

    // Put in your own code
    CbRect size(0, 100, 400, 0);
    _rect += size;
    _rect.NormalizeRect();
    (void)new SDNoteShapePoint(this, _rect.BottomRight());
}//@CODE_34688


/*@NOTE_34538
Constructor needed for serialization, not meant to use for other purposes!
*/
SDNoteShape::SDNoteShape() //@INIT_34538
    : SequenceDiagramShape()
{//@CODE_34538
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_34538


/*@NOTE_34536
Destructor method.
*/
SDNoteShape::~SDNoteShape()
{//@CODE_34536
    DestructorInclude();

    // Put in your own code
}//@CODE_34536


void SDNoteShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35122
    SDNoteShape* pSDNoteShape =
        new SDNoteShape(pSequenceDiagram, CbPoint(0, 0));
    pSDNoteShape->CopyState(this);
    _ptrIndex = intptr_t(pSDNoteShape);

    SDNoteShapePointIterator iSDNoteShapePoint(this);
    while (--iSDNoteShapePoint)
    {
        if (iSDNoteShapePoint.IsLast())
        {
            pSDNoteShape->GetLastSDNoteShapePoint()->CopyState(iSDNoteShapePoint);
        }
        else
        {
            SDNoteShapePoint* pSDNoteShapePoint =
                new SDNoteShapePoint(pSDNoteShape, iSDNoteShapePoint->GetPoint());
            pSDNoteShape->MoveSDNoteShapePointFirst(pSDNoteShapePoint);
        }
    }
}//@CODE_35122


void SDNoteShape::Draw(CbPainter& painter,
                       SequenceDiagramViewModel* pSequenceDiagramViewModel,
                       bool selected)
{//@CODE_40437
    int save = painter.Save();
    Shape::NoteDraw(painter, GetRect(), GetNote(), GetFontHeight(), selected, GetPenColor(), GetTextColor());

    painter.SetPen(PS_DASH, 1, GetPenColor());
    SDNoteShapePointIterator iSDNoteShapePoint(this);
    while (++iSDNoteShapePoint)
    {
        if (!iSDNoteShapePoint.IsLast())
        {
            painter.DrawLine(
                Shape::CrossPoint(GetRect(), iSDNoteShapePoint->GetPoint()),
                iSDNoteShapePoint->GetPoint());
        }
    }

    painter.Restore(save);

    if (painter.IsScreen() && selected)
        DrawSelectedRect(painter, CbPainter::GetSelectColor());
}//@CODE_40437


void SDNoteShape::DrawSelectedRect(CbPainter& painter, CbColorRef color)
{//@CODE_34741
    painter.DrawSelectionHandle(GetLeftSelectedPoint(),  color);
    painter.DrawSelectionHandle(GetRightSelectedPoint(), color);

    SDNoteShapePointIterator iSDNoteShapePoint(this);
    while (++iSDNoteShapePoint)
    {
        painter.DrawSelectionHandle(iSDNoteShapePoint->GetPoint(), color);
    }
}//@CODE_34741


CbPoint SDNoteShape::GetLeftSelectedPoint()
{//@CODE_34779
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.left;

    return value;
}//@CODE_34779


SDNoteShape* SDNoteShape::GetNoteShape()
{//@CODE_34744
    return this;
}//@CODE_34744


CbPoint SDNoteShape::GetRightSelectedPoint()
{//@CODE_34780
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.right;

    return value;
}//@CODE_34780


/*@NOTE_34745
Move the noteshape points within 'rect' with 'offset'.
*/
void SDNoteShape::MoveNoteShapePoints(const CbRect& rect, const CbSize& offset)
{//@CODE_34745
    SDNoteShapePointIterator 
        iSDNoteShapePoint(this, &SDNoteShapePoint::IsNotJustMoved);
    while (++iSDNoteShapePoint)
    {
        if (rect.PtInRect(iSDNoteShapePoint->GetPoint()) &&
            !iSDNoteShapePoint.IsLast())
        {
            // Snap to the 10-grid like the resize overload (Shape::Round @341).
            // Connector points are grid-aligned by design; a sub-grid follow
            // offset (the non-grid lifeline/signal x from the measured header)
            // then rounds back to the SAME grid point -> no move, no undo churn.
            CbPoint point = iSDNoteShapePoint->GetPoint() + offset;
            Shape::Round(point);
            iSDNoteShapePoint->SetPoint(point);
            if (iSDNoteShapePoint)
            {
                iSDNoteShapePoint->MarkAsJustMoved();
            }
        }
    }
}//@CODE_34745


/*@NOTE_34916
Move the noteshape points within 'oldRect' with 'newRect'.
*/
void SDNoteShape::MoveNoteShapePoints(const CbRect& oldRect,
                                      const CbRect& newRect)
{//@CODE_34916
    SDNoteShapePointIterator
        iSDNoteShapePoint(this, &SDNoteShapePoint::IsNotJustMoved);
    while (++iSDNoteShapePoint)
    {
        CbPoint point = iSDNoteShapePoint->GetPoint();
        if (oldRect.PtInRect(point) && !iSDNoteShapePoint.IsLast())
        {
            // JV follow rule: move the point by the rect's CENTER displacement
            // (old->new), then snap to the grid. Simpler + stabler than the old
            // proportional scale -- a pure resize leaves the center put, so the
            // point doesn't move. Best-effort by design (rare overlaps may miss).
            point += newRect.CenterPoint() - oldRect.CenterPoint();
            Shape::Round(point);
            iSDNoteShapePoint->SetPoint(point);
            if (iSDNoteShapePoint)
            {
                iSDNoteShapePoint->MarkAsJustMoved();
            }
        }
    }
}//@CODE_34916


int SDNoteShape::OnDelete(bool checkOnly)
{//@CODE_34748
    if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete note");
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }

    return 1;
}//@CODE_34748


int SDNoteShape::OnEditAttributes(bool checkOnly)
{//@CODE_34750
    return 0;
}//@CODE_34750


int SDNoteShape::OnOpen(bool checkOnly)
{//@CODE_34786
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowNoteShapeDialog(this, modelChanged, ownerHwnd))
    {
        if (modelChanged)
        {
            // Close the edit boundary like every other diagram edit: the dialog only
            // SaveState'd, so MarkLastUndo finalizes the undo step (and recomputes), then
            // refresh. Without it the undo step stays open and folds into the next edit.
            // An SD note is an SD-only shape (no tree node), so kick the SD canvases only --
            // NotifyStructureChanged would needlessly rebuild the whole tree.
            GetDataModelDoc()->MarkLastUndo();
            GetDataModelDoc()->NotifySdViews();
        }
        return 1;
    }

    return 0;
}//@CODE_34786


bool SDNoteShape::PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                               const CbPoint& pointLP)
{//@CODE_40883
    bool result = SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP);

    SDNoteShapePointIterator iSDNoteShapePoint(this);
    while (result == false && ++iSDNoteShapePoint)
    {
        result = iSDNoteShapePoint->PointInShape(pointLP);
    }

    return result;
}//@CODE_40883


void SDNoteShape::RecalcHeight(CbPainter& painter)
{//@CODE_41369
    SetRect(Shape::NoteCalcRect(painter, GetRect(), GetNote(), GetFontHeight()));
}//@CODE_41369


void SDNoteShape::ResolveNoteFollows(const std::vector<std::pair<CbRect,CbRect>>& moved)
{//@CODE_41347
    SDNoteShapePointIterator iSDNoteShapePoint(this);
    while (++iSDNoteShapePoint)
    {
        if (iSDNoteShapePoint.IsLast())            // box-end point stays put
            continue;

        CbPoint point = iSDNoteShapePoint->GetPoint();
        for (int i = 0; i < (int)moved.size(); i++)   // biggest old area first
        {
            if (moved[i].first.PtInRect(point))       // first hit wins
            {
                point += moved[i].second.CenterPoint()
                       - moved[i].first.CenterPoint();
                Shape::Round(point);
                iSDNoteShapePoint->SetPoint(point);
                break;
            }
        }
    }
}//@CODE_41347


/*@NOTE_34757
Set the value of member '_rect' to 'rRect'.
*/
void SDNoteShape::SetRect(const CbRect& rRect)
{//@CODE_34757
    
    if (_rect != rRect)
    {
        SaveState();
        _rect = rRect;

        GetLastSDNoteShapePoint()->SetPoint(rRect.BottomRight());
    }
}//@CODE_34757


/*@NOTE_34666
Returns the value of member '_note'.
*/
const CbString& SDNoteShape::GetNote()
{//@CODE_34666
    return _note;
}//@CODE_34666


/*@NOTE_34667
Set the value of member '_note' to 'rNote'.
*/
void SDNoteShape::SetNote(const CbString& rNote)
{//@CODE_34667
    if (_note != rNote)
    {
        _note = rNote;
    }
}//@CODE_34667


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_34545
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SDNoteShape::CleanupReferences()
{
    SequenceDiagramShape::CleanupReferences();
}


/*@NOTE_34535
Method which must be called first in a constructor.
*/
void SDNoteShape::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34537
Method which must be called first in a destructor.
*/
void SDNoteShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34546
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SDNoteShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
    SequenceDiagramShape::RemoveReferences();
}


/*@NOTE_34547
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SDNoteShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_34549
Save the state of the current object relations to pDataModelDocObject.
*/
void SDNoteShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape::SaveReferences(pDataModelDocObject);
}


/*@NOTE_34540
Serialize the members only to a CbObject object.
*/
void SDNoteShape::Serialize(CbArchive& archive)
{
    SequenceDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _note;
        archive << _fontHeight;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _note;
            archive >> _fontHeight;
        }
    }
}


/*@NOTE_34539
Method which must be called first in a serialize constructor.
*/
void SDNoteShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34542
Serialize the relations to a CbObject object.
*/
void SDNoteShape::SerializeRelations(CbArchive& archive,
                                     DataModelDocObject* pointerArray[])
{
    SequenceDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(SDNoteShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
METHODS_ITERATOR_MULTI_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
