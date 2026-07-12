/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DependencyEndSegment.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DependencyEndSegment'
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


/*@NOTE_23336
Constructor needed for serialization, not meant to use for other purposes!
*/
DependencyEndSegment::DependencyEndSegment() //@INIT_23336
    : ConnectionSegment()
{//@CODE_23336
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23336


/*@NOTE_23365
Constructor method.
*/
DependencyEndSegment::DependencyEndSegment(ConnectionSegment* pOld) //@INIT_23365
    : ConnectionSegment(pOld)
{//@CODE_23365
    ConstructorInclude();

    // Put in your own code
}//@CODE_23365


/*@NOTE_23334
Destructor method.
*/
DependencyEndSegment::~DependencyEndSegment()
{//@CODE_23334
    DestructorInclude();

    // Put in your own code
}//@CODE_23334


void DependencyEndSegment::Draw(CbPainter& painter)
{//@CODE_23368
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();

    const int size = 20;
    CbSize a(0,0);
    CbSize b(0,0);

    if (GetSize().cx == 0)
    {
        if (GetSize().cy > 0)
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
        if (GetSize().cx > 0)
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

    CbSize c(a.cx/2, a.cy/2);

    painter.DrawLine(end+c, start);

    // Arrowhead stays solid even when the dependency line is dotted.
    painter.Save();
    painter.SetPenSolid();   // solid arrowhead, but keep the caller's pen colour + width
    painter.DrawLine(end+a, end);
    painter.DrawLine(end, end+a-b);
    painter.DrawLine(end, end+a+b);
    painter.Restore();
}//@CODE_23368


CbPoint DependencyEndSegment::GetLineEndPoint()
{//@CODE_40602
    CbPoint start = GetStartPoint();
    CbPoint end = start + GetSize();
    const int size = 20;
    CbSize a(0, 0);
    if (GetSize().cx == 0)
        a.cy = (GetSize().cy > 0) ? -size : size;
    else
        a.cx = (GetSize().cx > 0) ? -size : size;
    return end + a;
}//@CODE_40602


CbPoint DependencyEndSegment::GetSelectedPoint()
{//@CODE_23370
    return GetEndPoint();
}//@CODE_23370


bool DependencyEndSegment::IsReplaced()
{//@CODE_41184
    return true;
}//@CODE_41184


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_23343
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void DependencyEndSegment::CleanupReferences()
{
    ConnectionSegment::CleanupReferences();
}


/*@NOTE_23333
Method which must be called first in a constructor.
*/
void DependencyEndSegment::ConstructorInclude()
{
}


/*@NOTE_23335
Method which must be called first in a destructor.
*/
void DependencyEndSegment::DestructorInclude()
{
}


/*@NOTE_23344
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void DependencyEndSegment::RemoveReferences()
{
    ConnectionSegment::RemoveReferences();
}


/*@NOTE_23345
Bring the current object relations into the same state as pDataModelDocObject.
*/
void DependencyEndSegment::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_23347
Save the state of the current object relations to pDataModelDocObject.
*/
void DependencyEndSegment::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionSegment::SaveReferences(pDataModelDocObject);
}


/*@NOTE_23338
Serialize the members only to a CbObject object.
*/
void DependencyEndSegment::Serialize(CbArchive& archive)
{
    ConnectionSegment::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_23337
Method which must be called first in a serialize constructor.
*/
void DependencyEndSegment::SerializeConstructorInclude()
{
}


/*@NOTE_23340
Serialize the relations to a CbObject object.
*/
void DependencyEndSegment::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(DependencyEndSegment)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
