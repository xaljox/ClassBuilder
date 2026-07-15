/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RootActivationShape.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RootActivationShape'
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
#ifndef _ROOTACTIVATIONSHAPE_H
#define _ROOTACTIVATIONSHAPE_H

//@START_USER1
//@END_USER1



class RootActivationShape
    : public ParentActivationShape
{
    CB_DECLARE_SERIAL(RootActivationShape)
    RELATION_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)

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
    RootActivationShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    RootActivationShape(SequenceDiagram* pSequenceDiagram);
    virtual ~RootActivationShape();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _ROOTACTIVATIONSHAPE_H_INLINES
#define _ROOTACTIVATIONSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
