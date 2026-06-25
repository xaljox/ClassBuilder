/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SignalShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SignalShape'
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
#include "qt/QtSignalDialog.h"
#include "CbPainter.h"
#include <math.h>
//@END_USER2


// Static members
CbPainter* SignalShape::_measurePainter = nullptr;


/*@NOTE_32499
Constructor method.
*/
SignalShape::SignalShape(ChildActivationShape* pReceiver,
                         ChildActivationShape* pSender) //@INIT_32499
    : SequenceDiagramShape(pReceiver->GetSequenceDiagram(), 
        pReceiver->GetDataModelDoc()->GetSignalPenColor(),
        pReceiver->GetDataModelDoc()->GetSignalTextColor())
    , _name()
    , _clause()
    , _label()
    , _nameOffset(30, 0)
    , _labelOffset(30, -30)
    , _scope(false)
    , _arguments(pReceiver->GetSequenceDiagram()->GetArguments())
    , _signalNoMethodPenColor(pReceiver->GetDataModelDoc()->GetSignalNoMethodPenColor())
    , _enableReturn(false)
    , _return()
    , _returnActiveAreaRect()
    , _async(false)
    , _returnOffset(30, 0)
    , _activeAreaRect()
    , _argumentNames(false)
    , _duration(0)
{//@CODE_32499
    ConstructorInclude(pReceiver, pSender);

    // Put in your own code
}//@CODE_32499


/*@NOTE_32115
Constructor needed for serialization, not meant to use for other purposes!
*/
SignalShape::SignalShape() //@INIT_32115
    : SequenceDiagramShape()
    , _name()
    , _clause()
    , _label()
    , _nameOffset(30, 0)
    , _labelOffset(30, -30)
    , _scope(false)
    , _arguments(false)
    , _signalNoMethodPenColor(Cb_RGB(128, 128, 128))
    , _enableReturn(false)
    , _return()
    , _returnActiveAreaRect()
    , _async(false)
    , _returnOffset(30, 0)
    , _activeAreaRect()
    , _argumentNames(false)
    , _duration(0)
{//@CODE_32115
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_32115


/*@NOTE_32113
Destructor method.
*/
SignalShape::~SignalShape()
{//@CODE_32113
    DestructorInclude();

    // Put in your own code
}//@CODE_32113


void SignalShape::AddContribution(CbPainter& painter)
{//@CODE_38427
    LifeLineShape* pSndLL = GetSender()->GetLifeLineShape();
    LifeLineShape* pRecvLL = GetReceiver()->GetLifeLineShape();
    SequenceDiagram* pSD = pSndLL->GetSequenceDiagram();
    int needed = GetRequiredLifelineDistance(painter);

    if (pSndLL == pRecvLL)
    {
        // Self-loop: constraint pushes the right neighbour.
        pRecvLL = pSD->GetNextLifeLineShape(pSndLL);
        if (!pRecvLL) return;
    }

    // Merge with any existing contribution for the same unordered pair.
    // The Contrib constructor normalises (left, right) and FindContrib
    // maps either lookup-order to the stored entry, so e.g. A->B and
    // B->A collapse into one max-needed entry.
    Contrib* pExisting = Contrib::FindContrib(pSndLL, pRecvLL);
    if (pExisting)
    {
        if (needed > pExisting->GetNeeded())
            pExisting->SetNeeded(needed);
    }
    else
    {
        (void)new Contrib(pSndLL, pRecvLL, needed);
    }
}//@CODE_38427


void SignalShape::AddContribution()
{//@CODE_38574
    LifeLineShape* pSndLL = GetSender()->GetLifeLineShape();
    LifeLineShape* pRecvLL = GetReceiver()->GetLifeLineShape();

    if (pSndLL != pRecvLL)
    {
        // No Self-loop: 
    
        // Increment any existing contribution for the same unordered pair.
        // The Contrib constructor normalises (left, right) and FindContrib
        // maps either lookup-order to the stored entry, so e.g. A->B and
        // B->A collapse into one entry.
        Contrib* pExisting = Contrib::FindContrib(pSndLL, pRecvLL);
        if (pExisting)
        {
            pExisting->IncrementNeeded();
        }
        else
        {
            (void)new Contrib(pSndLL, pRecvLL, 1);
        }
    }
}//@CODE_38574


void SignalShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35120
}//@CODE_35120


