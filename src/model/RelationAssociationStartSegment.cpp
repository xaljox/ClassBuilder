/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationAssociationStartSegment.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RelationAssociationStartSegment'
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


/*@NOTE_4417
Constructor needed for serialization, not meant to use for other purposes!
*/
RelationAssociationStartSegment::RelationAssociationStartSegment() //@INIT_4417
    : ConnectionSegment()
{//@CODE_4417
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4417


RelationAssociationStartSegment::RelationAssociationStartSegment(ConnectionSegment* pOld) //@INIT_4426
    : ConnectionSegment(pOld)
{//@CODE_4426
    ConstructorInclude();

    // Put in your own code
}//@CODE_4426


/*@NOTE_4415
Destructor method.
*/
RelationAssociationStartSegment::~RelationAssociationStartSegment()
{//@CODE_4415
    DestructorInclude();

    // Put in your own code
}//@CODE_4415


void RelationAssociationStartSegment::Draw(CbPainter& painter)
{//@CODE_4428
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    painter.DrawLine(start, end);
}//@CODE_4428


CbPoint RelationAssociationStartSegment::GetSelectedPoint()
{//@CODE_4527
    return GetStartPoint();
}//@CODE_4527


bool RelationAssociationStartSegment::IsReplaced()
{//@CODE_41178
    return true;
}//@CODE_41178


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5750
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RelationAssociationStartSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_4414
Method which must be called first in a constructor.
*/
void RelationAssociationStartSegment::ConstructorInclude()
{
}


/*@NOTE_4416
Method which must be called first in a destructor.
*/
void RelationAssociationStartSegment::DestructorInclude()
{
}


/*@NOTE_5751
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RelationAssociationStartSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_5752
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RelationAssociationStartSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5754
Save the state of the current object relations to pDataModelDocObject.
*/
void RelationAssociationStartSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4419
Serialize the members only to a CbObject object.
*/
void RelationAssociationStartSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4418
Method which must be called first in a serialize constructor.
*/
void RelationAssociationStartSegment::SerializeConstructorInclude()
{
}


/*@NOTE_4421
Serialize the relations to a CbObject object.
*/
void RelationAssociationStartSegment::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(RelationAssociationStartSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
