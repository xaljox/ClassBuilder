/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          NoteShape.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'NoteShape'
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
#include "CbPainter.h"
#include "qt/QtNoteShapeDialog.h"
#include "ClassBuilderDoc.h"
#include "MainFrm.h"
//@END_USER2


// Static members
bool NoteShape::_tracking = false;


NoteShape::NoteShape(ClassDiagram* pClassDiagram,
                     const CbPoint& point) //@INIT_3955
    : ClassDiagramShape(pClassDiagram, point, 
        pClassDiagram->GetDataModelDoc()->GetNoteShapePenColor(),
        pClassDiagram->GetDataModelDoc()->GetNoteShapeTextColor())
    , _note()
    , _fontHeight(28)
{//@CODE_3955
    ConstructorInclude();

    // Put in your own code
    CbRect size(0, 100, 400, 0);
    _rect += size;
    _rect.NormalizeRect();
    (void)new NoteShapePoint(this, _rect.BottomRight());
}//@CODE_3955


/*@NOTE_3945
Constructor needed for serialization, not meant to use for other purposes!
*/
NoteShape::NoteShape() //@INIT_3945
    : ClassDiagramShape()
    , _fontHeight(32)
{//@CODE_3945
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3945


/*@NOTE_3943
Destructor method.
*/
NoteShape::~NoteShape()
{//@CODE_3943
    DestructorInclude();

    // Put in your own code
}//@CODE_3943


void NoteShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35130
    NoteShape* pNoteShape =
        new NoteShape(pClassDiagram, CbPoint(0, 0));
    pNoteShape->CopyState(this);
    _ptrIndex = intptr_t(pNoteShape);

    NoteShapePointIterator iNoteShapePoint(this);
    while (--iNoteShapePoint)
    {
        if (iNoteShapePoint.IsLast())
        {
            pNoteShape->GetLastNoteShapePoint()->CopyState(iNoteShapePoint);
        }
        else
        {
            NoteShapePoint* pNoteShapePoint =
                new NoteShapePoint(pNoteShape, iNoteShapePoint->GetPoint());
            pNoteShape->MoveNoteShapePointFirst(pNoteShapePoint);
        }
    }
}//@CODE_35130


void NoteShape::Draw(CbPainter& painter,
                     ClassDiagramViewModel* pClassDiagramViewModel,
                     bool selected)
{//@CODE_40399
    int save = painter.Save();
    Shape::NoteDraw(painter, GetRect(), GetNote(), GetFontHeight(), selected, GetPenColor(), GetTextColor());

    painter.SetPen(PS_DASH, 1, GetPenColor());
    NoteShapePointIterator iNoteShapePoint(this);
    while (++iNoteShapePoint)
    {
        if (!iNoteShapePoint.IsLast())
        {
            painter.DrawLine(
                Shape::CrossPoint(GetRect(), iNoteShapePoint->GetPoint()),
                iNoteShapePoint->GetPoint());
        }
    }

    painter.Restore(save);

    if (painter.IsScreen() && selected)
        DrawSelectedRect(painter, CbPainter::GetSelectColor());
}//@CODE_40399


void NoteShape::DrawSelectedRect(CbPainter& painter, CbColorRef color)
{//@CODE_4987
    painter.DrawSelectionHandle(GetLeftSelectedPoint(),  color);
    painter.DrawSelectionHandle(GetRightSelectedPoint(), color);

    NoteShapePointIterator iSDNoteShapePoint(this);
    while (++iSDNoteShapePoint)
        painter.DrawSelectionHandle(iSDNoteShapePoint->GetPoint(), color);
}//@CODE_4987


CbPoint NoteShape::GetLeftSelectedPoint()
{//@CODE_4994
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.left;

    return value;
}//@CODE_4994


NoteShape* NoteShape::GetNoteShape()
{//@CODE_3970
    return this;
}//@CODE_3970


CbPoint NoteShape::GetRightSelectedPoint()
{//@CODE_4993
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.right;

    return value;
}//@CODE_4993


int NoteShape::IsAlignShape() const
{//@CODE_35327
    return 1;
}//@CODE_35327


