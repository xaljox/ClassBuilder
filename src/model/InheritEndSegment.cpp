/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          InheritEndSegment.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'InheritEndSegment'
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


/*@NOTE_4511
Constructor needed for serialization, not meant to use for other purposes!
*/
InheritEndSegment::InheritEndSegment() //@INIT_4511
    : ConnectionSegment()
{//@CODE_4511
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4511


InheritEndSegment::InheritEndSegment(ConnectionSegment* pOld) //@INIT_4520
    : ConnectionSegment(pOld)
{//@CODE_4520
    ConstructorInclude();

    // Put in your own code
}//@CODE_4520


/*@NOTE_4509
Destructor method.
*/
InheritEndSegment::~InheritEndSegment()
{//@CODE_4509
    DestructorInclude();

    // Put in your own code
}//@CODE_4509


void InheritEndSegment::Draw(CbPainter& painter)
{//@CODE_4522
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    painter.DrawLine(start, end);
}//@CODE_4522


CbPoint InheritEndSegment::GetSelectedPoint()
{//@CODE_4531
    return GetEndPoint();
}//@CODE_4531


bool InheritEndSegment::IsReplaced()
{//@CODE_41182
    return true;
}//@CODE_41182


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5774
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void InheritEndSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_4508
Method which must be called first in a constructor.
*/
void InheritEndSegment::ConstructorInclude()
{
}


/*@NOTE_4510
Method which must be called first in a destructor.
*/
void InheritEndSegment::DestructorInclude()
{
}


/*@NOTE_5775
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void InheritEndSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_5776
Bring the current object relations into the same state as pDataModelDocObject.
*/
void InheritEndSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5778
Save the state of the current object relations to pDataModelDocObject.
*/
void InheritEndSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4513
Serialize the members only to a CbObject object.
*/
void InheritEndSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4512
Method which must be called first in a serialize constructor.
*/
void InheritEndSegment::SerializeConstructorInclude()
{
}


/*@NOTE_4515
Serialize the relations to a CbObject object.
*/
void InheritEndSegment::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(InheritEndSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
