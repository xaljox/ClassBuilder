/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Property.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Property'
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


/*@NOTE_36052
Constructor needed for serialization, not meant to use for other purposes!
*/
Property::Property() //@INIT_36052
    : DataModelDocObject()
{//@CODE_36052
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_36052


/*@NOTE_36223
Constructor method.
*/
Property::Property(DataModelDocObject* pDataModelDocObject,
                   CbString name) //@INIT_36223
    : DataModelDocObject(pDataModelDocObject->GetDataModelDoc())
    , _name(name)
{//@CODE_36223
    ConstructorInclude(pDataModelDocObject);

    // Put in your own code
}//@CODE_36223


/*@NOTE_36049
Destructor method.
*/
Property::~Property()
{//@CODE_36049
    DestructorInclude();

    // Put in your own code
}//@CODE_36049


/*@NOTE_36069
Set the value of member '_name' to 'rName'.
*/
void Property::SetName(const CbString& rName)
{//@CODE_36069
    if (_name != rName)
    {
        DataModelDocObject* refDataModelDocObject = _refDataModelDocObject;
        refDataModelDocObject->RemoveProperty(this);

        _name = rName;

        refDataModelDocObject->AddProperty(this);
    }
}//@CODE_36069


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_36059
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Property::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_36050
Method which must be called first in a constructor.
*/
void Property::ConstructorInclude(DataModelDocObject* pDataModelDocObject)
{
    INIT_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_36051
Method which must be called first in a destructor.
*/
void Property::DestructorInclude()
{
    EXIT_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_36060
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Property::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_36061
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Property::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Property* pProperty = (Property*)pDataModelDocObject;
    RESTORE_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_36063
Save the state of the current object relations to pDataModelDocObject.
*/
void Property::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    Property* pProperty = (Property*)pDataModelDocObject;
    SAVE_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_36054
Serialize the members only to a CbObject object.
*/
void Property::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
        }
    }
}


/*@NOTE_36053
Method which must be called first in a serialize constructor.
*/
void Property::SerializeConstructorInclude()
{
    INIT_AVLTREE_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)
}


/*@NOTE_36056
Serialize the relations to a CbObject object.
*/
void Property::SerializeRelations(CbArchive& archive,
                                  DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Property)


// Methods for the relation(s) of the class
METHODS_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
