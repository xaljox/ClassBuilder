/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ChildActivationShape.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ChildActivationShape'
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
#include "MainFrm.h"
#include "ClassBuilderDoc.h"
#include "CbPainter.h"
#include "qt/QtUserCodeDialog.h"
//@END_USER2


// Static members


/*@NOTE_33135
Constructor method.
*/
ChildActivationShape::ChildActivationShape(LifeLineShape* pLifeLineShape) //@INIT_33135
    : ParentActivationShape(pLifeLineShape->GetSequenceDiagram(), 
        pLifeLineShape->GetDataModelDoc()->GetActivationPenColor())
    , _creation(false)
    , _destruction(false)
    , _sequenceNumber(0)
    , _sequenceSubNumber(0)
    , _offset(0)
    , _activationNoMethodPenColor(pLifeLineShape->GetDataModelDoc()->GetActivationNoMethodPenColor())
    , _activationUser3PenColor(pLifeLineShape->GetDataModelDoc()->GetActivationUser3PenColor())
    , _activationInitialPenColor(pLifeLineShape->GetDataModelDoc()->GetActivationInitialPenColor())
    , _merge(false)
{//@CODE_33135
    ConstructorInclude(pLifeLineShape, pLifeLineShape->GetSequenceDiagram()->GetRootActivationShape());

    // Put in your own code
}//@CODE_33135


/*@NOTE_33139
Constructor method.
*/
ChildActivationShape::ChildActivationShape(LifeLineShape* pLifeLineShape,
                                           ChildActivationShape* pChildActivationShape) //@INIT_33139
    : ParentActivationShape(pLifeLineShape->GetSequenceDiagram(), 
        pLifeLineShape->GetDataModelDoc()->GetActivationPenColor())
    , _creation(false)
    , _destruction(false)
    , _sequenceNumber(0)
    , _sequenceSubNumber(0)
    , _offset(0)
    , _activationNoMethodPenColor(pLifeLineShape->GetDataModelDoc()->GetActivationNoMethodPenColor())
    , _activationUser3PenColor(pLifeLineShape->GetDataModelDoc()->GetActivationUser3PenColor())
    , _activationInitialPenColor(pLifeLineShape->GetDataModelDoc()->GetActivationInitialPenColor())
    , _merge(false)
{//@CODE_33139
    ConstructorInclude(pLifeLineShape, pChildActivationShape);

    // Put in your own code
    (void)new SignalShape(this, pChildActivationShape);
}//@CODE_33139


/*@NOTE_31792
Constructor needed for serialization, not meant to use for other purposes!
*/
ChildActivationShape::ChildActivationShape() //@INIT_31792
    : ParentActivationShape()
    , _creation(false)
    , _destruction(false)
    , _sequenceNumber(0)
    , _sequenceSubNumber(0)
    , _offset(0)
    , _activationNoMethodPenColor(Cb_RGB(128, 128, 128))
    , _activationUser3PenColor(Cb_RGB(0, 64, 128))
    , _merge(false)
    , _activationInitialPenColor(Cb_RGB(128, 0, 0))
{//@CODE_31792
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_31792


/*@NOTE_31790
Destructor method.
*/
ChildActivationShape::~ChildActivationShape()
{//@CODE_31790
    DestructorInclude();

    // Put in your own code
}//@CODE_31790


bool ChildActivationShape::CanMerge()
{//@CODE_35394
    if (GetSender() && // GetSender()->GetAsync() && 
        !IsRecursiveActivation() &&
        GetLifeLineShape()->GetPrevChildActivationShape(this))
    {
        return true;
    }

    return false;
}//@CODE_35394


int ChildActivationShape::Compare(ChildActivationShape* pChildActivationShape1,
                                  ChildActivationShape* pChildActivationShape2)
{//@CODE_34135
    int result = pChildActivationShape2->GetRect().bottom - pChildActivationShape1->GetRect().bottom;

    // The only caller is the sort inside the paint recompute (SetRect:593); the
    // activation order is derived from y-position and re-derives on undo, so
    // recording each swap here only recorded undo while deriving -- removed.
    return result;
}//@CODE_34135


void ChildActivationShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35118
    ChildActivationShape* pChildActivationShape;
    if (GetSender())
    {
        pChildActivationShape = new ChildActivationShape(
            (LifeLineShape*)GetLifeLineShape()->_ptrIndex, 
            (ChildActivationShape*)GetSender()->GetSender()->_ptrIndex);
        pChildActivationShape->GetSender()->CopyState(GetSender());
    }
    else
    {
        pChildActivationShape = new ChildActivationShape(
            (LifeLineShape*)GetLifeLineShape()->_ptrIndex);
    }
    pChildActivationShape->CopyState(this);
    if (GetMethod())
    {
        GetMethod()->AddChildActivationShapeLast(pChildActivationShape);
    }

    _ptrIndex = intptr_t(pChildActivationShape);
    
    ParentActivationShape::CopyShape(pSequenceDiagram);
}//@CODE_35118


