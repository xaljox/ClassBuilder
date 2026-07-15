/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ConnectionSegment.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ConnectionSegment'
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
#include "MainFrm.h"
//@END_USER2


// Static members


ConnectionSegment::ConnectionSegment(ConnectionShape* pConnectionShape,
                                     const CbSize& size) //@INIT_4274
    : DataModelDocObject(pConnectionShape->GetDataModelDoc())
    , _size(size)
{//@CODE_4274
    ConstructorInclude(pConnectionShape);

    // Put in your own code
    _pUndoBase = GetDataModelDoc()->GetLastUndoBase();
    assert(_pUndoBase->GetDataModelDocObject() == this);
    
    // Only horizontal or vertical line segments are allowed.
    assert(size.cx == 0 || size.cy == 0);
}//@CODE_4274


ConnectionSegment::ConnectionSegment(ConnectionShape* pConnectionShape) //@INIT_4277
    : DataModelDocObject(pConnectionShape->GetDataModelDoc())
    , _size(0, 0)
{//@CODE_4277
    ConstructorInclude(pConnectionShape);

    // Put in your own code
    _pUndoBase = GetDataModelDoc()->GetLastUndoBase();
    assert(_pUndoBase->GetDataModelDocObject() == this);
}//@CODE_4277


/*@NOTE_4577
Copy constructor
*/
ConnectionSegment::ConnectionSegment(ConnectionShape* pConnectionShape,
                                     ConnectionSegment* pConnectionSegment) //@INIT_4577
    : DataModelDocObject(pConnectionSegment->GetDataModelDoc())
    , _size(pConnectionSegment->GetSize())
{//@CODE_4577
    ConstructorInclude(pConnectionShape);

    // Put in your own code
    _pUndoBase = GetDataModelDoc()->GetLastUndoBase();
    assert(_pUndoBase->GetDataModelDocObject() == this);
}//@CODE_4577


/*@NOTE_41161
Constructor needed for putting a new object in the old one's context.
*/
ConnectionSegment::ConnectionSegment(ConnectionSegment* pOld) //@INIT_41161
    : DataModelDocObject(pOld)
{//@CODE_41161
    ReplaceConstructorInclude(pOld);

    _size = pOld->_size;
    // base replace ctor already minted a fresh UndoNew for 'this'
    // we reuse pOld's entry instead, so drop the fresh one.
    delete GetDataModelDoc()->GetLastUndoBase();   // the base ctor's UndoNew_B
    _pUndoBase = pOld->_pUndoBase;
    _pUndoBase->ChangeDataModelDocObject(this);
    delete pOld;

    
    /*
    ConnectionShape* pConnectionShape = pOld->GetConnectionShape();
    ConstructorInclude(pConnectionShape);

    ConnectionSegment* pPrevConnectionSegment = 
        pConnectionShape->GetPrevConnectionSegment(pOld);
    if (pPrevConnectionSegment)
        pConnectionShape->MoveConnectionSegmentAfter(this, pPrevConnectionSegment);
    else
        pConnectionShape->MoveConnectionSegmentFirst(this);
        
    pOld->Delete();
    */
}//@CODE_41161


