/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          PropertyReal.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'PropertyReal'
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


/*@NOTE_36189
Constructor needed for serialization, not meant to use for other purposes!
*/
PropertyReal::PropertyReal() //@INIT_36189
    : Property()
{//@CODE_36189
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_36189


/*@NOTE_36244
Constructor method.
*/
PropertyReal::PropertyReal(DataModelDocObject* pDataModelDocObject,
                           CbString name, double value) //@INIT_36244
    : Property(pDataModelDocObject, name)
    , _value(value)
{//@CODE_36244
    ConstructorInclude();

    // Put in your own code
}//@CODE_36244


/*@NOTE_36186
Destructor method.
*/
PropertyReal::~PropertyReal()
{//@CODE_36186
    DestructorInclude();

    // Put in your own code
}//@CODE_36186


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_36196
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void PropertyReal::CleanupReferences()
{
    Property::CleanupReferences();
}


/*@NOTE_36187
Method which must be called first in a constructor.
*/
void PropertyReal::ConstructorInclude()
{
}


/*@NOTE_36188
Method which must be called first in a destructor.
*/
void PropertyReal::DestructorInclude()
{
}


/*@NOTE_36197
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void PropertyReal::RemoveReferences()
{
    Property::RemoveReferences();
}


/*@NOTE_36198
Bring the current object relations into the same state as pDataModelDocObject.
*/
void PropertyReal::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Property::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_36200
Save the state of the current object relations to pDataModelDocObject.
*/
void PropertyReal::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Property::SaveReferences(pDataModelDocObject);
}


/*@NOTE_36191
Serialize the members only to a CbObject object.
*/
void PropertyReal::Serialize(CbArchive& archive)
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


/*@NOTE_36190
Method which must be called first in a serialize constructor.
*/
void PropertyReal::SerializeConstructorInclude()
{
}


/*@NOTE_36193
Serialize the relations to a CbObject object.
*/
void PropertyReal::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(PropertyReal)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