void ChildActivationShape::Draw(CbPainter& painter,
                                SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                bool selected)
{//@CODE_40425
    int save = painter.Save();
    painter.SetNullBrush();
    painter.SetPen(PS_SOLID, 1, GetPenColor(painter));

    CbRect rect = GetMergeRect();

    if (selected)
    {
        painter.FillSolidRect(rect, painter.GetBkColor());
        painter.FillSolidRect(_rect, CbPainter::GetSelectFillColor());
        int saveSel = painter.Save();
        painter.SetPen(PS_SOLID, 2, CbPainter::GetSelectColor());
        painter.DrawRect(rect);
        painter.Restore(saveSel);
    }
    else if (!GetMerge() && GetLifeLineShape()->GetShowActivations())
    {
        painter.FillSolidRect(rect, painter.GetBkColor());
        painter.DrawRect(rect);
    }

    if (GetDestruction())
    {
        CbColorRef color = GetPenColor(painter);
        if (GetLifeLineShape()->GetDestructionChildActivationShape() != this)
        {
            color = Cb_RGB(255, 0, 0);
        }
        int saveDestruct = painter.Save();
        painter.SetPen(PS_SOLID, 3, color);

        CbPoint crossPoint = _rect.CenterPoint();
        crossPoint.y = _rect.top;
        const int size = 20;
        CbSize offset1(size, size);
        CbSize offset2(size, -size);
        painter.DrawLine(crossPoint+offset1, crossPoint-offset1);
        painter.DrawLine(crossPoint+offset2, crossPoint-offset2);
        painter.Restore(saveDestruct);
    }

    painter.Restore(save);

    if (GetChildActivation() && GetChildActivation()->GetSender())
    {
        SignalShape* pSignalShape = GetChildActivation()->GetSender();
        pSignalShape->Draw(painter, pSequenceDiagramViewModel,
            pSignalShape->IsSelectedIn(pSequenceDiagramViewModel));
    }

    ChildActivationShapeIterator iChildActivationShape(this);
    while (++iChildActivationShape)
    {
        iChildActivationShape->Draw(painter, pSequenceDiagramViewModel,
            iChildActivationShape->IsSelectedIn(pSequenceDiagramViewModel));
    }
}//@CODE_40425


ChildActivationShape* ChildActivationShape::GetChildActivation()
{//@CODE_33554
    return this;
}//@CODE_33554


int ChildActivationShape::GetMaxUpOffset()
{//@CODE_34263
    int maxUpOffset = GetOffset();

    if (GetParentActivationShape()->NeedExtraSpaceBefore() ||
        GetParentActivationShape()->GetPrevChildActivationShape(this))
    {
        // Above-natural drag headroom. The full space-before (25 -> grid-snaps to
        // 2 cells) lets an activation rise far enough to pull its message text into
        // whatever sits above it (e.g. a lowered/created class box). One grid is
        // enough headroom; cap it. Drag-limit ONLY -- layout still uses the full
        // GetActivationSpaceBefore(). (User: cap at 10, 2026-06-05.)
        maxUpOffset += 10;
    }

    return maxUpOffset;
}//@CODE_34263


