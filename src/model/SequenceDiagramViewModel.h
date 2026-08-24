/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SequenceDiagramViewModel.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SequenceDiagramViewModel'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