void SignalShape::Draw(CbPainter& painter,
                       SequenceDiagramViewModel* pSequenceDiagramViewModel,
                       bool selected)
{//@CODE_40433
    int save = painter.Save();
    painter.SetNullBrush();

    CbColorRef penColor = GetPenColor(painter);
    int penWidth = 1;
    if (selected)
    {
        painter.SetTextColor(CbPainter::GetSelectColor());
        painter.SetBold(true);
        penColor = CbPainter::GetSelectColor();
        penWidth = 3;
    }
    else
    {
        painter.SetTextColor(GetTextColor());
    }

    CbRect rect(0, 0, 0, 0);

    if (!GetLabel().IsEmpty())
    {
        rect *= GetLabelRect(painter, true);
    }

    if (GetEnableReturn() && !GetReturn().IsEmpty())
    {
        rect *= GetReturnRect(painter, true);
    }

    rect *= GetNameRect(painter, true);

    painter.SetPen(PS_SOLID, penWidth, penColor);
    CbPoint startPoint = GetStartPoint();

    CbSize inflate(0, 10);
    if (IsRecursiveActivation())
    {
        CbPoint point1 = GetStartPoint() + CbSize(SequenceDiagram::GetSignalLengthRecursive(), 0) + CbSize(0, -_duration/2);
        CbPoint point2 = point1 + CbSize(0, -SequenceDiagram::GetActivationSpaceRecursive());
        painter.DrawLine(GetStartPoint(), point1);
        painter.DrawLine(point1, point2);
        painter.DrawLine(point2, GetEndPoint());
        startPoint = point2;

        CbRect activeAreaRect(GetStartPoint()+inflate, point2-inflate);
        activeAreaRect.NormalizeRect();
        rect *= activeAreaRect;
    }
    else
    {
        painter.DrawLine(GetStartPoint(), GetEndPoint());

        CbRect activeAreaRect(GetStartPoint()+inflate, GetEndPoint()-inflate);
        activeAreaRect.NormalizeRect();
        rect *= activeAreaRect;
    }
    DrawArrow(painter, penColor, startPoint, GetEndPoint());

    if (GetEnableReturn())
    {
        int saveRet = painter.Save();
        painter.SetPen(PS_DASH, penWidth, penColor);
        painter.DrawLine(GetReturnStartPoint(), GetReturnEndPoint());

        CbRect returnActiveAreaRect(GetReturnStartPoint()+inflate, GetReturnEndPoint()-inflate);
        returnActiveAreaRect.NormalizeRect();
        rect *= returnActiveAreaRect;

        DrawArrow(painter, penColor, GetReturnStartPoint(), GetReturnEndPoint(), true);
        painter.Restore(saveRet);
    }

    SetRect(rect);

    painter.Restore(save);
}//@CODE_40433


void SignalShape::DrawArrow(CbPainter& painter, CbColorRef color,
                            const CbPoint& start, const CbPoint& end,
                            bool returnArrow)
{//@CODE_33605
    double x = start.x - end.x;
    double y = start.y - end.y;
    double distance = sqrt(x*x + y*y);
    if (distance == 0.0)
    {
        return;
    }
    x /= distance;
    y /= distance;

    int save = painter.Save();
    painter.SetPen(PS_SOLID, 1, color);
    painter.SetSolidBrush(color);

    int size = 16;
    if (GetAsync() && !returnArrow)
        size = 20;

    CbSize a(int(x*size), int(y*size));
    CbSize b(int(-y*(size/2)), int(x*(size/2)));
    if (b.cy < 0)
    {
        b = -b;
    }

    if (returnArrow)
    {
        painter.DrawLine(end, end+a-b);
        painter.DrawLine(end, end+a+b);
    }
    else if (GetAsync())
    {
        painter.DrawLine(end, end+a+b);
    }
    else
    {
        CbPoint points[3];
        points[0] = end;
        points[1] = end+a-b;
        points[2] = end+a+b;

        painter.Polygon(points, 3);
    }

    painter.Restore(save);
}//@CODE_33605


void SignalShape::DrawLabel(CbPainter& painter)
{//@CODE_34466
}//@CODE_34466


ChildActivationShape* SignalShape::GetChildActivation()
{//@CODE_34254
    return GetReceiver();
}//@CODE_34254


