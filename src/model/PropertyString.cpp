/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          PropertyString.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'PropertyString'
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


/*@NOTE_36208
Constructor needed for serialization, not meant to use for other purposes!
*/
PropertyString::PropertyString() //@INIT_36208
    : Property()
{//@CODE_36208
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_36208


/*@NOTE_36248
Constructor method.
*/
PropertyString::PropertyString(DataModelDocObject* pDataModelDocObject,
                               CbString name, CbString value) //@INIT_36248
    : Property(pDataModelDocObject, name)
    , _value(value)
{//@CODE_36248
    ConstructorInclude();

    // Put in your own code
}//@CODE_36248


/*@NOTE_36205
Destructor method.
*/
PropertyString::~PropertyString()
{//@CODE_36205
    DestructorInclude();

    // Put in your own code
}//@CODE_36205


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_36215
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void PropertyString::CleanupReferences()
{
    Property::CleanupReferences();
}


/*@NOTE_36206
Method which must be called first in a constructor.
*/
void PropertyString::ConstructorInclude()
{
}


/*@NOTE_36207
Method which must be called first in a destructor.
*/
void PropertyString::DestructorInclude()
{
}


/*@NOTE_36216
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void PropertyString::RemoveReferences()
{
    Property::RemoveReferences();
}


/*@NOTE_36217
Bring the current object relations into the same state as pDataModelDocObject.
*/
void PropertyString::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Property::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_36219
Save the state of the current object relations to pDataModelDocObject.
*/
void PropertyString::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Property::SaveReferences(pDataModelDocObject);
}


/*@NOTE_36210
Serialize the members only to a CbObject object.
*/
void PropertyString::Serialize(CbArchive& archive)
{
    Property::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _value;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _value;
        }
    }
}


/*@NOTE_36209
Method which must be called first in a serialize constructor.
*/
void PropertyString::SerializeConstructorInclude()
{
}


/*@NOTE_36212
Serialize the relations to a CbObject object.
*/
void PropertyString::SerializeRelations(CbArchive& archive,
                                        DataModelDocObject* pointerArray[])
{
    Property::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(PropertyString)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
