/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassDiagramViewModel.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassDiagramViewModel'
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


/*@NOTE_40005
Constructor method.
*/
ClassDiagramViewModel::ClassDiagramViewModel(ClassDiagram* pClassDiagram,
                                             RefreshCallback refreshFn,
                                             RefreshCallback closeFn,
                                             void* refreshCtx) //@INIT_40005
    : _closeFn(closeFn)
    , _refreshCtx(refreshCtx)
    , _refreshFn(refreshFn)
{//@CODE_40005
    ConstructorInclude(pClassDiagram);

    // Put in your own code
}//@CODE_40005


/*@NOTE_39840
Destructor method.
*/
ClassDiagramViewModel::~ClassDiagramViewModel()
{//@CODE_39840
    DestructorInclude();

    // Put in your own code
    if (_closeFn) _closeFn(_refreshCtx);
}//@CODE_39840


void ClassDiagramViewModel::Refresh() const
{//@CODE_40004
    if (_refreshFn)
    {
        _refreshFn(_refreshCtx); 
    }
}//@CODE_40004


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_39841
Method which must be called first in a constructor.
*/
void ClassDiagramViewModel::ConstructorInclude(ClassDiagram* pClassDiagram)
{
    INIT_MULTI_OWNED_ACTIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
    INIT_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
}


/*@NOTE_39842
Method which must be called first in a destructor.
*/
void ClassDiagramViewModel::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
    EXIT_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
METHODS_ITERATOR_MULTI_ACTIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
METHODS_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
