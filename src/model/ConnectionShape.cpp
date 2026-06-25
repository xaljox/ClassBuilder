/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ConnectionShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ConnectionShape'
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
//@END_USER2


// Static members


ConnectionShape::ConnectionShape(ClassDiagram* pClassDiagram,
                                 ClassShape* pFromClassShape,
                                 ClassShape* pToClassShape, CbColorRef penColor,
                                 CbColorRef textColor) //@INIT_4373
    : ClassDiagramShape(pClassDiagram, penColor, textColor)
    , _startPoint(pFromClassShape->ConnectionPoint(pToClassShape))
    , _endPoint(pToClassShape->ConnectionPoint(pFromClassShape))
    , _minStartSize(GetMinSize(pFromClassShape->GetRect(), _startPoint))
    , _minEndSize(GetMinSize(pToClassShape->GetRect(), _endPoint))
    , _initial(true)
{//@CODE_4373
    ConstructorInclude(pFromClassShape, pToClassShape);
    
    // Put in your own code
}//@CODE_4373


/*@NOTE_4575
Copy constructor, needed for temporary copy, while modifying shape.
*/
ConnectionShape::ConnectionShape(ConnectionShape* pConnectionShape) //@INIT_4575
    : ClassDiagramShape(pConnectionShape->GetClassDiagram(),
        pConnectionShape->GetPenColor(),
        pConnectionShape->GetTextColor())
    , _startPoint(pConnectionShape->GetStartPoint())
    , _endPoint(pConnectionShape->GetEndPoint())
    , _minStartSize(pConnectionShape->GetMinStartSize())
    , _minEndSize(pConnectionShape->GetMinEndSize())
    , _initial(true)
{//@CODE_4575
    ConstructorInclude(pConnectionShape->GetFromClassShape(), 
        pConnectionShape->GetToClassShape());

    // Put in your own code
    ConnectionSegmentIterator iConnectionSegment(pConnectionShape);
    while (++iConnectionSegment)
    {
        (void)new ConnectionSegment(this, iConnectionSegment);
    }
}//@CODE_4575


