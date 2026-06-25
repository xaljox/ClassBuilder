/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          InheritStartSegment.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'InheritStartSegment'
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


/*@NOTE_4468
Constructor needed for serialization, not meant to use for other purposes!
*/
InheritStartSegment::InheritStartSegment() //@INIT_4468
    : ConnectionSegment()
{//@CODE_4468
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4468


InheritStartSegment::InheritStartSegment(ConnectionSegment* pOld) //@INIT_4477
    : ConnectionSegment(pOld)
{//@CODE_4477
    ConstructorInclude();

    // Put in your own code
}//@CODE_4477


/*@NOTE_4466
Destructor method.
*/
InheritStartSegment::~InheritStartSegment()
{//@CODE_4466
    DestructorInclude();

    // Put in your own code
}//@CODE_4466


void InheritStartSegment::Draw(CbPainter& painter)
{//@CODE_4479
    CbPoint start = GetStartPoint();

    const int size = 30;
    CbSize a(0,0);
    CbSize b(0,0);

    if (GetSize().cx == 0)
    {
        if (GetSize().cy < 0)
        {
            a.cy = -size;
            b.cx = size/2;
        }
        else
        {
            a.cy = size;
            b.cx = size/2;
        }
    }
    else
    {
        if (GetSize().cx < 0)
        {
            a.cx = -size;
            b.cy = size/2;
        }
        else
        {
            a.cx = size;
            b.cy = size/2;
        }
    }

    painter.DrawLine(start, start+a-b);
    painter.DrawLine(start, start+a+b);
    painter.DrawLine(start+a-b, start+a+b);
    painter.DrawLine(start+a, start+GetSize());
}//@CODE_4479


CbPoint InheritStartSegment::GetLineStartPoint()
{//@CODE_40596
    CbPoint start = GetStartPoint();
    const int size = 30;
    CbSize a(0, 0);
    if (GetSize().cx == 0)
        a.cy = (GetSize().cy < 0) ? -size : size;
    else
        a.cx = (GetSize().cx < 0) ? -size : size;
    return start + a;
}//@CODE_40596


CbPoint InheritStartSegment::GetSelectedPoint()
{//@CODE_4532
    return GetStartPoint();
}//@CODE_4532


bool InheritStartSegment::IsReplaced()
{//@CODE_41181
    return true;
}//@CODE_41181


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5768
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void InheritStartSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_4465
Method which must be called first in a constructor.
*/
void InheritStartSegment::ConstructorInclude()
{
}


/*@NOTE_4467
Method which must be called first in a destructor.
*/
void InheritStartSegment::DestructorInclude()
{
}


/*@NOTE_5769
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void InheritStartSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_5770
Bring the current object relations into the same state as pDataModelDocObject.
*/
void InheritStartSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5772
Save the state of the current object relations to pDataModelDocObject.
*/
void InheritStartSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4470
Serialize the members only to a CbObject object.
*/
void InheritStartSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4469
Method which must be called first in a serialize constructor.
*/
void InheritStartSegment::SerializeConstructorInclude()
{
}


/*@NOTE_4472
Serialize the relations to a CbObject object.
*/
void InheritStartSegment::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(InheritStartSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
