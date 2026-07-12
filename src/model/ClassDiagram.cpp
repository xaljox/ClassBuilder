/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassDiagram.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassDiagram'
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
#include "MainFrm.h"
#include "qt/QtClassDiagramDialog.h"
#include "qt/QtClassDiagramView.h"   // Qt_ShowClassDiagramView (open-diagram)
//@END_USER2


// Static members


ClassDiagram::ClassDiagram(Gti* pGti) //@INIT_3760
    : Gti(pGti->GetDataModelDoc())
    , _name()
    , _note()
    , __notUsed_uml(true)
    , _width(2100)
    , _height(2970)
    , _multiPage(0)
    , _scale(80)
    , _caption()
    , _privateMembers(false)
    , _privateMethods(false)
    , _protectedMembers(false)
    , _protectedMethods(false)
    , _publicMembers(false)
    , _publicMethods(false)
    , _getSetMethods(false)
{//@CODE_3760
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
}//@CODE_3760


/*@NOTE_35102
Constructor method.
*/
ClassDiagram::ClassDiagram(Gti* pGti, ClassDiagram* pClassDiagram) //@INIT_35102
    : Gti(pGti->GetDataModelDoc())
    , _name()
    , _note()
    , __notUsed_uml(true)
    , _width(2100)
    , _height(2970)
    , _multiPage(0)
    , _scale(80)
    , _caption()
    , _privateMembers(false)
    , _privateMethods(false)
    , _protectedMembers(false)
    , _protectedMethods(false)
    , _publicMembers(false)
    , _publicMethods(false)
    , _getSetMethods(false)
{//@CODE_35102
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

    CopyState(pClassDiagram);
    
    SetPhase(pClassDiagram->GetPhase());
    SetAdded(false);

    ClassDiagramShapeIterator iClassShape(pClassDiagram, &ClassDiagramShape::IsClassShape);
    while (++iClassShape)
    {
        iClassShape->CopyShape(this);
    }
    
    ClassDiagramShapeIterator iConnectionShape(pClassDiagram, &ClassDiagramShape::IsConnectionShape);
    while (++iConnectionShape)
    {
        iConnectionShape->CopyShape(this);
    }
    
    ClassDiagramShapeIterator iNoteShape(pClassDiagram, &ClassDiagramShape::IsNoteShape);
    while (++iNoteShape)
    {
        iNoteShape->CopyShape(this);
    }
}//@CODE_35102


/*@NOTE_3354
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassDiagram::ClassDiagram() //@INIT_3354
    : Gti()
    , __notUsed_uml(true)
    , _width(2100)
    , _height(2970)
    , _multiPage(0)
    , _scale(80)
    , _caption()
    , _privateMembers(false)
    , _privateMethods(false)
    , _protectedMembers(false)
    , _protectedMethods(false)
    , _publicMembers(false)
    , _publicMethods(false)
    , _getSetMethods(false)
{//@CODE_3354
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3354


/*@NOTE_3352
Destructor method.
*/
ClassDiagram::~ClassDiagram()
{//@CODE_3352
    DestructorInclude();

    // Put in your own code
}//@CODE_3352


void ClassDiagram::Add()
{//@CODE_3902
    if (!GetAdded())
    {
        SetItemText(GetName());
        SetIcon(ICON_CLASSDIAGRAM);

        Gti::Add();
    }
}//@CODE_3902


/*@NOTE_4181
Put associated  InheritShape object in the appropiate drawings.
*/
void ClassDiagram::AddInherit(Inherit* pInherit)
{//@CODE_4181
    DataModelDoc::ClassDiagramIterator iClassDiagram(pInherit->GetDataModelDoc());
    while (++iClassDiagram)
    {
        ClassShape* pBaseClassShape = pInherit->GetBaseClass()->FindClassShape(iClassDiagram);
        while (pBaseClassShape)
        {
            ClassShape* pExternClassShape = pInherit->GetExternClass()->FindClassShape(iClassDiagram);
            while (pExternClassShape)
            {
                (void)new InheritShape(iClassDiagram, pInherit, pBaseClassShape, pExternClassShape);

                pExternClassShape = pInherit->GetExternClass()->FindClassShape(iClassDiagram, pExternClassShape);
            }

            pBaseClassShape = pInherit->GetBaseClass()->FindClassShape(iClassDiagram, pBaseClassShape);
        }
    }
}//@CODE_4181


