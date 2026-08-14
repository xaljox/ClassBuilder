/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Actors.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Actors'
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


/*@NOTE_33629
Constructor needed for serialization, not meant to use for other purposes!
*/
Actors::Actors() //@INIT_33629
    : Gti()
{//@CODE_33629
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_33629


/*@NOTE_34003
Constructor method.
*/
Actors::Actors(DataModelDoc* pDataModelDoc) //@INIT_34003
    : Gti(pDataModelDoc)
{//@CODE_34003
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_34003


/*@NOTE_33627
Destructor method.
*/
Actors::~Actors()
{//@CODE_33627
    DestructorInclude();

    // Put in your own code
}//@CODE_33627


void Actors::Add()
{//@CODE_33646
    if (!GetAdded())
    {
        SetItemText(CbString("Actors"));
        SetIcon(ICON_FILE);

        Gti::Add();

        DataModelDoc::ActorIterator iActor(GetDataModelDoc());
        while (++iActor)
            iActor->Add();
    }
}//@CODE_33646


Gti* Actors::GetNext(Gti* pGti)
{//@CODE_35379
    Gti* pNextGti = Gti::GetNext(pGti);

    if (!pNextGti)
    {
        pNextGti = GetDataModelDoc()->GetExternClasses();
    }

    return pNextGti;
}//@CODE_35379


void Actors::Update()
{//@CODE_33647
    if (GetAdded())
    {
        Gti::Update();
    }
}//@CODE_33647


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_33636
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Actors::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
}


/*@NOTE_33626
Method which must be called first in a constructor.
*/
void Actors::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
}


/*@NOTE_33628
Method which must be called first in a destructor.
*/
void Actors::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
}


/*@NOTE_33637
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Actors::RemoveReferences()
{
    Gti::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
}


/*@NOTE_33638
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Actors::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Actors* pActors = (Actors*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_33640
Save the state of the current object relations to pDataModelDocObject.
*/
void Actors::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    Actors* pActors = (Actors*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
}


/*@NOTE_33631
Serialize the members only to a CbObject object.
*/
void Actors::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_33630
Method which must be called first in a serialize constructor.
*/
void Actors::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)
}


/*@NOTE_33633
Serialize the relations to a CbObject object.
*/
void Actors::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(Actors)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
