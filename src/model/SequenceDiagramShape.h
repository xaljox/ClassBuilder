/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SequenceDiagramShape.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SequenceDiagramShape'
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
#ifndef _SEQUENCEDIAGRAMSHAPE_H
#define _SEQUENCEDIAGRAMSHAPE_H

//@START_USER1
//@END_USER1



class SequenceDiagramShape
    : public Shape
{
    RELATION_MULTI_OWNED_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
    RELATION_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(SequenceDiagram* pSequenceDiagram);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    SequenceDiagramShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SequenceDiagramShape(SequenceDiagram* pSequenceDiagram,
                         const CbPoint& point, CbColorRef penColor = Cb_RGB(0,
                         0, 0), CbColorRef textColor = Cb_RGB(0, 0, 0));
    SequenceDiagramShape(SequenceDiagram* pSequenceDiagram,
                         CbColorRef penColor = Cb_RGB(0, 0, 0),
                         CbColorRef textColor = Cb_RGB(0, 0, 0));
    virtual ~SequenceDiagramShape();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram) = 0;
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected) = 0;
    SequenceDiagramViewModelSelection* FindSequenceDiagramViewModelSelection(SequenceDiagramViewModel* pSequenceDiagramViewModel);
    virtual ActorLifeLineShape* GetActorLifeLine();
    virtual CbRect GetBoundingRect();
    virtual ChildActivationShape* GetChildActivation();
    virtual ClassLifeLineShape* GetClassLifeLine();
    virtual SequenceDiagramShape* GetHitShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                              const CbPoint& pointLP,
                                              bool nested);
    virtual LifeLineShape* GetLifeLine();
    virtual SDNoteShape* GetNoteShape();
    virtual SequenceDiagramShape* GetOuterSequenceDiagramShape();
    virtual SignalShape* GetSignal();
    int IsSelectedIn(SequenceDiagramViewModel* pSequenceDiagramViewModel);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false) = 0;
    virtual int OnOpen(bool checkOnly = false) = 0;
    virtual bool PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              const CbPoint& pointLP);
    virtual int UsesPenColor() const;
    virtual int UsesTextColor() const;
    virtual void CleanupReferences();
    bool IsActorLifeLineShape() const;
    bool IsChildActivationShape() const;
    bool IsClassLifeLineShape() const;
    bool IsLifeLineShape() const;
    bool IsParentActivationShape() const;
    bool IsRootActivationShape() const;
    bool IsSDNoteShape() const;
    bool IsSignalShape() const;
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SEQUENCEDIAGRAMSHAPE_H_INLINES
#define _SEQUENCEDIAGRAMSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