/*@NOTE_4182
Put associated RelationShape Object in the appropiate drawings.
*/
void ClassDiagram::AddRelation(Relation* pRelation)
{//@CODE_4182
    DataModelDoc::ClassDiagramIterator iClassDiagram(pRelation->GetDataModelDoc());
    while (++iClassDiagram)
    {
        ClassShape* pFromClassShape = pRelation->GetFromClass()->FindClassShape(iClassDiagram);
        while (pFromClassShape)
        {
            ClassShape* pToClassShape = pRelation->GetToClass()->FindClassShape(iClassDiagram);
            while (pToClassShape)
            {
                (void)new RelationShape(iClassDiagram, pRelation, pFromClassShape, pToClassShape);

                pToClassShape = pRelation->GetToClass()->FindClassShape(iClassDiagram, pToClassShape);
            }

            pFromClassShape = pRelation->GetFromClass()->FindClassShape(iClassDiagram, pFromClassShape);
        }
    }
}//@CODE_4182


bool ClassDiagram::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_4130
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
}//@CODE_4130


void ClassDiagram::Draw(CbPainter& painter,
                        ClassDiagramViewModel* pClassDiagramViewModel)
{//@CODE_40340
    // Paint is read-only -- it never mutates the model -- so there is no draw guard.
    // The _isDrawing scaffold retired once the note-height recompute moved out of Draw
    // to the edit boundary (Shape::NoteCalcRect via each note's RecalcHeight).
    ClassDiagramShapeIterator iClassDiagramShape(this, &ClassDiagramShape::DrawDirect);
    while (++iClassDiagramShape)
    {
        if (!iClassDiagramShape->IsSelectedIn(pClassDiagramViewModel))
            iClassDiagramShape->Draw(painter, pClassDiagramViewModel, false);
    }

    ClassDiagramViewModel::SelectedIterator iSelected(pClassDiagramViewModel);
    while (++iSelected)
    {
        iSelected->GetClassDiagramShape()->Draw(painter, pClassDiagramViewModel, true);
    }
}//@CODE_40340


void ClassDiagram::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_4109
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
}//@CODE_4109


bool ClassDiagram::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_4112
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
}//@CODE_4112


CbPoint ClassDiagram::FindFreeShapePlacement()
{//@CODE_40907
    bool stop = false;
    int x = 0;
    int y = 200;
    while (!stop && y < GetHeight())
    {
        x = 100;
        while (!stop && x < GetWidth())
        {
            stop = true;
            CbRect rect(x, -(y + 250), x + 250, -y);
            ClassDiagram::ClassDiagramShapeIterator iShape(
                this, &ClassDiagramShape::IsClassShape);
            while (++iShape)
            {
                CbRect tmpRect;
                if (tmpRect.IntersectRect(rect, iShape->GetRect()))
                {
                    stop = false;
                    break;
                }
            }
            if (!stop)
                x += 500;
        }
        if (!stop)
            y += 600;
    }
    if (!stop)
        x = y = 0;

    return CbPoint(x, -y);
}//@CODE_40907


CbRect ClassDiagram::GetBoundingRect()
{//@CODE_35257
    // Union only what Draw() paints: top-level shapes (member/method rows are
    // painted by their ClassShape inside its rect, but sit in this list too
    // with sub-rects that can go stale) and no hidden connections. Anything
    // never drawn must not stretch the tight SVG-export crop.
    CbRect rect(0, 0, 0, 0);

    ClassDiagramShapeIterator iClassDiagramShape(this, &ClassDiagramShape::DrawDirect);
    while (++iClassDiagramShape)
    {
        ConnectionShape* pConnectionShape = iClassDiagramShape->GetConnectionShape();
        if (pConnectionShape)
        {
            if (pConnectionShape->GetHidden())
                continue;
            // A connection paints its segments, not its stored _rect -- the
            // rect can be stale without ever showing. Sync it first.
            pConnectionShape->RecalculateRect();
        }

        rect *= iClassDiagramShape->GetBoundingRect();
    }

    return rect;
}//@CODE_35257


