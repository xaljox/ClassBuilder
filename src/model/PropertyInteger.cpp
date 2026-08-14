/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          PropertyInteger.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'PropertyInteger'
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


/*@NOTE_36170
Constructor needed for serialization, not meant to use for other purposes!
*/
PropertyInteger::PropertyInteger() //@INIT_36170
    : Property()
{//@CODE_36170
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_36170


/*@NOTE_36240
Constructor method.
*/
PropertyInteger::PropertyInteger(DataModelDocObject* pDataModelDocObject,
                                 CbString name, int value) //@INIT_36240
    : Property(pDataModelDocObject, name)
    , _value(value)
{//@CODE_36240
    ConstructorInclude();

    // Put in your own code
}//@CODE_36240


/*@NOTE_36167
Destructor method.
*/
PropertyInteger::~PropertyInteger()
{//@CODE_36167
    DestructorInclude();

    // Put in your own code
}//@CODE_36167


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_36177
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void PropertyInteger::CleanupReferences()
{
    Property::CleanupReferences();
}


/*@NOTE_36168
Method which must be called first in a constructor.
*/
void PropertyInteger::ConstructorInclude()
{
}


/*@NOTE_36169
Method which must be called first in a destructor.
*/
void PropertyInteger::DestructorInclude()
{
}


/*@NOTE_36178
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void PropertyInteger::RemoveReferences()
{
    Property::RemoveReferences();
}


/*@NOTE_36179
Bring the current object relations into the same state as pDataModelDocObject.
*/
void PropertyInteger::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Property::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_36181
Save the state of the current object relations to pDataModelDocObject.
*/
void PropertyInteger::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Property::SaveReferences(pDataModelDocObject);
}


/*@NOTE_36172
Serialize the members only to a CbObject object.
*/
void PropertyInteger::Serialize(CbArchive& archive)
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


/*@NOTE_36171
Method which must be called first in a serialize constructor.
*/
void PropertyInteger::SerializeConstructorInclude()
{
}


/*@NOTE_36174
Serialize the relations to a CbObject object.
*/
void PropertyInteger::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(PropertyInteger)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