CbRect ChildActivationShape::GetMergeRect()
{//@CODE_35393
    CbRect mergeRect = _rect;

    if (!IsRecursiveActivation())
    {
        LifeLineShape::ChildActivationShapeIterator 
            iChildActivationShape(GetLifeLineShape(), 0, this);
        while (++iChildActivationShape && 
            (iChildActivationShape->GetMerge() || iChildActivationShape->IsRecursiveActivation()))
        {
            if (iChildActivationShape->GetMerge())
            {
                mergeRect.top = iChildActivationShape->GetRect().top;
            }
        }
    }
    
    return mergeRect;
}//@CODE_35393


CbString ChildActivationShape::GetNumbering()
{//@CODE_34206
    CbString numbering;

    SeqType seqType = GetSequenceDiagram()->GetNumbering();
    if (seqType)
    {
        // A top-level activation -- a direct child of the root -- is the diagram's
        // initiator: the actor's own call, or the start method when there is no
        // actor. Like the flat scheme's !IsRootActivationShape() gate in
        // ParentActivationShape::RecalculateRect, it carries no number itself;
        // real numbering starts one level below, so the first message is "1"
        // (or "a"/"A"), not "1.1". Returning empty here also lets the deeper
        // levels recurse correctly: the first message's own children become
        // "1.1", "1.2", ... instead of "1.1.1".
        if (GetParentActivationShape()->IsRootActivationShape())
            return numbering;

        if (seqType == SEQ_1_1_1 || seqType == SEQ_a_a_a || seqType == SEQ_A_A_A)
        {
            numbering = GetParentActivationShape()->GetNumbering();
            if (!numbering.IsEmpty())
            {
                numbering += ".";
            }
        }
        
        CbString number;
        switch (seqType)
        {
        case SEQ_1:
            number.Format("%d", GetSequenceNumber());
            break;
        case SEQ_1_1_1:
            number.Format("%d", GetSequenceSubNumber());
            break;
            
        case SEQ_a:
            if (GetSequenceNumber() <= 26)
                number.Format("%c", char('a' + GetSequenceNumber()-1));
            else
                number.Format("%c%c", char('a' + ((GetSequenceNumber()-1)/26)-1), 
                                      char('a' + (GetSequenceNumber()-1)%26));
            break;
        case SEQ_a_a_a:
            if (GetSequenceNumber() <= 26)
                number.Format("%c", char('a' + GetSequenceSubNumber()-1));
            else
                number.Format("%c%c", char('a' + ((GetSequenceSubNumber()-1)/26)-1), 
                                      char('a' + (GetSequenceSubNumber()-1)%26));
            break;
            
        case SEQ_A:
            if (GetSequenceNumber() <= 26)
                number.Format("%c", char('A' + GetSequenceNumber()-1));
            else
                number.Format("%c%c", char('A' + ((GetSequenceNumber()-1)/26)-1), 
                                      char('A' + (GetSequenceNumber()-1)%26));
            break;
        case SEQ_A_A_A:
            if (GetSequenceNumber() <= 26)
                number.Format("%c", char('A' + GetSequenceSubNumber()-1));
            else
                number.Format("%c%c", char('A' + ((GetSequenceSubNumber()-1)/26)-1), 
                                      char('A' + (GetSequenceSubNumber()-1)%26));
            break;
            
        default:
            break;
        }
        
        numbering += number;
    }
    
    return numbering;
}//@CODE_34206


/*@NOTE_35063
Returns the value of member '_textColor'.
*/
CbColorRef ChildActivationShape::GetPenColor() const
{//@CODE_35063
    if (GetMethod() && GetMethod()->GetImplement())
    {
        if (GetMethod()->GetUntouched())
        {
            return _activationInitialPenColor;
        }
        
        return Shape::GetPenColor();
    }
    else if (HasUser3Method())
    {
        return _activationUser3PenColor;
    }
    else
    {
        return _activationNoMethodPenColor;
    }

}//@CODE_35063


CbColorRef ChildActivationShape::GetPenColor(CbPainter& painter)
{//@CODE_34388
    if (!painter.IsScreen())
    {
        return Shape::GetPenColor();
    }
    else if (GetMethod() && GetMethod()->GetImplement())
    {
        if (GetMethod()->GetUntouched())
        {
            return _activationInitialPenColor;
        }

        return Shape::GetPenColor();
    }
    else if (HasUser3Method())
    {
        return _activationUser3PenColor;
    }
    else
    {
        return _activationNoMethodPenColor;
    }

}//@CODE_34388


