/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationMultiEndSegment.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RelationMultiEndSegment'
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


/*@NOTE_4434
Constructor needed for serialization, not meant to use for other purposes!
*/
RelationMultiEndSegment::RelationMultiEndSegment() //@INIT_4434
    : ConnectionSegment()
{//@CODE_4434
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4434


RelationMultiEndSegment::RelationMultiEndSegment(ConnectionSegment* pOld) //@INIT_4443
    : ConnectionSegment(pOld)
{//@CODE_4443
    ConstructorInclude();

    // Put in your own code
}//@CODE_4443


/*@NOTE_4432
Destructor method.
*/
RelationMultiEndSegment::~RelationMultiEndSegment()
{//@CODE_4432
    DestructorInclude();

    // Put in your own code
}//@CODE_4432


void RelationMultiEndSegment::Draw(CbPainter& painter)
{//@CODE_4445
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    painter.DrawLine(start, end);
}//@CODE_4445


CbPoint RelationMultiEndSegment::GetSelectedPoint()
{//@CODE_4529
    return GetEndPoint();
}//@CODE_4529


bool RelationMultiEndSegment::IsReplaced()
{//@CODE_41179
    return true;
}//@CODE_41179


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5756
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RelationMultiEndSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_4431
Method which must be called first in a constructor.
*/
void RelationMultiEndSegment::ConstructorInclude()
{
}


/*@NOTE_4433
Method which must be called first in a destructor.
*/
void RelationMultiEndSegment::DestructorInclude()
{
}


/*@NOTE_5757
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RelationMultiEndSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_5758
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RelationMultiEndSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5760
Save the state of the current object relations to pDataModelDocObject.
*/
void RelationMultiEndSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4436
Serialize the members only to a CbObject object.
*/
void RelationMultiEndSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4435
Method which must be called first in a serialize constructor.
*/
void RelationMultiEndSegment::SerializeConstructorInclude()
{
}


/*@NOTE_4438
Serialize the relations to a CbObject object.
*/
void RelationMultiEndSegment::SerializeRelations(CbArchive& archive,
                                                 DataModelDocObject* pointerArray[])
{
    ConnectionSegment::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(RelationMultiEndSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
