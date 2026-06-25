/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SequenceDiagramViewModel.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SequenceDiagramViewModel'
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


/*@NOTE_39818
Constructor method.
*/
SequenceDiagramViewModel::SequenceDiagramViewModel(SequenceDiagram* pSequenceDiagram,
                                                   RefreshCallback refreshFn,
                                                   RefreshCallback closeFn,
                                                   void* refreshCtx) //@INIT_39818
    : _refreshFn(refreshFn)
    , _refreshCtx(refreshCtx)
    , _closeFn(closeFn)
{//@CODE_39818
    ConstructorInclude(pSequenceDiagram);

    // Put in your own code
}//@CODE_39818


/*@NOTE_39463
Destructor method.
*/
SequenceDiagramViewModel::~SequenceDiagramViewModel()
{//@CODE_39463
    DestructorInclude();

    // Put in your own code
    if (_closeFn) _closeFn(_refreshCtx);
}//@CODE_39463


void SequenceDiagramViewModel::Refresh() const
{//@CODE_39826
    if (_refreshFn)
    {
        _refreshFn(_refreshCtx); 
    }
}//@CODE_39826


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_39464
Method which must be called first in a constructor.
*/
void SequenceDiagramViewModel::ConstructorInclude(SequenceDiagram* pSequenceDiagram)
{
    INIT_MULTI_OWNED_ACTIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
    INIT_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
}


/*@NOTE_39465
Method which must be called first in a destructor.
*/
void SequenceDiagramViewModel::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
    EXIT_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
METHODS_ITERATOR_MULTI_ACTIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
METHODS_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