CbPoint SignalShape::GetEndPoint()
{//@CODE_33603
    CbPoint endPoint;
    
    if (GetReceiver()->GetCreation() && 
        GetReceiver()->GetLifeLineShape()->GetFirstChildActivationShape() == GetReceiver())
    {
        CbRect rectLifeLine = GetReceiver()->GetLifeLineShape()->GetRect();
        endPoint.y = rectLifeLine.top + 30;
        
        if (IsReversed())
        {
            endPoint.x = rectLifeLine.right;
        }
        else
        {
            endPoint.x = rectLifeLine.left;
        }
    }
    else
    {
        CbRect rectReceiver = GetReceiver()->GetRect();
        endPoint.y = rectReceiver.bottom;
        
        if (GetReceiver()->GetLifeLineShape()->GetShowActivations())
        {
            if (IsReversed())
            {
                endPoint.x = rectReceiver.right;
            }
            else
            {
                endPoint.x = rectReceiver.left;
            }
        }
        else
        {
            endPoint.x = GetReceiver()->GetRect().CenterPoint().x;
        }
    }
    
    return endPoint;
}//@CODE_33603


CbPoint SignalShape::GetLabelPoint()
{//@CODE_34180
    // No rounding of the absolute position -- track reference + offset so the
    // gap to the line stays constant and drag-out-and-back returns exactly. The
    // offset stays grid-aligned via the drag's rounded delta. (Same as
    // GetNamePoint; see the note there.)
    return GetReferencePoint() + _labelOffset;
}//@CODE_34180


CbRect SignalShape::GetLabelRect(CbPainter& painter, bool draw)
{//@CODE_34463
    int save = painter.Save();

    painter.SetFont(CBF_LABEL);
    painter.SetTextAlign(TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);

    // Start from a zero-size rect at the label point and let CalcText
    // (DT_CALCRECT in the CDC backend, QFontMetrics::boundingRect in the
    // Qt backend) expand it to the text's bounds.
    CbRect rect(GetLabelPoint(), GetLabelPoint());
    const unsigned int calcFlags =
        DT_LEFT|DT_NOCLIP|DT_EXTERNALLEADING|DT_NOPREFIX|DT_EXPANDTABS;
    painter.CalcText(GetLabel(), rect, calcFlags);

    if (draw)
    {
        // Anchor TextOut at the rect's model bottom (LabelPoint.y +
        // line-height). The old CDC backend drew with DrawText under
        // MM_ISOTROPIC's inverted Y, which landed at this same model point;
        // matching it keeps the label hit-test / drag rect math unchanged
        // now that backend is gone.
        painter.SetTextAlign(TA_LEFT|TA_TOP|TA_NOUPDATECP);
        painter.TextOut(GetLabelPoint().x, rect.bottom, GetLabel());
    }
    rect.NormalizeRect();

    // CalcText already returns a rect covering the drawn label (drawn at
    // rect.bottom with TA_TOP), so no extra +line-height shift is needed.
    // (The removed CDC backend needed one to compensate MM_ISOTROPIC's
    // inverted Y; that path is gone.)

    painter.Restore(save);

    return rect;
}//@CODE_34463


CbPoint SignalShape::GetNamePoint()
{//@CODE_34179
    // Position = reference + offset, with NO rounding of the absolute point.
    // Rounding the absolute position snapped the name to the 10-grid while the
    // signal line / activation sit at non-grid coords, so the gap to the line
    // jittered as the activation moved AND a drag-out-then-back didn't return
    // exactly (the offset got re-based through the rounding). The OFFSET stays
    // grid-aligned because the drag rounds its delta, so dragging still snaps in
    // grid steps and reverses cleanly, while the resting position tracks the
    // reference.
    return GetReferencePoint() + _nameOffset;
}//@CODE_34179


CbRect SignalShape::GetNameRect(CbPainter& painter, bool draw)
{//@CODE_34469
    int save = painter.Save();

    painter.SetFont(CBF_SIGNAL);
    painter.SetTextAlign(TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);
    CbRect rect(GetNamePoint(), painter.GetTextExtent(GetSignalName()));
    rect.NormalizeRect();

    if (draw)
    {
        painter.TextOut(GetNamePoint().x, GetNamePoint().y, GetSignalName());
    }

    painter.Restore(save);

    return rect;
}//@CODE_34469


/*@NOTE_35062
Returns the value of member '_textColor'.
*/
CbColorRef SignalShape::GetPenColor() const
{//@CODE_35062
    if (GetReceiver()->GetMethod())
    {
        return Shape::GetPenColor();
    }
    else
    {
        return _signalNoMethodPenColor;
    }
    
}//@CODE_35062