Class* ChildActivationShape::HasUser3Method(int& start, int& end,
                                            Class* pClass) const
{//@CODE_35342
    if (!pClass)
    {
        pClass = GetLifeLineShape()->GetClass();
    }

    if (pClass && GetSender())
    {
        CbString name = GetSender()->GetName();
        int index = name.Find("(");
        if (index != -1)
        {
            name = name.Left(index);
        }
        
        if (name.Find("::") == -1)
        {
            name = pClass->GetName() + "::" + name;
        }
        
        index = pClass->GetCppUser3().Find(name);
        if (index != -1)
        {
            start = index;
            end = start + name.GetLength();
            
            return pClass;
        }

        ExternClass::InheritIterator iInherit(pClass);
        while (++iInherit)
        {
            Class* pBaseClass = dynamic_cast<Class*>(iInherit->GetBaseClass());
            if (pBaseClass)
            {
                pBaseClass = HasUser3Method(start, end, pBaseClass);
                if (pBaseClass)
                {
                    return pBaseClass;
                }
            }
        }
    }

    return 0;
}//@CODE_35342


bool ChildActivationShape::HasUser3Method() const
{//@CODE_35350
    int start, end;
    
    return (HasUser3Method(start, end) ? true: false);
}//@CODE_35350


bool ChildActivationShape::IsRecursiveActivation()
{//@CODE_33571
    return (GetParentActivationShape()->GetLifeLineShape() == GetLifeLineShape());
}//@CODE_33571


bool ChildActivationShape::NeedExtraSpaceBefore()
{//@CODE_33624
    if (GetSender() && GetFirstChildActivationShape())
    {
        SignalShape* pSender = GetSender();
        SignalShape* pReceiver = GetFirstReceiver();
        // true is right side
        bool sender = pSender->IsReversed();
        bool receiver = !pReceiver->IsReversed() || pReceiver->IsRecursiveActivation();
        
        // The signals are at the same side
        if (sender == receiver)
        {
            return true;
        }
    }
    
    if (GetCreation() && 
        GetLifeLineShape()->GetFirstChildActivationShape() == this)
    {
        return true;
    }
    
    return false;
}//@CODE_33624


int ChildActivationShape::OnDelete(bool checkOnly)
{//@CODE_33232
    if (!checkOnly)
    {
        SequenceDiagram* pSequenceDiagram = GetSequenceDiagram();
        Delete();
        pSequenceDiagram->CheckAndUpdatePhase();
    }

    return 1;
}//@CODE_33232


int ChildActivationShape::OnEditAttributes(bool checkOnly)
{//@CODE_33234
    if (GetMethod())
    {
        return GetMethod()->OnEditAttributes(checkOnly);
    }

    return 0;
}//@CODE_33234


int ChildActivationShape::OnOpen(bool checkOnly)
{//@CODE_33236
    if (GetMethod() && GetMethod()->GetImplement())
    {
        return GetMethod()->OnOpen(checkOnly);
    }
    else
    {
        int start, end;
        Class* pClass = HasUser3Method(start, end);
        if (pClass)
        {
            if (!checkOnly)
            {
                void* ownerHwnd = Cb_OwnerHwnd();
                Qt_ShowUserCodeDialog(pClass, 3, false, ownerHwnd,
                                      start, end);
            }
            
            return 1;
        }
    }

    return 0;
}//@CODE_33236


bool ChildActivationShape::PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                        const CbPoint& pointLP)
{//@CODE_40874
    return SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP);
}//@CODE_40874


/*@NOTE_34386
Set the value of member '_textColor' to 'penColor'.
*/
void ChildActivationShape::SetPenColor(CbColorRef penColor)
{//@CODE_34386
    if (GetMethod() && GetMethod()->GetImplement())
    {
        Shape::SetPenColor(penColor);
    }
    else if (HasUser3Method())
    {
        _activationUser3PenColor = penColor;
    }
    else
    {
        _activationNoMethodPenColor = penColor;
    }
    
}//@CODE_34386