ClassDiagramShape* ClassDiagram::GetHitShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                             CbPoint pointLP, bool nested)
{//@CODE_40863
    ClassDiagramShape* pClassDiagramShape = 0;

    ClassDiagramShapeIterator iClassDiagramShape(this, &ClassDiagramShape::DrawDirect);
    while (!pClassDiagramShape && --iClassDiagramShape)
    {
        if (iClassDiagramShape->PointInShape(pClassDiagramViewModel, pointLP))
        {
            pClassDiagramShape = iClassDiagramShape->GetHitShape(pClassDiagramViewModel, pointLP, nested);
        }
    }

    return pClassDiagramShape;
}//@CODE_40863


/*@NOTE_5868
Move the noteshape points within 'rect' with 'offset'.
*/
void ClassDiagram::MoveNoteShapePoints(const CbRect& rect, const CbSize& offset)
{//@CODE_5868
    ClassDiagramShapeIterator iClassDiagramShape(this);
    while (++iClassDiagramShape)
    {
        NoteShape* pNoteShape = dynamic_cast<NoteShape*>(iClassDiagramShape.Get());
        if (pNoteShape)
        {
            pNoteShape->MoveNoteShapePoints(rect, offset);
        }
    }
}//@CODE_5868


void ClassDiagram::MoveSelectedShapes(ClassDiagramViewModel* pClassDiagramViewModel,
                                      const CbSize& offset)
{//@CODE_40612
    CbRect rect;
    bool any = false;

    ClassDiagramViewModel::SelectedIterator iSelected(pClassDiagramViewModel);
    while (++iSelected)
    {
        ClassDiagramShape* pShape = iSelected->GetClassDiagramShape();

        // Accumulate the pre-move bounding rect (the MFC code used the drag
        // tracking rectangle; here it comes from the selected shapes).
        CbRect shapeRect = pShape->GetRect();
        if (!any)
        {
            rect = shapeRect;
            any = true;
        }
        else
        {
            if (shapeRect.left < rect.left)     rect.left = shapeRect.left;
            if (shapeRect.top < rect.top)       rect.top = shapeRect.top;
            if (shapeRect.right > rect.right)   rect.right = shapeRect.right;
            if (shapeRect.bottom > rect.bottom) rect.bottom = shapeRect.bottom;
        }

        NoteShape* pNoteShape = dynamic_cast<NoteShape*>(pShape);
        if (pNoteShape)
        {
            pNoteShape->SetRect(pNoteShape->GetRect() + offset);
        }

        ClassShape* pClassShape = dynamic_cast<ClassShape*>(pShape);
        if (pClassShape)
        {
            pClassShape->SaveState();
            pClassShape->Shape::SetRect(pClassShape->GetRect() + offset);

            ClassShape::FromConnectionShapeIterator iFromConnectionShape(pClassShape);
            while (++iFromConnectionShape)
            {
                if (iFromConnectionShape->GetToClassShape()->IsSelectedIn(pClassDiagramViewModel))
                {
                    iFromConnectionShape->SetStartPoint(iFromConnectionShape->GetStartPoint() + offset);
                }
                else
                {
                    if (iFromConnectionShape->GetInitial())
                    {
                        ClassShape* pToClassShape = iFromConnectionShape->GetToClassShape();
                        iFromConnectionShape->SetStartPoint(pClassShape->ConnectionPoint(pToClassShape));
                        iFromConnectionShape->SetEndPoint(pToClassShape->ConnectionPoint(pClassShape));
                        iFromConnectionShape->MakeNewRouting();
                    }
                    else
                    {
                        iFromConnectionShape->UpdateStartPoint(iFromConnectionShape->GetStartPoint() + offset);
                    }
                }
            }

            ClassShape::ToConnectionShapeIterator iToConnectionShape(pClassShape);
            while (++iToConnectionShape)
            {
                if (iToConnectionShape->GetFromClassShape()->IsSelectedIn(pClassDiagramViewModel))
                {
                    iToConnectionShape->SetEndPoint(iToConnectionShape->GetEndPoint() + offset);
                }
                else
                {
                    if (iToConnectionShape->GetInitial())
                    {
                        ClassShape* pFromClassShape = iToConnectionShape->GetFromClassShape();
                        iToConnectionShape->SetStartPoint(pFromClassShape->ConnectionPoint(pClassShape));
                        iToConnectionShape->SetEndPoint(pClassShape->ConnectionPoint(pFromClassShape));
                        iToConnectionShape->MakeNewRouting();
                    }
                    else
                    {
                        iToConnectionShape->UpdateEndPoint(iToConnectionShape->GetEndPoint() + offset);
                    }
                }
            }
        }
    }

    if (any)
    {
        rect.InflateRect(10, 10, 11, 11);
        MoveNoteShapePoints(rect, offset);
    }
}//@CODE_40612