CbColorRef SignalShape::GetPenColor(CbPainter& painter)
{//@CODE_34382
    if (painter.IsScreen()&& GetReceiver()->WrongCreationOrDestruction())
    {
        return Cb_RGB(255, 0, 0);
    }
    else if (painter.IsScreen() && !GetReceiver()->GetMethod())
    {
        return _signalNoMethodPenColor;
    }
    else
    {
        return Shape::GetPenColor();
    }
}//@CODE_34382


CbPoint SignalShape::GetReferencePoint()
{//@CODE_33617
    if (IsReversed() && !IsRecursiveActivation())
    {
        return GetEndPoint();
    }
    else
    {
        return GetStartPoint();
    }
}//@CODE_33617


int SignalShape::GetRequiredLifelineDistance(CbPainter& painter)
{//@CODE_38312
    // Minimum centerline-to-centerline distance from the sender lifeline to
    // the receiver (for normal signals) or to the right neighbour (for
    // self-loops, where the recursive arrow extends rightward and pushes
    // whoever sits next to the lifeline). Drives OptimizePlacement layout.
    //
    // Each text is drawn at the signal's reference point plus a user-
    // adjustable offset, so the worst-case rightward reach from the
    // reference point is offset.cx + textWidth, taken as the max across
    // name/label/return.
    static const int ARROW_TEXT_PAD = 30;

    int reach = 0;
    CbSize ext;
    ext = painter.GetTextExtent(GetSignalName());
    if (GetNameOffset().cx + ext.cx > reach)
        reach = GetNameOffset().cx + ext.cx;
    ext = painter.GetTextExtent(GetLabel());
    if (GetLabelOffset().cx + ext.cx > reach)
        reach = GetLabelOffset().cx + ext.cx;
    ext = painter.GetTextExtent(GetReturn());
    if (GetReturnOffset().cx + ext.cx > reach)
        reach = GetReturnOffset().cx + ext.cx;

    int activationWidth = SequenceDiagram::GetActivationWidth();
    int req = reach + ARROW_TEXT_PAD;

    // Constructor signals (receiver activation has the Creation flag) end
    // at the receiver class's box LEFT edge instead of its activation,
    // losing actW/2 + receiver.width/2 on the receiver side. Only applies
    // to inter-lifeline signals (a self-loop can't be a creation).
    LifeLineShape* pToLL = GetReceiver()->GetLifeLineShape();
    bool selfLoop = (GetSender()->GetLifeLineShape() == pToLL);
    if (!selfLoop && GetReceiver()->GetCreation())
    {
        int receiverWidth = pToLL->GetRect().Width();
        return req + activationWidth/2 + receiverWidth/2;
    }

    // Normal signal: arrow loses one full activation width (sender's right
    // half + receiver's left half). Self-loops draw their text the same
    // way as a rightward signal between this lifeline and its right
    // neighbour, so the geometry is identical.
    return req + activationWidth;
}//@CODE_38312


CbPoint SignalShape::GetReturnEndPoint()
{//@CODE_34424
    CbRect rectSender = GetSender()->GetRect();
    CbPoint endPoint(rectSender.CenterPoint().x, GetReturnStartPoint().y);
    
    if (GetSender()->GetLifeLineShape()->GetShowActivations())
    {
        if (IsReversed())
        {
            endPoint.x = rectSender.left;
        }
        else
        {
            endPoint.x = rectSender.right;
        }
    }
    
    return endPoint;
}//@CODE_34424


CbPoint SignalShape::GetReturnPoint()
{//@CODE_34440
    // No rounding of the absolute position (same as GetNamePoint/GetLabelPoint):
    // track reference + offset so the gap to the return line stays constant and
    // drag-out-and-back returns exactly; the offset stays grid-aligned via the
    // drag's rounded delta.
    return GetReturnReferencePoint() + _returnOffset;
}//@CODE_34440


CbRect SignalShape::GetReturnRect(CbPainter& painter, bool draw)
{//@CODE_34472
    int save = painter.Save();

    painter.SetFont(CBF_SIGNAL);
    painter.SetTextAlign(TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);
    CbRect rect(GetReturnPoint(), painter.GetTextExtent(GetReturn()));
    rect.NormalizeRect();

    if (draw)
    {
        painter.TextOut(GetReturnPoint().x, GetReturnPoint().y, GetReturn());
    }

    painter.Restore(save);

    return rect;
}//@CODE_34472


CbPoint SignalShape::GetReturnReferencePoint()
{//@CODE_34433
    if (IsReversed())
    {
        return GetReturnStartPoint();
    }
    else
    {
        return GetReturnEndPoint();
    }
}//@CODE_34433


