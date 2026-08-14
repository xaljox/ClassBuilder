/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Actor.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Actor'
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
#include "qt/QtActorDialog.h"
//@END_USER2


// Static members


/*@NOTE_33654
Constructor needed for serialization, not meant to use for other purposes!
*/
Actor::Actor() //@INIT_33654
    : Gti()
{//@CODE_33654
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_33654


/*@NOTE_33904
Constructor method.
*/
Actor::Actor(DataModelDoc* pDataModelDoc) //@INIT_33904
    : Gti(pDataModelDoc)
    , _name("")
    , _note("")
{//@CODE_33904
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_33904


/*@NOTE_33652
Destructor method.
*/
Actor::~Actor()
{//@CODE_33652
    DestructorInclude();

    // Put in your own code
}//@CODE_33652


void Actor::Add()
{//@CODE_33763
    if (!GetAdded())
    {
        SaveState(1);
        GetDataModelDoc()->GetActors()->AddChildLast(this);
        SetItemText(GetName());
        SetIcon(ICON_ACTOR);

        Gti::Add();
    }
}//@CODE_33763


bool Actor::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_34005
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = true;
    }
    else
    {
    }

    return value;
}//@CODE_34005


void Actor::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_34011
    if (ctrlKeyDown)
    {
        ClassDiagram* pClassDiagram = dynamic_cast<ClassDiagram*>(pGtiDrop);
        SequenceDiagram* pSequenceDiagram = dynamic_cast<SequenceDiagram*>(pGtiDrop);
        if (pSequenceDiagram)
        {
            int lastRight = 80;
            if (pSequenceDiagram->GetLastLifeLineShape())
            {
                lastRight = pSequenceDiagram->GetLastLifeLineShape()->GetRect().right;
            }
            CbPoint point(lastRight + 20, 0);
            Shape::Round(point);
            (void)new ActorLifeLineShape(pSequenceDiagram, this, point);
        }
    }
    else
    {
    }
}//@CODE_34011


bool Actor::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_34008
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
        return value;

    if (ctrlKeyDown)
    {
        if (pGtiDrop->IsSequenceDiagram())
        {
            value = true;
        }
    }
    else
    {
    }

    return value;
}//@CODE_34008


int Actor::OnDelete(bool checkOnly)
{//@CODE_33764
    if (!checkOnly)
    {
        CbString str;
        str.Format("Are you sure you want to delete actor '%s'", GetName().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            Delete();
        }
    }
    
    return 1;
}//@CODE_33764


int Actor::OnEditAttributes(bool checkOnly)
{//@CODE_33766
    if (checkOnly)
        return 1;
            
    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowActorDialog(this, modelChanged, ownerHwnd))
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
}//@CODE_33766


/*@NOTE_33749
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Actor::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_33749
    Gti::OnUndoRedoChanged(pOldState);

    Actor* pActor = (Actor*)pOldState;

    if (pActor && pActor->GetName() != GetName())
    {
        Update();
    }
}//@CODE_33749


void Actor::Update()
{//@CODE_33768
    if (GetAdded())
    {
        SetItemText(GetName());

        Gti::Update();
    }
}//@CODE_33768


/*@NOTE_33760
Returns the value of member '_note'.
*/
const CbString& Actor::GetNote()
{//@CODE_33760
    return _note;
}//@CODE_33760


/*@NOTE_33761
Set the value of member '_note' to 'rNote'.
*/
void Actor::SetNote(const CbString& rNote)
{//@CODE_33761
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_33761


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_33661
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Actor::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
}


/*@NOTE_33651
Method which must be called first in a constructor.
*/
void Actor::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
}


/*@NOTE_33653
Method which must be called first in a destructor.
*/
void Actor::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
}


/*@NOTE_33662
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Actor::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    Gti::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
}


/*@NOTE_33663
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Actor::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Actor* pActor = (Actor*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_33665
Save the state of the current object relations to pDataModelDocObject.
*/
void Actor::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    Actor* pActor = (Actor*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
}


/*@NOTE_33656
Serialize the members only to a CbObject object.
*/
void Actor::Serialize(CbArchive& archive)
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


/*@NOTE_33655
Method which must be called first in a serialize constructor.
*/
void Actor::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)
}


/*@NOTE_33658
Serialize the relations to a CbObject object.
*/
void Actor::SerializeRelations(CbArchive& archive,
                               DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Actor)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
METHODS_ITERATOR_MULTI_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
