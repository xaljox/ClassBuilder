/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          NoteShapePoint.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'NoteShapePoint'
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
//@END_USER2


// Static members


NoteShapePoint::NoteShapePoint(NoteShape* pNoteShape,
                               CbPoint point) //@INIT_5064
    : DataModelDocObject(pNoteShape->GetDataModelDoc())
    , _point(point)
    , _pLastUndoBase((UndoBase*)-1)
{//@CODE_5064
    ConstructorInclude(pNoteShape);

    // Put in your own code
}//@CODE_5064


/*@NOTE_5013
Constructor needed for serialization, not meant to use for other purposes!
*/
NoteShapePoint::NoteShapePoint() //@INIT_5013
    : DataModelDocObject()
    , _pLastUndoBase((UndoBase*)-1)
{//@CODE_5013
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_5013


/*@NOTE_5011
Destructor method.
*/
NoteShapePoint::~NoteShapePoint()
{//@CODE_5011
    DestructorInclude();

    // Put in your own code
}//@CODE_5011


bool NoteShapePoint::IsNotJustMoved() const
{//@CODE_35407
    return (GetDataModelDoc()->GetLastMarked() != _pLastUndoBase);
}//@CODE_35407


void NoteShapePoint::MarkAsJustMoved()
{//@CODE_35409
    _pLastUndoBase = GetDataModelDoc()->GetLastMarked();
}//@CODE_35409


bool NoteShapePoint::PointInShape(CbPoint pointLP)
{//@CODE_27321
    CbRect rect(pointLP, pointLP);
    rect.InflateRect(10, 10);
    CbPoint dummy(0, 0);

    return Shape::CrossPoint(GetNoteShape()->GetRect().CenterPoint(), 
        GetPoint(), rect, dummy);
}//@CODE_27321


/*@NOTE_4996
Returns the value of member '_point'.
*/
const CbPoint& NoteShapePoint::GetPoint()
{//@CODE_4996
    return _point;
}//@CODE_4996


/*@NOTE_4997
Set the value of member '_point' to 'rPoint'.
*/
void NoteShapePoint::SetPoint(const CbPoint& rPoint)
{//@CODE_4997
    if (_point != rPoint)
    {
        CbRect rect = GetNoteShape()->GetRect();
        
        if (GetNoteShape()->GetLastNoteShapePoint() == this)
        {
            SaveState();
            if (rect.PtInRect(rPoint) || rPoint == rect.BottomRight())
            {
                _point = rect.BottomRight();
            }
            else
            {
                _point = rPoint;
                (void)new NoteShapePoint(GetNoteShape(), rect.BottomRight());
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
}//@CODE_4997


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5804
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void NoteShapePoint::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5010
Method which must be called first in a constructor.
*/
void NoteShapePoint::ConstructorInclude(NoteShape* pNoteShape)
{
    INIT_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5012
Method which must be called first in a destructor.
*/
void NoteShapePoint::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5805
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void NoteShapePoint::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5806
Bring the current object relations into the same state as pDataModelDocObject.
*/
void NoteShapePoint::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    NoteShapePoint* pNoteShapePoint = (NoteShapePoint*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5808
Save the state of the current object relations to pDataModelDocObject.
*/
void NoteShapePoint::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    NoteShapePoint* pNoteShapePoint = (NoteShapePoint*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5015
Serialize the members only to a CbObject object.
*/
void NoteShapePoint::Serialize(CbArchive& archive)
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


/*@NOTE_5014
Method which must be called first in a serialize constructor.
*/
void NoteShapePoint::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)
}


/*@NOTE_5017
Serialize the relations to a CbObject object.
*/
void NoteShapePoint::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(NoteShapePoint)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
