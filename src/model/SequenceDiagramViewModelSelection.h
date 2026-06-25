/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SequenceDiagramViewModelSelection.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SequenceDiagramViewModelSelection'
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
#ifndef _SEQUENCEDIAGRAMVIEWMODELSELECTION_H
#define _SEQUENCEDIAGRAMVIEWMODELSELECTION_H

//@START_USER1
//@END_USER1



class SequenceDiagramViewModelSelection
{
    RELATION_MULTI_OWNED_PASSIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
    RELATION_MULTI_OWNED_PASSIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                            SequenceDiagramShape* pSequenceDiagramShape);
    void DestructorInclude();

protected:

public:
    SequenceDiagramViewModelSelection(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                      SequenceDiagramShape* pSequenceDiagramShape);
    virtual ~SequenceDiagramViewModelSelection();
};

#endif


#ifdef CB_INLINES
#ifndef _SEQUENCEDIAGRAMVIEWMODELSELECTION_H_INLINES
#define _SEQUENCEDIAGRAMVIEWMODELSELECTION_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
