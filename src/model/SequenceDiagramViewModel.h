/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SequenceDiagramViewModel.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SequenceDiagramViewModel'
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
#ifndef _SEQUENCEDIAGRAMVIEWMODEL_H
#define _SEQUENCEDIAGRAMVIEWMODEL_H

//@START_USER1
//@END_USER1



class SequenceDiagramViewModel
{
    RELATION_MULTI_OWNED_ACTIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
    RELATION_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)

//@START_USER2
//@END_USER2

// Members
private:
    RefreshCallback _refreshFn;
    void* _refreshCtx;
    RefreshCallback _closeFn;

protected:

public:

// Methods
private:
    void ConstructorInclude(SequenceDiagram* pSequenceDiagram);
    void DestructorInclude();

protected:

public:
    SequenceDiagramViewModel(SequenceDiagram* pSequenceDiagram,
                             RefreshCallback refreshFn, RefreshCallback closeFn,
                             void* refreshCtx);
    virtual ~SequenceDiagramViewModel();
    void Refresh() const;
};

#endif


#ifdef CB_INLINES
#ifndef _SEQUENCEDIAGRAMVIEWMODEL_H_INLINES
#define _SEQUENCEDIAGRAMVIEWMODEL_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