/*@NOTE_5871
Move the noteshape points within 'rect' with 'offset'.
*/
void NoteShape::MoveNoteShapePoints(const CbRect& rect, const CbSize& offset)
{//@CODE_5871
    NoteShapePointIterator 
        iNoteShapePoint(this, &NoteShapePoint::IsNotJustMoved);
    while (++iNoteShapePoint)
    {
        if (rect.PtInRect(iNoteShapePoint->GetPoint()) &&
            !iNoteShapePoint.IsLast())
        {
            // Capture this point for undo. SetPoint only SaveStates the LAST
            // point, which this loop skips -- so without this the move (e.g. a
            // multi-select drag dragging note connector points along) could not
            // be undone.
            iNoteShapePoint->SaveState();
            iNoteShapePoint->SetPoint(
                iNoteShapePoint->GetPoint() + offset);
            if (iNoteShapePoint)
            {
                iNoteShapePoint->MarkAsJustMoved();
            }
        }
    }
}//@CODE_5871


int NoteShape::OnDelete(bool checkOnly)
{//@CODE_5007
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
}//@CODE_5007


int NoteShape::OnEditAttributes(bool checkOnly)
{//@CODE_3954
    return 0;
}//@CODE_3954


int NoteShape::OnOpen(bool checkOnly)
{//@CODE_35152
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
            // A note is a CD-only shape (no tree node), so kick the CD canvases only --
            // NotifyStructureChanged would needlessly rebuild the whole tree.
            GetDataModelDoc()->MarkLastUndo();
            GetDataModelDoc()->NotifyCdViews();
        }
        return 1;
    }

    return 0;
}//@CODE_35152


bool NoteShape::PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                             CbPoint pointLP)
{//@CODE_40856
    bool result = ClassDiagramShape::PointInShape(pClassDiagramViewModel, pointLP);

    NoteShapePointIterator iSDNoteShapePoint(this);
    while (result == false && ++iSDNoteShapePoint)
    {
        result = iSDNoteShapePoint->PointInShape(pointLP);
    }

    return result;
}//@CODE_40856


void NoteShape::RecalcHeight(CbPainter& painter)
{//@CODE_41367
    SetRect(Shape::NoteCalcRect(painter, GetRect(), GetNote(), GetFontHeight()));
}//@CODE_41367


/*@NOTE_5003
Set the value of member '_rect' to 'rRect'.
*/
void NoteShape::SetRect(const CbRect& rRect)
{//@CODE_5003
    
    if (_rect != rRect)
    {
        SaveState();
        _rect = rRect;

        GetLastNoteShapePoint()->SetPoint(rRect.BottomRight());
    }
}//@CODE_5003


/*@NOTE_5000
Returns the value of member '_note'.
*/
const CbString& NoteShape::GetNote()
{//@CODE_5000
    return _note;
}//@CODE_5000


/*@NOTE_5001
Set the value of member '_note' to 'rNote'.
*/
void NoteShape::SetNote(const CbString& rNote)
{//@CODE_5001
    if (_note != rNote)
    {
        _note = rNote;
    }
}//@CODE_5001


/*@NOTE_5006
Returns the value of member '_tracking'.
*/
bool NoteShape::GetTracking()
{//@CODE_5006
    return _tracking;
}//@CODE_5006


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5726
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void NoteShape::CleanupReferences()
{
    ClassDiagramShape::CleanupReferences();
}


/*@NOTE_3942
Method which must be called first in a constructor.
*/
void NoteShape::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_3944
Method which must be called first in a destructor.
*/
void NoteShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5727
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void NoteShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
    ClassDiagramShape::RemoveReferences();
}


/*@NOTE_5728
Bring the current object relations into the same state as pDataModelDocObject.
*/
void NoteShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5730
Save the state of the current object relations to pDataModelDocObject.
*/
void NoteShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape::SaveReferences(pDataModelDocObject);
}


/*@NOTE_3947
Serialize the members only to a CbObject object.
*/
void NoteShape::Serialize(CbArchive& archive)
{
    ClassDiagramShape::Serialize(archive);
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


/*@NOTE_3946
Method which must be called first in a serialize constructor.
*/
void NoteShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_3949
Serialize the relations to a CbObject object.
*/
void NoteShape::SerializeRelations(CbArchive& archive,
                                   DataModelDocObject* pointerArray[])
{
    ClassDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(NoteShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
METHODS_ITERATOR_MULTI_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
