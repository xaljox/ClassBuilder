/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SDNoteShapePoint.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SDNoteShapePoint'
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
//@END_USER2


// Static members


/*@NOTE_34679
Constructor method.
*/
SDNoteShapePoint::SDNoteShapePoint(SDNoteShape* pSDNoteShape,
                                   const CbPoint& point) //@INIT_34679
    : DataModelDocObject(pSDNoteShape->GetDataModelDoc())
    , _point(point)
    , _pLastUndoBase((UndoBase*)-1)
{//@CODE_34679
    ConstructorInclude(pSDNoteShape);

    // Put in your own code
}//@CODE_34679


/*@NOTE_34557
Constructor needed for serialization, not meant to use for other purposes!
*/
SDNoteShapePoint::SDNoteShapePoint() //@INIT_34557
    : DataModelDocObject()
    , _pLastUndoBase((UndoBase*)-1)
{//@CODE_34557
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_34557


/*@NOTE_34555
Destructor method.
*/
SDNoteShapePoint::~SDNoteShapePoint()
{//@CODE_34555
    DestructorInclude();

    // Put in your own code
}//@CODE_34555


bool SDNoteShapePoint::IsNotJustMoved() const
{//@CODE_35406
    return (!GetSDNoteShape()->GetSequenceDiagram()->GetMoveOnce() || 
        GetDataModelDoc()->GetLastMarked() != _pLastUndoBase);
}//@CODE_35406


void SDNoteShapePoint::MarkAsJustMoved()
{//@CODE_35408
    _pLastUndoBase = GetDataModelDoc()->GetLastMarked();
}//@CODE_35408


bool SDNoteShapePoint::PointInShape(const CbPoint& pointLP)
{//@CODE_34732
    CbRect rect(pointLP, pointLP);
    rect.InflateRect(10, 10);
    CbPoint dummy(0, 0);

    return Shape::CrossPoint(GetSDNoteShape()->GetRect().CenterPoint(), 
        GetPoint(), rect, dummy);
}//@CODE_34732


/*@NOTE_34676
Returns the value of member '_point'.
*/
const CbPoint& SDNoteShapePoint::GetPoint()
{//@CODE_34676
    return _point;
}//@CODE_34676


/*@NOTE_34677
Set the value of member '_point' to 'rPoint'.
*/
void SDNoteShapePoint::SetPoint(const CbPoint& rPoint)
{//@CODE_34677
    if (_point != rPoint)
    {
        CbRect rect = GetSDNoteShape()->GetRect();

        if (GetSDNoteShape()->GetLastSDNoteShapePoint() == this)
        {
            SaveState();
            if (rect.PtInRect(rPoint) || rPoint == rect.BottomRight())
            {
                _point = rect.BottomRight();
            }
            else
            {
                _point = rPoint;
                (void)new SDNoteShapePoint(GetSDNoteShape(), rect.BottomRight());
            }
        }
        else
        {
            if (rect.PtInRect(rPoint) || rPoint == rect.BottomRight())
            {
                Delete();
            }
            else
            {
                SaveState();
                _point = rPoint;
            }
        }
    }
}//@CODE_34677


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_34564
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SDNoteShapePoint::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34554
Method which must be called first in a constructor.
*/
void SDNoteShapePoint::ConstructorInclude(SDNoteShape* pSDNoteShape)
{
    INIT_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34556
Method which must be called first in a destructor.
*/
void SDNoteShapePoint::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34565
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SDNoteShapePoint::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34566
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SDNoteShapePoint::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SDNoteShapePoint* pSDNoteShapePoint = (SDNoteShapePoint*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_34568
Save the state of the current object relations to pDataModelDocObject.
*/
void SDNoteShapePoint::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    SDNoteShapePoint* pSDNoteShapePoint = (SDNoteShapePoint*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34559
Serialize the members only to a CbObject object.
*/
void SDNoteShapePoint::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _point;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _point;
        }
    }
}


/*@NOTE_34558
Method which must be called first in a serialize constructor.
*/
void SDNoteShapePoint::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)
}


/*@NOTE_34561
Serialize the relations to a CbObject object.
*/
void SDNoteShapePoint::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(SDNoteShapePoint)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
