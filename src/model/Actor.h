/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Actor.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Actor'
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
#ifndef _ACTOR_H
#define _ACTOR_H

//@START_USER1
//@END_USER1



class Actor
    : public Gti
{
    CB_DECLARE_SERIAL(Actor)
    RELATION_MULTI_OWNED_ACTIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actor, Actor)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;
    CbString _note;

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Actor();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Actor(DataModelDoc* pDataModelDoc);
    virtual ~Actor();
    virtual void Add();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void Update();
    const CbString& GetName() const;
    void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _ACTOR_H_INLINES
#define _ACTOR_H_INLINES

/*@NOTE_33752
Returns the value of member '_name'.
*/
inline const CbString& Actor::GetName() const
{//@CODE_33752
    return _name;
}//@CODE_33752



/*@NOTE_33753
Set the value of member '_name' to 'rName'.
*/
inline void Actor::SetName(const CbString& rName)
{//@CODE_33753
    _name = rName;
}//@CODE_33753



//@START_USER3
//@END_USER3

#endif
#endif
