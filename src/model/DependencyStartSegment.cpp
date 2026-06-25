/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DependencyStartSegment.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DependencyStartSegment'
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


/*@NOTE_23317
Constructor needed for serialization, not meant to use for other purposes!
*/
DependencyStartSegment::DependencyStartSegment() //@INIT_23317
    : ConnectionSegment()
{//@CODE_23317
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23317


/*@NOTE_23362
Constructor method.
*/
DependencyStartSegment::DependencyStartSegment(ConnectionSegment* pOld) //@INIT_23362
    : ConnectionSegment(pOld)
{//@CODE_23362
    ConstructorInclude();

    // Put in your own code
}//@CODE_23362


/*@NOTE_23315
Destructor method.
*/
DependencyStartSegment::~DependencyStartSegment()
{//@CODE_23315
    DestructorInclude();

    // Put in your own code
}//@CODE_23315


void DependencyStartSegment::Draw(CbPainter& painter)
{//@CODE_23351
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    painter.DrawLine(start, end);
}//@CODE_23351


CbPoint DependencyStartSegment::GetSelectedPoint()
{//@CODE_23353
    return GetStartPoint();
}//@CODE_23353


bool DependencyStartSegment::IsReplaced()
{//@CODE_41183
    return true;
}//@CODE_41183


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_23324
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void DependencyStartSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_23314
Method which must be called first in a constructor.
*/
void DependencyStartSegment::ConstructorInclude()
{
}


/*@NOTE_23316
Method which must be called first in a destructor.
*/
void DependencyStartSegment::DestructorInclude()
{
}


/*@NOTE_23325
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void DependencyStartSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_23326
Bring the current object relations into the same state as pDataModelDocObject.
*/
void DependencyStartSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_23328
Save the state of the current object relations to pDataModelDocObject.
*/
void DependencyStartSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_23319
Serialize the members only to a CbObject object.
*/
void DependencyStartSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_23318
Method which must be called first in a serialize constructor.
*/
void DependencyStartSegment::SerializeConstructorInclude()
{
}


/*@NOTE_23321
Serialize the relations to a CbObject object.
*/
void DependencyStartSegment::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(DependencyStartSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
