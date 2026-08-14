/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          OtherTypes.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'OtherTypes'
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


OtherTypes::OtherTypes(DataModelDoc* pDataModelDoc) //@INIT_957
    : Gti(pDataModelDoc)
{//@CODE_957
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_957


/*@NOTE_347
Constructor needed for serialization, not meant to use for other purposes!
*/
OtherTypes::OtherTypes() //@INIT_347
    : Gti()
{//@CODE_347
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_347


/*@NOTE_345
Destructor method
*/
OtherTypes::~OtherTypes()
{//@CODE_345
    DestructorInclude();

    // Put in your own code
}//@CODE_345


void OtherTypes::Add()
{//@CODE_959
    if (!GetAdded())
    {
        SetItemText(CbString("Other Types"));
        SetIcon(ICON_FILE);

        Gti::Add();

        DataModelDoc::TypeIterator otherType(GetDataModelDoc(), &Type::IsOtherType);
        while (++otherType)
            otherType->Add();
    }
}//@CODE_959


void OtherTypes::Update()
{//@CODE_960
    if (GetAdded())
    {
        Gti::Update();
    }
}//@CODE_960


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5504
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void OtherTypes::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
}


/*@NOTE_344
Method which must be called first in a constructor
*/
void OtherTypes::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
}


/*@NOTE_346
Method which must be called first in a destructor
*/
void OtherTypes::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
}


/*@NOTE_5505
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void OtherTypes::RemoveReferences()
{
    Gti::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
}


/*@NOTE_5506
Bring the current object relations into the same state as pDataModelDocObject.
*/
void OtherTypes::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    OtherTypes* pOtherTypes = (OtherTypes*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5508
Save the state of the current object relations to pDataModelDocObject.
*/
void OtherTypes::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    OtherTypes* pOtherTypes = (OtherTypes*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
}


/*@NOTE_349
Serialize the members only to a CbObject object
*/
void OtherTypes::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_348
Method which must be called first in a serialize constructor
*/
void OtherTypes::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
}


/*@NOTE_351
Serialize the relations to a CbObject object
*/
void OtherTypes::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(OtherTypes)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