/*@NOTE_4219
Constructor needed for serialization, not meant to use for other purposes!
*/
ConnectionSegment::ConnectionSegment() //@INIT_4219
    : DataModelDocObject()
    , _pUndoBase(0)
{//@CODE_4219
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4219


/*@NOTE_4217
Destructor method.
*/
ConnectionSegment::~ConnectionSegment()
{//@CODE_4217
    DestructorInclude();

    // Put in your own code
}//@CODE_4217


bool ConnectionSegment::CanMove(const CbSize& size)
{//@CODE_40589
    ConnectionSegment* pPrev = GetConnectionShape()->GetPrevConnectionSegment(this);
    ConnectionSegment* pNext = GetConnectionShape()->GetNextConnectionSegment(this);
    if (!pPrev || !pNext)
        return false;
    if (!((size.cx == 0 && _size.cy == 0 && pPrev->_size.cx == 0 && pNext->_size.cx == 0) ||
          (size.cy == 0 && _size.cx == 0 && pPrev->_size.cy == 0 && pNext->_size.cy == 0)))
        return false;

    if (GetConnectionShape()->GetFirstConnectionSegment() == pPrev)
    {
        CbSize sizeNew = pPrev->_size + size;
        double x = GetConnectionShape()->GetMinStartSize().cx;
        double y = GetConnectionShape()->GetMinStartSize().cy;
        double minSize = x * x + y * y;
        double xNew = sizeNew.cx;
        double yNew = sizeNew.cy;
        double minSizeNew = xNew * xNew + yNew * yNew;
        if (minSizeNew < minSize ||
            (x < 0 && xNew > 0) || (x > 0 && xNew < 0) ||
            (y < 0 && yNew > 0) || (y > 0 && yNew < 0))
            return false;
    }
    if (GetConnectionShape()->GetLastConnectionSegment() == pNext)
    {
        CbSize sizeNew = pNext->_size - size;
        double x = GetConnectionShape()->GetMinEndSize().cx;
        double y = GetConnectionShape()->GetMinEndSize().cy;
        double minSize = x * x + y * y;
        double xNew = sizeNew.cx;
        double yNew = sizeNew.cy;
        double minSizeNew = xNew * xNew + yNew * yNew;
        if (minSizeNew < minSize ||
            (x < 0 && xNew < 0) || (x > 0 && xNew > 0) ||
            (y < 0 && yNew < 0) || (y > 0 && yNew > 0))
            return false;
    }
    return true;
}//@CODE_40589


void ConnectionSegment::Draw(CbPainter& painter)
{//@CODE_4411
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    painter.DrawLine(start, end);
}//@CODE_4411


void ConnectionSegment::DrawSelectedRect(CbPainter& painter, CbColorRef color)
{//@CODE_4524
    painter.DrawSelectionHandle(GetSelectedPoint(), color);
}//@CODE_4524


/*@NOTE_4279
Get the end point of this segment.
*/
CbPoint ConnectionSegment::GetEndPoint()
{//@CODE_4279
    return GetStartPoint() + _size;
}//@CODE_4279


CbPoint ConnectionSegment::GetLineEndPoint()
{//@CODE_40595
    return GetEndPoint();
}//@CODE_40595


CbPoint ConnectionSegment::GetLineStartPoint()
{//@CODE_40594
    return GetStartPoint();
}//@CODE_40594


CbPoint ConnectionSegment::GetSelectedPoint()
{//@CODE_4526
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    
    return CbPoint(start.x + (end.x - start.x)/2, 
                  start.y + (end.y - start.y)/2);

}//@CODE_4526


/*@NOTE_4280
Get the start point of this segment.
*/
CbPoint ConnectionSegment::GetStartPoint()
{//@CODE_4280
    ConnectionSegment* pConnectionSegment = 
        GetConnectionShape()->GetPrevConnectionSegment(this);

    if (pConnectionSegment)
        return pConnectionSegment->GetEndPoint();
    else
        return GetConnectionShape()->GetStartPoint();
}//@CODE_4280


bool ConnectionSegment::IsReplaced()
{//@CODE_41176
    return false;
}//@CODE_41176


/*@NOTE_4281
Move this segment by size, size may only be in the x or the y direction, not both. If it is
in the x direction this must be an y segment and if it is in the y direction it must be a x
segment. Size is added to the size member of the previous segment and substracted 
from the next segment. So the previous and the next segment must exist. If all goes well
true is returned, otherwise false is returned.
*/
bool ConnectionSegment::Move(const CbSize& size)
{//@CODE_4281
    bool value = false;

    ConnectionSegment* pPrev = GetConnectionShape()->GetPrevConnectionSegment(this);
    ConnectionSegment* pNext = GetConnectionShape()->GetNextConnectionSegment(this);
    if (pPrev && pNext)
    {
        if ((size.cx == 0 && _size.cy == 0 && pPrev->_size.cx == 0 && pNext->_size.cx == 0) ||
            (size.cy == 0 && _size.cx == 0 && pPrev->_size.cy == 0 && pNext->_size.cy == 0))
        {
            pPrev->SaveState();
            pNext->SaveState();

            bool minSizeOk = true;
            if (GetConnectionShape()->GetFirstConnectionSegment() == pPrev)
            {
                CbSize sizeNew = pPrev->_size + size;
                double x = GetConnectionShape()->GetMinStartSize().cx;
                double y = GetConnectionShape()->GetMinStartSize().cy;
                double minSize = x * x + y * y;

                double xNew = sizeNew.cx;
                double yNew = sizeNew.cy;
                double minSizeNew = xNew * xNew + yNew * yNew;

                if (minSizeNew < minSize ||
                    (x < 0 && xNew > 0) || (x > 0 && xNew < 0) ||
                    (y < 0 && yNew > 0) || (y > 0 && yNew < 0))
                {
                    minSizeOk = false;
                }
            }

            if (GetConnectionShape()->GetLastConnectionSegment() == pNext)
            {
                CbSize sizeNew = pNext->_size - size;
                double x = GetConnectionShape()->GetMinEndSize().cx;
                double y = GetConnectionShape()->GetMinEndSize().cy;
                double minSize = x * x + y * y;

                double xNew = sizeNew.cx;
                double yNew = sizeNew.cy;
                double minSizeNew = xNew * xNew + yNew * yNew;

                if (minSizeNew < minSize ||
                    (x < 0 && xNew < 0) || (x > 0 && xNew > 0) ||
                    (y < 0 && yNew < 0) || (y > 0 && yNew > 0))
                {
                    minSizeOk = false;
                }
            }

            if (minSizeOk)
            {
                pPrev->_size += size;
                pNext->_size -= size;

                value = true;
            }
        }
    }

    return value;
}//@CODE_4281


bool ConnectionSegment::MoveAndAdjust(const CbSize& delta,
                                      bool moveCollinearSiblings)
{//@CODE_40591
    CbRect neighbourhood(GetStartPoint(), GetEndPoint());
    neighbourhood.NormalizeRect();
    neighbourhood.InflateRect(10, 10, 11, 11);

    if (!Move(delta))
        return false;

    GetConnectionShape()->SaveState();
    GetConnectionShape()->SetInitial(false);
    GetConnectionShape()->GetClassDiagram()->MoveNoteShapePoints(neighbourhood, delta);

    if (moveCollinearSiblings && GetConnectionShape()->GetFromClassShape())
    {
        ClassShape::FromConnectionShapeIterator iFromConnectionShape(GetConnectionShape()->GetFromClassShape());
        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape.Get() != GetConnectionShape())
            {
                ConnectionShape::ConnectionSegmentIterator iRefConnectionSegment(GetConnectionShape());
                ConnectionShape::ConnectionSegmentIterator iConnectionSegment(iFromConnectionShape);
                while (++iRefConnectionSegment && ++iConnectionSegment)
                {
                    CbPoint refPoint = iRefConnectionSegment->GetStartPoint();
                    CbPoint point = iConnectionSegment->GetStartPoint();
                    if (refPoint != point)
                    {
                        if (iRefConnectionSegment.Get() == this && refPoint == (point + delta))
                        {
                            iConnectionSegment->Move(delta);
                            iConnectionSegment->GetConnectionShape()->SaveState();
                            iConnectionSegment->GetConnectionShape()->SetInitial(false);
                        }
                        break;
                    }
                }
            }
        }
    }
    return true;
}//@CODE_40591