CbPoint SignalShape::GetReturnStartPoint()
{//@CODE_34423
    CbRect rectReceiver = GetReceiver()->GetRect();
    CbPoint startPoint(rectReceiver.CenterPoint().x, rectReceiver.top);
    
    if (GetReceiver()->GetLifeLineShape()->GetShowActivations())
    {
        if (IsReversed())
        {
            startPoint.x = rectReceiver.right;
        }
        else
        {
            startPoint.x = rectReceiver.left;
        }
    }
    
    return startPoint;
}//@CODE_34423


SignalShape* SignalShape::GetSignal()
{//@CODE_33555
    return this;
}//@CODE_33555


CbString SignalShape::GetSignalName()
{//@CODE_34210
    CbString name = GetReceiver()->GetNumbering();

    if (!name.IsEmpty() && !GetClause().IsEmpty())
    {
        name += " ";
    }
    
    name += GetClause();
    
    if (!name.IsEmpty())
    {
        name += ":";
    }
    
    name += GetName();
    
    return name;
}//@CODE_34210


CbPoint SignalShape::GetStartPoint()
{//@CODE_33602
    CbRect rectSender = GetSender()->GetRect();
    CbPoint startPoint(rectSender.CenterPoint().x, GetEndPoint().y+GetDuration());
    
    if (IsRecursiveActivation())
    {
        startPoint.y += SequenceDiagram::GetActivationSpaceRecursive();
        if (GetSender()->GetLifeLineShape()->GetShowActivations())
        {
            startPoint.x = rectSender.right;
        }
    }
    else
    {
        if (GetSender()->GetLifeLineShape()->GetShowActivations())
        {
            if (IsReversed())
            {
                startPoint.x = rectSender.left;
            }
            else
            {
                startPoint.x = rectSender.right;
            }
        }
    }
    
    return startPoint;
}//@CODE_33602


bool SignalShape::IsRecursiveActivation()
{//@CODE_33604
    return GetReceiver()->IsRecursiveActivation();
}//@CODE_33604


bool SignalShape::IsReversed()
{//@CODE_33601
    if (GetReceiver()->IsRecursiveActivation() ||
        GetReceiver()->GetLifeLineShape()->GetLeftActivation() <
        GetSender()->GetLifeLineShape()->GetLeftActivation())
    {
        return true;
    }
    
    return false;
}//@CODE_33601


int SignalShape::OnDelete(bool checkOnly)
{//@CODE_33254
    return GetReceiver()->OnDelete(checkOnly);
}//@CODE_33254


int SignalShape::OnEditAttributes(bool checkOnly)
{//@CODE_33256
    return GetReceiver()->OnEditAttributes(checkOnly);
}//@CODE_33256


int SignalShape::OnOpen(bool checkOnly)
{//@CODE_33258
    if (!checkOnly)
    {
        void* ownerHwnd = Cb_OwnerHwnd();
        Qt_ShowSignalDialog(this, ownerHwnd);
    }

    return 1;
}//@CODE_33258


bool SignalShape::PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                               const CbPoint& pointLP)
{//@CODE_40886
    if (_activeAreaRect.PtInRect(pointLP))
    {
        return true;
    }

    if (GetEnableReturn() && _returnActiveAreaRect.PtInRect(pointLP))
    {
        return true;
    }

    auto hitText = [&](CbPainter& painter) -> bool
    {
        if (GetNameRect(painter).PtInRect(pointLP))
            return true;
        if (!GetLabel().IsEmpty() && GetLabelRect(painter).PtInRect(pointLP))
            return true;
        if (GetEnableReturn() && !GetReturn().IsEmpty() &&
            GetReturnRect(painter).PtInRect(pointLP))
            return true;
        return false;
    };

    if (CbPainter* pMeasure = GetMeasurePainter())
        return hitText(*pMeasure);
    return false;
}//@CODE_40886


void SignalShape::SetLabelPoint(const CbPoint& point)
{//@CODE_34181
    CbSize labelOffset = point - GetReferencePoint();
    if (_labelOffset != labelOffset)
    {
        SaveState();
        _labelOffset = labelOffset;
    }
}//@CODE_34181


void SignalShape::SetNamePoint(const CbPoint& point)
{//@CODE_34183
    CbSize nameOffset = point - GetReferencePoint();
    if (_nameOffset != nameOffset)
    {
        SaveState();
        _nameOffset = nameOffset;
    }
}//@CODE_34183