/*@NOTE_4195
Constructor needed for serialization, not meant to use for other purposes!
*/
ConnectionShape::ConnectionShape() //@INIT_4195
    : ClassDiagramShape()
    , _initial(true)
{//@CODE_4195
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4195


/*@NOTE_35286
Constructor method.
*/
ConnectionShape::ConnectionShape(ClassDiagram* pClassDiagram,
                                 ConnectionShape* pConnectionShape) //@INIT_35286
    : ClassDiagramShape(pClassDiagram,
        pConnectionShape->GetPenColor(),
        pConnectionShape->GetTextColor())
    , _startPoint(pConnectionShape->GetStartPoint())
    , _endPoint(pConnectionShape->GetEndPoint())
    , _minStartSize(pConnectionShape->GetMinStartSize())
    , _minEndSize(pConnectionShape->GetMinEndSize())
    , _initial(true)
{//@CODE_35286
    // Put in your own code
    ConstructorInclude((ClassShape*)pConnectionShape->GetFromClassShape()->_ptrIndex, 
        (ClassShape*)pConnectionShape->GetToClassShape()->_ptrIndex);

    // Put in your own code
    ConnectionSegmentIterator iConnectionSegment(pConnectionShape);
    while (++iConnectionSegment)
    {
        (void)new ConnectionSegment(this, iConnectionSegment);
    }

    if (pConnectionShape->GetHidden())
    {
        pClassDiagram->AddHiddenLast(this);
    }
}//@CODE_35286


/*@NOTE_4193
Destructor method.
*/
ConnectionShape::~ConnectionShape()
{//@CODE_4193
    DestructorInclude();

    // Put in your own code
}//@CODE_4193


/*@NOTE_4537
Calculate how many segments are needed if routed again.
*/
int ConnectionShape::CountNewRouting()
{//@CODE_4537
    int count = 0;
    
    CbSize size = (_endPoint - _startPoint) - _minStartSize + _minEndSize;
    
    if (GetMinStartSize().cy == 0)
    {
        if ((GetMinStartSize().cx < 0 && size.cx <= 0) ||
            (GetMinStartSize().cx > 0 && size.cx >= 0))
        {
            count += 1;
        }
        else
        {
            CbSize extra(0, (size.cy/20)*10);
            // Check if we have crossing
            if ((GetMinEndSize().cy < 0 && size.cy < 0) ||
                (GetMinEndSize().cy > 0 && size.cy > 0))
            {
                extra.cy = size.cy;
            }
            size -= extra; // Compensation for the future
            
            count += 3;
        }
    }
    else
    {
            count += 2;
    }
    
    if (GetMinEndSize().cx == 0)
    {
        // Note we have to invert the MinEndSize
        if ((GetMinEndSize().cy > 0 && size.cy <= 0) ||
            (GetMinEndSize().cy < 0 && size.cy >= 0))
        {
            count += 1;
        }
        else
        {
            count = 4;
        }
    }
    else
    {
        if ((count == 2) &&
            ((GetMinStartSize().cy < 0 && size.cy <= 0) ||
             (GetMinStartSize().cy > 0 && size.cy >= 0)) &&
            ((GetMinEndSize().cx > 0 && size.cx <= 0) ||
             (GetMinEndSize().cx < 0 && size.cx >= 0)))
        {
            // No update on count
        }
        else
        {
            count += 2;
        }
    }
    
    if (count == 2)
    {
        // If only 2 segments and they are in the same axis add a third in between
        if ((GetMinStartSize().cx == 0 && GetMinEndSize().cx == 0) ||
            (GetMinStartSize().cy == 0 && GetMinEndSize().cy == 0))
        {
            count += 1;
        }
    }

    return count;
}//@CODE_4537


void ConnectionShape::Draw(CbPainter& painter,
                           ClassDiagramViewModel* pClassDiagramViewModel,
                           bool selected)
{//@CODE_40367
    ConnectionShape::ConnectionSegmentIterator iConnectionSegment(this);
    while (++iConnectionSegment)
        iConnectionSegment->Draw(painter);

    // Per-segment route handles when selected -- the affordance for the upcoming
    // segment / endpoint drag (mirrors the MFC overload + the class/note Qt
    // selection). Drawn for every connection type since each calls this base.
    // Screen-only.
    if (painter.IsScreen() && selected)
    {
        iConnectionSegment.Reset();
        while (++iConnectionSegment)
            iConnectionSegment->DrawSelectedRect(painter, CbPainter::GetSelectColor());
    }
}//@CODE_40367


ConnectionShape* ConnectionShape::GetConnectionShape()
{//@CODE_4574
    return this;
}//@CODE_4574


/*@NOTE_4395
Get a vector of the minimum size one should go away, from a connection point of a 
ClassShape.
*/
CbSize ConnectionShape::GetMinSize(const CbRect& rect, const CbPoint& point)
{//@CODE_4395
    CbSize minSize(0, 0);
    const int offset = 60;

    if (rect.bottom == point.y)
        minSize.cy = offset;
    else if (rect.top == point.y)
    {
        // Compensate to get it on grid
        int compensation = 10 - abs(point.y%10);
        if (compensation == 10)
            compensation = 0;
        minSize.cy = -(offset + compensation);
    }
    else if (rect.left == point.x)
        minSize.cx = -offset;
    else if (rect.right == point.x)
        minSize.cx = offset;

    return minSize;
}//@CODE_4395


int ConnectionShape::GetOwned() const
{//@CODE_35783
    return 0;
}//@CODE_35783


/*@NOTE_6102
Hide shape from the drawing, by add it to the relation hidden
*/
void ConnectionShape::Hide()
{//@CODE_6102
    if (!GetHidden())
    {
        SaveState(1);
        GetClassDiagram()->AddHiddenLast(this);
    }
}//@CODE_6102


bool ConnectionShape::IsFromAtBottom() const
{//@CODE_14308
    int value = 0;

    if (GetFromClassShape()->GetRect().bottom == GetStartPoint().y)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14308


bool ConnectionShape::IsFromAtLeft() const
{//@CODE_14309
    int value = 0;

    if (GetFromClassShape()->GetRect().left == GetStartPoint().x)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14309


bool ConnectionShape::IsFromAtRight() const
{//@CODE_14306
    int value = 0;

    if (GetFromClassShape()->GetRect().right == GetStartPoint().x)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14306


bool ConnectionShape::IsFromAtTop() const
{//@CODE_14310
    int value = 0;

    if (GetFromClassShape()->GetRect().top == GetStartPoint().y)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14310


bool ConnectionShape::IsToAtBottom() const
{//@CODE_14314
    int value = 0;

    if (GetToClassShape()->GetRect().bottom == GetEndPoint().y)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14314


bool ConnectionShape::IsToAtLeft() const
{//@CODE_14315
    int value = 0;

    if (GetToClassShape()->GetRect().left == GetEndPoint().x)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14315


bool ConnectionShape::IsToAtRight() const
{//@CODE_14316
    int value = 0;

    if (GetToClassShape()->GetRect().right == GetEndPoint().x)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14316


bool ConnectionShape::IsToAtTop() const
{//@CODE_14317
    int value = 0;

    if (GetToClassShape()->GetRect().top == GetEndPoint().y)
    {
        value = 1;
    }
    
    return value;
}//@CODE_14317


/*@NOTE_4502
Make a brand new routing, delete old routing first.
*/
void ConnectionShape::MakeNewRouting()
{//@CODE_4502
    ConnectionSegmentIterator iConnectionSegment(this);
    while (++iConnectionSegment)
    {
        iConnectionSegment->Delete();
    }
    
    CbSize size = (_endPoint - _startPoint) - _minStartSize + _minEndSize;
    
    if (GetMinStartSize().cy == 0)
    {
        if ((GetMinStartSize().cx < 0 && size.cx <= 0) ||
            (GetMinStartSize().cx > 0 && size.cx >= 0))
        {
            (void)new ConnectionSegment(this, GetMinStartSize()+CbSize(size.cx, 0));
        }
        else
        {
            CbSize extra(0, (size.cy/20)*10);
            // Check if we have crossing
            if ((GetMinEndSize().cy < 0 && size.cy < 0) ||
                (GetMinEndSize().cy > 0 && size.cy > 0))
            {
                extra.cy = size.cy;
            }
            size -= extra; // Compensation for the future
            
            (void)new ConnectionSegment(this, GetMinStartSize());
            (void)new ConnectionSegment(this, extra);
            (void)new ConnectionSegment(this, CbSize(size.cx, 0));
        }
    }
    else
    {
        (void)new ConnectionSegment(this, GetMinStartSize());
        (void)new ConnectionSegment(this, CbSize(size.cx, 0));
    }
    
    if (GetMinEndSize().cx == 0)
    {
        // Note we have to invert the MinEndSize
        if ((GetMinEndSize().cy > 0 && size.cy <= 0) ||
            (GetMinEndSize().cy < 0 && size.cy >= 0))
        {
            (void)new ConnectionSegment(this, CbSize(0, size.cy)-GetMinEndSize());
        }
        else
        {
            CbSize extra((size.cx/20)*10, 0);
            size -= extra; // Compensation for the past
        
            // Redo a bit of the past
            GetLastConnectionSegment()->Delete();
            if (GetConnectionSegmentCount())
            {
                (void)new ConnectionSegment(this, CbSize(size.cx, 0));
            }
            else
            {
                (void)new ConnectionSegment(this, GetMinStartSize()+CbSize(size.cx, 0));
            }
        
            (void)new ConnectionSegment(this, CbSize(0, size.cy));
            (void)new ConnectionSegment(this, extra);
            (void)new ConnectionSegment(this, -GetMinEndSize());
        }
    }
    else
    {
        if ((GetConnectionSegmentCount() == 2) &&
            ((GetMinStartSize().cy < 0 && size.cy <= 0) ||
             (GetMinStartSize().cy > 0 && size.cy >= 0)) &&
            ((GetMinEndSize().cx > 0 && size.cx <= 0) ||
             (GetMinEndSize().cx < 0 && size.cx >= 0)))
        {
            GetFirstConnectionSegment()->SetSize(GetMinStartSize()+CbSize(0, size.cy));
            GetLastConnectionSegment()->SetSize(CbSize(size.cx, 0)-GetMinEndSize());
        }
        else
        {
            (void)new ConnectionSegment(this, CbSize(0, size.cy));
            (void)new ConnectionSegment(this, -GetMinEndSize());
        }
    }
    
    if (GetConnectionSegmentCount() == 2)
    {
        // If only 2 segments and they are in the same axis add a third in between
        if ((GetMinStartSize().cx == 0 && GetMinEndSize().cx == 0) ||
            (GetMinStartSize().cy == 0 && GetMinEndSize().cy == 0))
        {
            ConnectionSegment* pSegment = new ConnectionSegment(this, CbSize(0, 0));
            MoveConnectionSegmentAfter(pSegment, GetFirstConnectionSegment());
        }
    }
    
    // If 3 segments put the second one in the middle
    if (GetConnectionSegmentCount() == 3)
    {
        ConnectionSegment* pSegment1 = GetFirstConnectionSegment();
        ConnectionSegment* pSegment2 = GetNextConnectionSegment(pSegment1);
        ConnectionSegment* pSegment3 = GetNextConnectionSegment(pSegment2);

        // Be sure to be in the correct situation
        if (((pSegment1->GetSize().cx == 0 && pSegment3->GetSize().cx == 0) ||
             (pSegment1->GetSize().cy == 0 && pSegment3->GetSize().cy == 0)) && 
            _minStartSize != _minEndSize && 
            (_minStartSize.cy >= 0 || _minEndSize.cy >= 0))
        {
            CbSize total = pSegment1->GetSize() + pSegment3->GetSize() - _minStartSize + _minEndSize;
            CbSize move((total.cx/20)*10, (total.cy/20)*10);
            if (move.cy == 0)
            {
                move = -move;
            }

            // Remember original initial, since move set it at false
            pSegment2->Move(move);
        }
    }
}//@CODE_4502


bool ConnectionShape::PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                   CbPoint pointLP)
{//@CODE_40844
    if (!GetHidden())
    {
        ConnectionSegmentIterator iConnectionSegment(this);
        while (++iConnectionSegment)
        {
            if (iConnectionSegment->PointInShape(pointLP))
            {
                return true;
            }
        }
    }

    return false;
}//@CODE_40844


/*@NOTE_4766
Recalculate the rectangle if something has changed.
*/
void ConnectionShape::RecalculateRect()
{//@CODE_4766
    CbRect rect(_startPoint, _endPoint);
    rect.NormalizeRect();
    CbPoint point = _startPoint;
    ConnectionSegmentIterator iConnectionSegment(this);
    while (++iConnectionSegment)
    {
        point += iConnectionSegment->GetSize();
        if (!rect.PtInRect(point))
        {
            if (point.x < rect.left)
                rect.left = point.x;
            if (point.x > rect.right)
                rect.right = point.x;
            if (point.y < rect.top)
                rect.top = point.y;
            if (point.y > rect.bottom)
                rect.bottom = point.y;
        }
    }
    Shape::SetRect(rect);
}//@CODE_4766


void ConnectionShape::ReplaceReference(ConnectionSegment* pOld,
                                       ConnectionSegment* pNew)
{//@CODE_41193
    DataModelDocObject::ReplaceReference(pOld, pNew);
    if (_firstConnectionSegment == pOld) _firstConnectionSegment = pNew;
    if (_lastConnectionSegment == pOld) _lastConnectionSegment = pNew;
}//@CODE_41193


/*@NOTE_6103
Unhide the ConnectionShape by removing it from the Hidden relation.
*/
void ConnectionShape::Show()
{//@CODE_6103
    if (GetHidden())
    {
        SaveState(1);
        GetHidden()->RemoveHidden(this);
    }
}//@CODE_6103


bool ConnectionShape::SlideEndpoint(bool atStart, CbPoint target,
                                    bool moveSiblings)
{//@CODE_40597
    ClassShape* pClass = atStart ? GetFromClassShape() : GetToClassShape();
    if (!pClass)
        return false;
    CbPoint snapped = Shape::TrackCrossPoint(pClass->GetRect(), target);
    CbPoint oldPoint = atStart ? GetStartPoint() : GetEndPoint();
    if (snapped == oldPoint)
        return false;

    ConnectionSegment* pSeg = atStart ? GetFirstConnectionSegment()
                                      : GetLastConnectionSegment();
    CbRect neighbourhood(pSeg->GetStartPoint(), pSeg->GetEndPoint());
    neighbourhood.NormalizeRect();
    neighbourhood.InflateRect(10, 10, 11, 11);
    CbSize offset = snapped - oldPoint;
    GetClassDiagram()->MoveNoteShapePoints(neighbourhood, offset);

    if (atStart && moveSiblings)
    {
        ClassShape::FromConnectionShapeIterator iFromConnectionShape(GetFromClassShape());
        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->GetStartPoint() == oldPoint)
            {
                iFromConnectionShape->UpdateStartPoint(snapped);
            }
        }
    }
    else if (atStart)
    {
        UpdateStartPoint(snapped);
    }
    else
    {
        UpdateEndPoint(snapped);
    }
    return true;
}//@CODE_40597


void ConnectionShape::UpdateEndPoint(const CbPoint& endPoint)
{//@CODE_4556
    CbSize oldMinEndSize = GetMinEndSize();
    CbSize size = endPoint - _endPoint;

    SetEndPoint(endPoint);
    SetInitial(false);
    
    // Check if change in direction, make sure it isn't only a change in compensation
    if (oldMinEndSize != GetMinEndSize() && 
        (oldMinEndSize.cy >= 0 || GetMinEndSize().cy >= 0))
    {
        MakeNewRouting();
    }
    else
    {
        if (GetConnectionSegmentCount() != CountNewRouting())
        {
            MakeNewRouting();
        }
        else
        {
            ConnectionSegment* pSegment1 = GetLastConnectionSegment();
            ConnectionSegment* pSegment2 = GetPrevConnectionSegment(pSegment1);

            CbSize segment1Size = pSegment1->GetSize();
            CbSize segment2Size = pSegment2->GetSize();

            // Deterine what to add where
            if (GetMinEndSize().cx != 0)
            {
                segment1Size += CbSize(size.cx, 0);
                segment2Size += CbSize(0, size.cy);
            }
            else
            {
                segment1Size += CbSize(0, size.cy);
                segment2Size += CbSize(size.cx, 0);
            }

            pSegment1->SetSize(segment1Size);
            pSegment2->SetSize(segment2Size);

            // If segment not long enough, make new routing
            int minSize = -(GetMinEndSize().cx + GetMinEndSize().cy);
            int actualSize = pSegment1->GetSize().cx + pSegment1->GetSize().cy;
            if ((minSize < 0 && minSize < actualSize) || 
                (minSize > 0 && minSize > actualSize))
            {
                MakeNewRouting();
            }
        }
    }
}//@CODE_4556


void ConnectionShape::UpdateStartPoint(const CbPoint& startPoint)
{//@CODE_4555
    CbSize oldMinStartSize = GetMinStartSize();
    CbSize size = _startPoint - startPoint;

    SetStartPoint(startPoint);
    SetInitial(false);
    
    // Check if change in direction, make sure it isn't only a change in compensation
    if (oldMinStartSize != GetMinStartSize() && 
        (oldMinStartSize.cy >= 0 || GetMinStartSize().cy >= 0))
    {
        MakeNewRouting();
    }
    else
    {
        if (GetConnectionSegmentCount() != CountNewRouting())
        {
            MakeNewRouting();
        }
        else
        {
            ConnectionSegment* pSegment1 = GetFirstConnectionSegment();
            ConnectionSegment* pSegment2 =
                pSegment1 ? GetNextConnectionSegment(pSegment1) : nullptr;

            // The count matched CountNewRouting() above, yet the segment list is
            // actually shorter than it claims (an inconsistent list left by an undo
            // restore -- count vs links disagree). Rebuild the routing rather than
            // dereference a missing segment (was a null-this AV in GetSize on undo).
            if (!pSegment1 || !pSegment2)
            {
                MakeNewRouting();
                return;
            }

            CbSize segment1Size = pSegment1->GetSize();
            CbSize segment2Size = pSegment2->GetSize();

            // Deterine what to add where
            if (GetMinStartSize().cx != 0)
            {
                segment1Size += CbSize(size.cx, 0);
                segment2Size += CbSize(0, size.cy);
            }
            else
            {
                segment1Size += CbSize(0, size.cy);
                segment2Size += CbSize(size.cx, 0);
            }

            pSegment1->SetSize(segment1Size);
            pSegment2->SetSize(segment2Size);

            // If segment not long enough, make new routing
            int minSize = GetMinStartSize().cx + GetMinStartSize().cy;
            int actualSize = pSegment1->GetSize().cx + pSegment1->GetSize().cy;
            if ((minSize < 0 && minSize < actualSize) || 
                (minSize > 0 && minSize > actualSize))
            {
                MakeNewRouting();
            }
        }
    }
}//@CODE_4555


/*@NOTE_4559
Returns the value of member '_endPoint'.
*/
const CbPoint& ConnectionShape::GetEndPoint() const
{//@CODE_4559
    return _endPoint;
}//@CODE_4559


/*@NOTE_4560
Set the value of member '_endPoint' to 'rEndPoint'.
*/
void ConnectionShape::SetEndPoint(const CbPoint& rEndPoint)
{//@CODE_4560
    SaveState();
    SetMinEndSize(GetMinSize(GetToClassShape()->GetRect(), rEndPoint));

    RelationShape* pRelationShape = GetRelationShape();
    if (pRelationShape)
    {
        pRelationShape->SaveState();
        pRelationShape->SetToNamePoint(pRelationShape->GetToNamePoint() + (rEndPoint - _endPoint));
        pRelationShape->SetToUmlPoint(pRelationShape->GetToUmlPoint() + (rEndPoint - _endPoint));
    }

    RelationDiagramOnlyShape* pRelationDiagramOnlyShape = GetRelationDiagramOnlyShape();
    if (pRelationDiagramOnlyShape)
    {
        pRelationDiagramOnlyShape->SetToNamePoint(pRelationDiagramOnlyShape->GetToNamePoint() + (rEndPoint - _endPoint));
        pRelationDiagramOnlyShape->SetToUmlPoint(pRelationDiagramOnlyShape->GetToUmlPoint() + (rEndPoint - _endPoint));
    }

    DependencyShape* pDependencyShape = GetDependencyShape();
    if (pDependencyShape)
    {
        pDependencyShape->SaveState();
        pDependencyShape->SetNamePoint(pDependencyShape->GetNamePoint() + (rEndPoint - _endPoint));
    }

    _endPoint = rEndPoint;
    RecalculateRect();
}//@CODE_4560


/*@NOTE_4552
Returns the value of member '_initial'.
*/
bool ConnectionShape::GetInitial()
{//@CODE_4552
    return _initial;
}//@CODE_4552


/*@NOTE_4553
Set the value of member '_initial' to 'initial'.
*/
void ConnectionShape::SetInitial(bool initial)
{//@CODE_4553
    _initial = initial;
}//@CODE_4553


/*@NOTE_4212
Returns the value of member '_minEndSize'.
*/
CbSize ConnectionShape::GetMinEndSize()
{//@CODE_4212
    return _minEndSize;
}//@CODE_4212


/*@NOTE_4213
Set the value of member '_minEndSize' to 'rMinEndSize'.
*/
void ConnectionShape::SetMinEndSize(const CbSize& rMinEndSize)
{//@CODE_4213
    _minEndSize = rMinEndSize;
}//@CODE_4213


/*@NOTE_4208
Returns the value of member '_minStartSize'.
*/
CbSize ConnectionShape::GetMinStartSize()
{//@CODE_4208
    return _minStartSize;
}//@CODE_4208


/*@NOTE_4209
Set the value of member '_minStartSize' to 'rMinStartSize'.
*/
void ConnectionShape::SetMinStartSize(const CbSize& rMinStartSize)
{//@CODE_4209
    _minStartSize = rMinStartSize;
}//@CODE_4209


/*@NOTE_4562
Returns the value of member '_startPoint'.
*/
const CbPoint& ConnectionShape::GetStartPoint() const
{//@CODE_4562
    return _startPoint;
}//@CODE_4562


/*@NOTE_4563
Set the value of member '_startPoint' to 'rStartPoint'.
*/
void ConnectionShape::SetStartPoint(const CbPoint& rStartPoint)
{//@CODE_4563
    SaveState();
    SetMinStartSize(GetMinSize(GetFromClassShape()->GetRect(), rStartPoint));

    RelationShape* pRelationShape = GetRelationShape();
    if (pRelationShape)
    {
        pRelationShape->SetFromNamePoint(pRelationShape->GetFromNamePoint() + (rStartPoint - _startPoint));
        pRelationShape->SetFromUmlPoint(pRelationShape->GetFromUmlPoint() + (rStartPoint - _startPoint));
    }

    RelationDiagramOnlyShape* pRelationDiagramOnlyShape = GetRelationDiagramOnlyShape();
    if (pRelationDiagramOnlyShape)
    {
        pRelationDiagramOnlyShape->SetFromNamePoint(pRelationDiagramOnlyShape->GetFromNamePoint() + (rStartPoint - _startPoint));
        pRelationDiagramOnlyShape->SetFromUmlPoint(pRelationDiagramOnlyShape->GetFromUmlPoint() + (rStartPoint - _startPoint));
    }

    DependencyShape* pDependencyShape = GetDependencyShape();
    if (pDependencyShape)
    {
        pDependencyShape->SaveState();
        pDependencyShape->SetStereotypePoint(pDependencyShape->GetStereotypePoint() + (rStartPoint - _startPoint));
    }
    
    _startPoint = rStartPoint;
    RecalculateRect();
}//@CODE_4563


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5732
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ConnectionShape::CleanupReferences()
{
    ClassDiagramShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    CLEANUP_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    CLEANUP_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
}


/*@NOTE_4192
Method which must be called first in a constructor.
*/
void ConnectionShape::ConstructorInclude(ClassShape* pFromClassShape,
                                         ClassShape* pToClassShape)
{
    INIT_MULTI_OWNED_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    INIT_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    INIT_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    INIT_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
}


/*@NOTE_4194
Method which must be called first in a destructor.
*/
void ConnectionShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    EXIT_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    EXIT_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    EXIT_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
}


/*@NOTE_5733
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ConnectionShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    ClassDiagramShape::RemoveReferences();
    REMOVE_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    REMOVE_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    REMOVE_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
}


/*@NOTE_5734
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ConnectionShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape* pConnectionShape = (ConnectionShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    RESTORE_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    RESTORE_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    ClassDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5736
Save the state of the current object relations to pDataModelDocObject.
*/
void ConnectionShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape::SaveReferences(pDataModelDocObject);
    ConnectionShape* pConnectionShape = (ConnectionShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    SAVE_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    SAVE_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
}


/*@NOTE_4197
Serialize the members only to a CbObject object.
*/
void ConnectionShape::Serialize(CbArchive& archive)
{
    ClassDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _startPoint;
        archive << _endPoint;
        archive << _minStartSize;
        archive << _minEndSize;
        archive << _initial;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _startPoint;
            archive >> _endPoint;
            archive >> _minStartSize;
            archive >> _minEndSize;
            archive >> _initial;
        }
    }
}


/*@NOTE_4196
Method which must be called first in a serialize constructor.
*/
void ConnectionShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    INIT_MULTI_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    INIT_MULTI_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    INIT_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
}


/*@NOTE_4199
Serialize the relations to a CbObject object.
*/
void ConnectionShape::SerializeRelations(CbArchive& archive,
                                         DataModelDocObject* pointerArray[])
{
    ClassDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
        }
    }
}


// ClassBuilder macro to support serialization for this class
// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
METHODS_ITERATOR_MULTI_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
METHODS_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
METHODS_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
METHODS_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