bool ConnectionSegment::PointInShape(CbPoint pointLP)
{//@CODE_4495
    CbRect rect(pointLP, pointLP);
    rect.InflateRect(10, 10);
    CbPoint dummy(0, 0);

    return Shape::CrossPoint(GetStartPoint(), GetEndPoint(), rect, dummy);
}//@CODE_4495


void ConnectionSegment::ReplaceReference(ConnectionSegment* pOld,
                                         ConnectionSegment* pNew)
{//@CODE_41190
    DataModelDocObject::ReplaceReference(pOld, pNew);
    if (_prevConnectionShape == pOld) _prevConnectionShape = pNew;
    if (_nextConnectionShape == pOld) _nextConnectionShape = pNew;
}//@CODE_41190


bool ConnectionSegment::SetCursor(const CbRect& rect)
{//@CODE_4535
    // MFC-era hover-cursor decision. The Qt canvas reimplements this Qt-side
    // (see ClassDiagramQtView), so this only reports whether the point is on
    // this segment's drag handle; it no longer sets a view cursor.
    return rect.PtInRect(GetSelectedPoint());
}//@CODE_4535


/*@NOTE_4271
Returns the value of member '_size'.
*/
CbSize ConnectionSegment::GetSize()
{//@CODE_4271
    return _size;
}//@CODE_4271


