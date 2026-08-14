/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Group.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Group'
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
#include "qt/QtGroupDialog.h"
//@END_USER2


// Static members


Group::Group(DataModelDoc* refDataModelDoc) //@INIT_1019
    : Gti(refDataModelDoc)
    , _name()
    , _note()
{//@CODE_1019
    ConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_1019


/*@NOTE_516
Constructor needed for serialization, not meant to use for other purposes!
*/
Group::Group() //@INIT_516
    : Gti()
{//@CODE_516
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_516


/*@NOTE_514
Destructor method
*/
Group::~Group()
{//@CODE_514
    DestructorInclude();

    // Put in your own code
}//@CODE_514


int Group::OnEditAttributes(bool checkOnly)
{//@CODE_1021
    if (checkOnly)
        return 1;
    
    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowGroupDialog(this, modelChanged, ownerHwnd))
    {
        if (modelChanged)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_1021


/*@NOTE_4785
Returns the value of member '_name'.
*/
const CbString& Group::GetName()
{//@CODE_4785
    return _name;
}//@CODE_4785


/*@NOTE_4786
Set the value of member '_name' to 'rName'.
*/
void Group::SetName(const CbString& rName)
{//@CODE_4786
    _name = rName;
}//@CODE_4786


const CbString& Group::GetNote()
{//@CODE_1175
    return _note;
}//@CODE_1175


void Group::SetNote(const CbString& rNote)
{//@CODE_1176
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_1176


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5444
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Group::CleanupReferences()
{
    Gti::CleanupReferences();
}


/*@NOTE_513
Method which must be called first in a constructor
*/
void Group::ConstructorInclude()
{
}


/*@NOTE_515
Method which must be called first in a destructor
*/
void Group::DestructorInclude()
{
}


/*@NOTE_5445
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Group::RemoveReferences()
{
    Gti::RemoveReferences();
}


/*@NOTE_5446
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Group::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5448
Save the state of the current object relations to pDataModelDocObject.
*/
void Group::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
}


/*@NOTE_518
Serialize the members only to a CbObject object
*/
void Group::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
        archive << _note;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
            archive >> _note;
        }
    }
}


/*@NOTE_517
Method which must be called first in a serialize constructor
*/
void Group::SerializeConstructorInclude()
{
}


/*@NOTE_520
Serialize the relations to a CbObject object
*/
void Group::SerializeRelations(CbArchive& archive,
                               DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Group)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