/*@NOTE_34384
Set the value of member '_textColor' to 'penColor'.
*/
void SignalShape::SetPenColor(CbColorRef penColor)
{//@CODE_34384
    if (GetReceiver()->GetMethod())
    {
        Shape::SetPenColor(penColor);
    }
    else
    {
        _signalNoMethodPenColor = penColor;
    }
    
}//@CODE_34384


void SignalShape::SetReturnPoint(const CbPoint& point)
{//@CODE_34438
    CbSize returnOffset = point - GetReturnReferencePoint();
    if (_returnOffset != returnOffset)
    {
        SaveState();
        _returnOffset = returnOffset;
    }
}//@CODE_34438


/*@NOTE_34798
Set the value of member '_activeAreaRect' to 'rActiveAreaRect'.
*/
void SignalShape::SetActiveAreaRect(const CbRect& rActiveAreaRect)
{//@CODE_34798
    // _activeAreaRect is the DERIVED hit-test region, recomputed every paint in Draw.
    // Draw must NOT alter the model, so there is NO note-follow here -- the follow is
    // calculated once at the edit boundary (SequenceDiagram::RecalculateAfterEdit),
    // against _noteAnchorRect (the last-resolved active area, which paint never touches).
    _activeAreaRect = rActiveAreaRect;
}//@CODE_34798


/*@NOTE_34235
Returns the value of member '_note'.
*/
const CbString& SignalShape::GetNote()
{//@CODE_34235
    return _note;
}//@CODE_34235


/*@NOTE_34236
Set the value of member '_note' to 'rNote'.
*/
void SignalShape::SetNote(const CbString& rNote)
{//@CODE_34236
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_34236


/*@NOTE_34794
Set the value of member '_returnActiveAreaRect' to 'rReturnActiveAreaRect'.
*/
void SignalShape::SetReturnActiveAreaRect(const CbRect& rReturnActiveAreaRect)
{//@CODE_34794
    // Derived hit-test region (see SetActiveAreaRect) -- no model change in paint.
    _returnActiveAreaRect = rReturnActiveAreaRect;
}//@CODE_34794


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_32122
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SignalShape::CleanupReferences()
{
    SequenceDiagramShape::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    CLEANUP_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
}


/*@NOTE_32112
Method which must be called first in a constructor.
*/
void SignalShape::ConstructorInclude(ChildActivationShape* pReceiver,
                                     ChildActivationShape* pSender)
{
    INIT_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    INIT_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
}


/*@NOTE_32114
Method which must be called first in a destructor.
*/
void SignalShape::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    EXIT_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
}


/*@NOTE_32123
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SignalShape::RemoveReferences()
{
    SequenceDiagramShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    REMOVE_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
}


/*@NOTE_32124
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SignalShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SignalShape* pSignalShape = (SignalShape*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    RESTORE_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    SequenceDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_32126
Save the state of the current object relations to pDataModelDocObject.
*/
void SignalShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape::SaveReferences(pDataModelDocObject);
    SignalShape* pSignalShape = (SignalShape*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    SAVE_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
}


/*@NOTE_32117
Serialize the members only to a CbObject object.
*/
void SignalShape::Serialize(CbArchive& archive)
{
    SequenceDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
        archive << _clause;
        archive << _label;
        archive << _nameOffset;
        archive << _labelOffset;
        archive << _scope;
        archive << _arguments;
        archive << _note;
        archive << _signalNoMethodPenColor;
        archive << _enableReturn;
        archive << _return;
        archive << _returnActiveAreaRect;
        archive << _async;
        archive << _returnOffset;
        archive << _activeAreaRect;
        archive << _argumentNames;
        archive << _duration;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
            archive >> _clause;
            archive >> _label;
            archive >> _nameOffset;
            archive >> _labelOffset;
            archive >> _scope;
            archive >> _arguments;
            archive >> _note;
            archive >> _signalNoMethodPenColor;
            archive >> _enableReturn;
            archive >> _return;
            archive >> _returnActiveAreaRect;
            archive >> _async;
            archive >> _returnOffset;
            archive >> _activeAreaRect;
            archive >> _argumentNames;
            archive >> _duration;
        }
    }
}


/*@NOTE_32116
Method which must be called first in a serialize constructor.
*/
void SignalShape::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    INIT_MULTI_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)
}


/*@NOTE_32119
Serialize the relations to a CbObject object.
*/
void SignalShape::SerializeRelations(CbArchive& archive,
                                     DataModelDocObject* pointerArray[])
{
    SequenceDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(SignalShape)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
METHODS_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