/*@NOTE_34138
Set the value of member '_rect' to 'rRect'.
*/
void ChildActivationShape::SetRect(const CbRect& rRect)
{//@CODE_34138
    CbRect newRect = rRect;
    CbRect oldRect = _rect;
    
    if (oldRect != newRect)
    {
        // _rect is DERIVED from _offset (recomputed every paint via
        // RecalculateRect); activation edits SaveState the activation + _offset
        // explicitly, so this internal SaveState was redundant on edits and
        // wrong on the derived paint recompute -- removed so the recompute
        // records no undo (calculating derived geometry must not mutate the model).
        // The note-follow is no longer queued here: RecalculateRect records each
        // activation move it makes into the edit's move-list (RecalculateAfterEdit).
        _rect = newRect;
    }
}//@CODE_34138


int ChildActivationShape::UsesTextColor() const
{//@CODE_34409
    return 0;
}//@CODE_34409


bool ChildActivationShape::WrongCreationOrDestruction()
{//@CODE_34394
    if (GetCreation() && GetLifeLineShape()->GetPrevChildActivationShape(this))
    {
        if (!IsRecursiveActivation() ||
            !GetSender()->GetSender()->GetCreation())
        {
            return true;
        }
        else
        {
            ChildActivationShapeIterator iChild(GetParentActivationShape());
            while (++iChild && iChild.Get() != this)
            {
                if (!iChild->IsRecursiveActivation() ||
                    !iChild->GetCreation())
                {
                    return true;
                }
            }
        }
    }

    if (GetDestruction() && 
        GetLifeLineShape()->GetDestructionChildActivationShape() != this)
    {
        return true;
    }

    return false;
}//@CODE_34394


/*@NOTE_34256
Returns the value of member '_offset'.
*/
int ChildActivationShape::GetOffset() const
{//@CODE_34256
    return _offset;
}//@CODE_34256


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_31799
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ChildActivationShape::CleanupReferences()
{
    ParentActivationShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    CLEANUP_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    CLEANUP_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_31789
Method which must be called first in a constructor.
*/
void ChildActivationShape::ConstructorInclude(LifeLineShape* pLifeLineShape,
                                              ParentActivationShape* pParentActivationShape)
{
    INIT_SINGLE_OWNED_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    INIT_MULTI_OWNED_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    INIT_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_31791
Method which must be called first in a destructor.
*/
void ChildActivationShape::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    EXIT_MULTI_OWNED_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    EXIT_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    EXIT_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    EXIT_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_31800
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ChildActivationShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    REMOVE_SINGLE_OWNED_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    ParentActivationShape::RemoveReferences();
    REMOVE_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    REMOVE_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    REMOVE_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_31801
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ChildActivationShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ChildActivationShape* pChildActivationShape = (ChildActivationShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    RESTORE_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    RESTORE_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    ParentActivationShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_31803
Save the state of the current object relations to pDataModelDocObject.
*/
void ChildActivationShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ParentActivationShape::SaveReferences(pDataModelDocObject);
    ChildActivationShape* pChildActivationShape = (ChildActivationShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    SAVE_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    SAVE_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_31794
Serialize the members only to a CbObject object.
*/
void ChildActivationShape::Serialize(CbArchive& archive)
{
    ParentActivationShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _creation;
        archive << _destruction;
        archive << _offset;
        archive << _activationNoMethodPenColor;
        archive << _activationUser3PenColor;
        archive << _merge;
        archive << _activationInitialPenColor;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _creation;
            archive >> _destruction;
            archive >> _offset;
            archive >> _activationNoMethodPenColor;
            archive >> _activationUser3PenColor;
            archive >> _merge;
            archive >> _activationInitialPenColor;
        }
    }
}


/*@NOTE_31793
Method which must be called first in a serialize constructor.
*/
void ChildActivationShape::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    INIT_MULTI_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    INIT_MULTI_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    INIT_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_31796
Serialize the relations to a CbObject object.
*/
void ChildActivationShape::SerializeRelations(CbArchive& archive,
                                              DataModelDocObject* pointerArray[])
{
    ParentActivationShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
        WRITE_MULTI_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
            READ_MULTI_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ChildActivationShape)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
METHODS_MULTI_OWNED_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
METHODS_ITERATOR_MULTI_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
METHODS_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
METHODS_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
METHODS_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