/*@NOTE_4272
Set the value of member '_size' to 'rSize'.
*/
void ConnectionSegment::SetSize(const CbSize& rSize)
{//@CODE_4272
    if (_size != rSize)
    {
        SaveState();
        _size = rSize;
    }
}//@CODE_4272


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5738
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ConnectionSegment::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_4216
Method which must be called first in a constructor.
*/
void ConnectionSegment::ConstructorInclude(ConnectionShape* pConnectionShape)
{
    INIT_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_4218
Method which must be called first in a destructor.
*/
void ConnectionSegment::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_41486
Method that returns true if it is actually a DependencyEndSegment Object.
*/
bool ConnectionSegment::IsDependencyEndSegment() const
{
    return (dynamic_cast<const DependencyEndSegment*>(this) != nullptr);
}


/*@NOTE_41485
Method that returns true if it is actually a DependencyStartSegment Object.
*/
bool ConnectionSegment::IsDependencyStartSegment() const
{
    return (dynamic_cast<const DependencyStartSegment*>(this) != nullptr);
}


/*@NOTE_41484
Method that returns true if it is actually a InheritEndSegment Object.
*/
bool ConnectionSegment::IsInheritEndSegment() const
{
    return (dynamic_cast<const InheritEndSegment*>(this) != nullptr);
}


/*@NOTE_41483
Method that returns true if it is actually a InheritStartSegment Object.
*/
bool ConnectionSegment::IsInheritStartSegment() const
{
    return (dynamic_cast<const InheritStartSegment*>(this) != nullptr);
}


/*@NOTE_41479
Method that returns true if it is actually a RelationAggregationStartSegment Object.
*/
bool ConnectionSegment::IsRelationAggregationStartSegment() const
{
    return (dynamic_cast<const RelationAggregationStartSegment*>(this) != nullptr);
}


/*@NOTE_41480
Method that returns true if it is actually a RelationAssociationStartSegment Object.
*/
bool ConnectionSegment::IsRelationAssociationStartSegment() const
{
    return (dynamic_cast<const RelationAssociationStartSegment*>(this) != nullptr);
}


/*@NOTE_41481
Method that returns true if it is actually a RelationMultiEndSegment Object.
*/
bool ConnectionSegment::IsRelationMultiEndSegment() const
{
    return (dynamic_cast<const RelationMultiEndSegment*>(this) != nullptr);
}


/*@NOTE_41482
Method that returns true if it is actually a RelationSingleEndSegment Object.
*/
bool ConnectionSegment::IsRelationSingleEndSegment() const
{
    return (dynamic_cast<const RelationSingleEndSegment*>(this) != nullptr);
}


/*@NOTE_5739
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ConnectionSegment::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_41163
Method which must be called first in a replace constructor.
*/
void ConnectionSegment::ReplaceConstructorInclude(ConnectionSegment* pOld)
{
    REPLACE_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_5740
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ConnectionSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment* pConnectionSegment = (ConnectionSegment*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5742
Save the state of the current object relations to pDataModelDocObject.
*/
void ConnectionSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    ConnectionSegment* pConnectionSegment = (ConnectionSegment*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_4221
Serialize the members only to a CbObject object.
*/
void ConnectionSegment::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _size;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _size;
        }
    }
}


/*@NOTE_4220
Method which must be called first in a serialize constructor.
*/
void ConnectionSegment::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
}


/*@NOTE_4223
Serialize the relations to a CbObject object.
*/
void ConnectionSegment::SerializeRelations(CbArchive& archive,
                                           DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ConnectionSegment)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
