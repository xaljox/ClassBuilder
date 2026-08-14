/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RelationSingleEndSegment.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RelationSingleEndSegment'
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


/*@NOTE_4451
Constructor needed for serialization, not meant to use for other purposes!
*/
RelationSingleEndSegment::RelationSingleEndSegment() //@INIT_4451
    : ConnectionSegment()
{//@CODE_4451
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4451


RelationSingleEndSegment::RelationSingleEndSegment(ConnectionSegment* pOld) //@INIT_4460
    : ConnectionSegment(pOld)
{//@CODE_4460
    ConstructorInclude();

    // Put in your own code
}//@CODE_4460


/*@NOTE_4449
Destructor method.
*/
RelationSingleEndSegment::~RelationSingleEndSegment()
{//@CODE_4449
    DestructorInclude();

    // Put in your own code
}//@CODE_4449


void RelationSingleEndSegment::Draw(CbPainter& painter)
{//@CODE_4462
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    painter.DrawLine(start, end);
}//@CODE_4462


CbPoint RelationSingleEndSegment::GetSelectedPoint()
{//@CODE_4530
    return GetEndPoint();
}//@CODE_4530


bool RelationSingleEndSegment::IsReplaced()
{//@CODE_41180
    return true;
}//@CODE_41180


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5762
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RelationSingleEndSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_4448
Method which must be called first in a constructor.
*/
void RelationSingleEndSegment::ConstructorInclude()
{
}


/*@NOTE_4450
Method which must be called first in a destructor.
*/
void RelationSingleEndSegment::DestructorInclude()
{
}


/*@NOTE_5763
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RelationSingleEndSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_5764
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RelationSingleEndSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5766
Save the state of the current object relations to pDataModelDocObject.
*/
void RelationSingleEndSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4453
Serialize the members only to a CbObject object.
*/
void RelationSingleEndSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4452
Method which must be called first in a serialize constructor.
*/
void RelationSingleEndSegment::SerializeConstructorInclude()
{
}


/*@NOTE_4455
Serialize the relations to a CbObject object.
*/
void RelationSingleEndSegment::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(RelationSingleEndSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
