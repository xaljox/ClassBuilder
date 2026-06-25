/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ActorLifeLineShape.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ActorLifeLineShape'
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
#ifndef _ACTORLIFELINESHAPE_H
#define _ACTORLIFELINESHAPE_H

//@START_USER1
//@END_USER1



class ActorLifeLineShape
    : public LifeLineShape
{
    CB_DECLARE_SERIAL(ActorLifeLineShape)
    RELATION_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    int RecalculateRectWidth();
    void ConstructorInclude(Actor* pActor);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ActorLifeLineShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ActorLifeLineShape(SequenceDiagram* pSequenceDiagram, Actor* pActor,
                       const CbPoint& point);
    virtual ~ActorLifeLineShape();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected);
    virtual ActorLifeLineShape* GetActorLifeLine();
    CbRect GetActorRect();
    virtual CbRect GetBoundingRect();
    virtual CbString GetTypeName();
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual bool PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              const CbPoint& pointLP);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _ACTORLIFELINESHAPE_H_INLINES
#define _ACTORLIFELINESHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