int ClassDiagram::OnDelete(bool checkOnly)
{//@CODE_3896
    if (!checkOnly)
    {
        if (GetClassDiagramViewModelCount() == 0)
        {
            CbString str;
            str.Format("Are you sure you want to delete class diagram '%s'", 
                GetName().c_str());
            if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_OKCANCEL) == CBMB_IDOK)
            {
                Delete();
            }
        }
        else
        {
            CbString str;
            str.Format("Can not delete class diagram '%s' views on it are still open, close them first.", 
                GetName().c_str());
            CbMessageBox(str, CBMB_ICONEXCLAMATION);
        }
    }
    
    return (GetClassDiagramViewModelCount() ? 0: 1);
}//@CODE_3896


int ClassDiagram::OnEditAttributes(bool checkOnly)
{//@CODE_3813
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    bool sizeChanged = false;
    if (Qt_ShowClassDiagramDialog(this, modelChanged, sizeChanged, ownerHwnd))
    {
        // Coalesce the size-repaint and Update()'s rebuild into one refresh
        // (CbViewLock also shows the wait cursor).
        CbViewLock lock(GetDataModelDoc());

        // The Qt dialog applied the page width/height to the model; repaint
        // the open Qt canvases so they pick up the new page size.
        if (sizeChanged)
            UpdateClassDiagramViews();

        if (modelChanged)
            Update();

        return 1;
    }

    return 0;
}//@CODE_3813


int ClassDiagram::OnOpen(bool checkOnly)
{//@CODE_3818
    if (!checkOnly)
    {
        // Open the diagram as a Qt canvas (the MFC MDI view was removed).
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowClassDiagramView(this, ownerHwnd);
    }

    return 1;
}//@CODE_3818


/*@NOTE_22924
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void ClassDiagram::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_22924
    Gti::OnUndoRedoChanged(pOldState);

    ClassDiagram* pClassDiagram = (ClassDiagram*)pOldState;
    
    if (pClassDiagram &&
        GetWidth() != pClassDiagram->GetWidth() && 
        GetHeight() != pClassDiagram->GetHeight())
    {
        UpdateClassDiagramViews();
    }
    
}//@CODE_22924


/*@NOTE_41185
Recompute the geometry of this diagram's shapes -- box sizes, and the
connection reroutes a resize triggers. Changes the diagram model; it does NOT
draw it. Called at the edit boundary (DataModelDoc::MarkLastUndo), never from
ClassShape::Draw -- a paint must mutate nothing.
*/
void ClassDiagram::RecalculateDiagram()
{//@CODE_41185
    ClassDiagramShapeIterator iClassDiagramShape(this);
    while (++iClassDiagramShape)
    {
        ClassShape* pClassShape = dynamic_cast<ClassShape*>(iClassDiagramShape.Get());
        if (pClassShape)
            pClassShape->RecalculateRect();
    }
}//@CODE_41185


/*@NOTE_4183
Remove all associated InheritShape objects from drawings.
*/
void ClassDiagram::RemoveInherit(Inherit* pInherit)
{//@CODE_4183
    Inherit::InheritShapeIterator iBaseInheritShape(pInherit);
    while (++iBaseInheritShape)
    {
        iBaseInheritShape->Delete();
    }
}//@CODE_4183


/*@NOTE_4184
Remove all associated RelationShape objects from drawings.
*/
void ClassDiagram::RemoveRelation(Relation* pRelation)
{//@CODE_4184
    Relation::RelationShapeIterator iFromRelationShape(pRelation);
    while (++iFromRelationShape)
    {
        iFromRelationShape->Delete();
    }
}//@CODE_4184


void ClassDiagram::Update()
{//@CODE_3814
    if (GetAdded())
    {
        SetItemText(GetName());

        Gti::Update();
    }
}//@CODE_3814


void ClassDiagram::UpdateClassDiagramViews()
{//@CODE_40333
    // Repaint the open CD canvases by invoking each ClassDiagramViewModel's
    // registered refresh callback -- a posted, idempotent QWidget::update(), so
    // it coalesces and is safe even mid-bulk-op. (The NotifyDiagramChanged
    // lock-gate was removed 2026-06-17 -- cross-canvas/lock coalescing is the
    // chokepoint path's job now via NotifyCdViews; this is just the leaf repaint,
    // reached either directly by a setter or through RepaintCdViews at flush.)
    ClassDiagramViewModelIterator iViewModel(this);
    while (++iViewModel)
    {
        iViewModel->Refresh();
    }
}//@CODE_40333


/*@NOTE_35163
Returns the value of member '_caption'.
*/
const CbString& ClassDiagram::GetCaption()
{//@CODE_35163
    return _caption;
}//@CODE_35163


/*@NOTE_35164
Set the value of member '_caption' to 'rCaption'.
*/
void ClassDiagram::SetCaption(const CbString& rCaption)
{//@CODE_35164
    _caption = rCaption;
    if (!rCaption.IsEmpty())
    {
        if (rCaption[rCaption.GetLength()-1] != '\n')
            _caption += NL;
    }
}//@CODE_35164


/*@NOTE_4782
Returns the value of member '_name'.
*/
const CbString& ClassDiagram::GetName()
{//@CODE_4782
    return _name;
}//@CODE_4782


/*@NOTE_4783
Set the value of member '_name' to 'rName'.
*/
void ClassDiagram::SetName(const CbString& rName)
{//@CODE_4783
    _name = rName;
}//@CODE_4783


/*@NOTE_3907
Returns the value of member '_note'.
*/
const CbString& ClassDiagram::GetNote()
{//@CODE_3907
    return _note;
}//@CODE_3907


/*@NOTE_3908
Set the value of member '_note' to 'rNote'.
*/
void ClassDiagram::SetNote(const CbString& rNote)
{//@CODE_3908
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_3908


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5678
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassDiagram::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
}


/*@NOTE_3351
Method which must be called first in a constructor.
*/
void ClassDiagram::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
    INIT_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    INIT_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
}


/*@NOTE_3353
Method which must be called first in a destructor.
*/
void ClassDiagram::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
    EXIT_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    EXIT_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
}


/*@NOTE_5679
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassDiagram::RemoveReferences()
{
    EXIT_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
    REMOVE_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    REMOVE_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
    Gti::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
}


/*@NOTE_5680
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassDiagram::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagram* pClassDiagram = (ClassDiagram*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5682
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassDiagram::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    ClassDiagram* pClassDiagram = (ClassDiagram*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
}


/*@NOTE_3356
Serialize the members only to a CbObject object.
*/
void ClassDiagram::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
        archive << _note;
        archive << __notUsed_uml;
        archive << _width;
        archive << _height;
        archive << _multiPage;
        archive << _scale;
        archive << _caption;
        archive << _privateMembers;
        archive << _privateMethods;
        archive << _protectedMembers;
        archive << _protectedMethods;
        archive << _publicMembers;
        archive << _publicMethods;
        archive << _getSetMethods;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
            archive >> _note;
            archive >> __notUsed_uml;
            archive >> _width;
            archive >> _height;
            archive >> _multiPage;
            archive >> _scale;
            archive >> _caption;
            archive >> _privateMembers;
            archive >> _privateMethods;
            archive >> _protectedMembers;
            archive >> _protectedMethods;
            archive >> _publicMembers;
            archive >> _publicMethods;
            archive >> _getSetMethods;
        }
    }
}


/*@NOTE_3355
Method which must be called first in a serialize constructor.
*/
void ClassDiagram::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
    INIT_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    INIT_MULTI_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
}


/*@NOTE_3358
Serialize the relations to a CbObject object.
*/
void ClassDiagram::SerializeRelations(CbArchive& archive,
                                      DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
        WRITE_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
            READ_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ClassDiagram)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
METHODS_ITERATOR_MULTI_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
METHODS_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
METHODS_ITERATOR_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
METHODS_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
METHODS_ITERATOR_NOFILTER_MULTI_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
