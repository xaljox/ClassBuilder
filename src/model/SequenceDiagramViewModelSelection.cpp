/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SequenceDiagramViewModelSelection.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SequenceDiagramViewModelSelection'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
//@END_USER2


// Static members


/*@NOTE_39807
Constructor method.
*/
SequenceDiagramViewModelSelection::SequenceDiagramViewModelSelection(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                                                     SequenceDiagramShape* pSequenceDiagramShape) //@INIT_39807
{//@CODE_39807
    ConstructorInclude(pSequenceDiagramViewModel, pSequenceDiagramShape);

    // Put in your own code
}//@CODE_39807


/*@NOTE_39580
Destructor method.
*/
SequenceDiagramViewModelSelection::~SequenceDiagramViewModelSelection()
{//@CODE_39580
    DestructorInclude();

    // Put in your own code
}//@CODE_39580


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_39581
Method which must be called first in a constructor.
*/
void SequenceDiagramViewModelSelection::ConstructorInclude(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                                           SequenceDiagramShape* pSequenceDiagramShape)
{
    INIT_MULTI_OWNED_PASSIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
    INIT_MULTI_OWNED_PASSIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
}


/*@NOTE_39582
Method which must be called first in a destructor.
*/
void SequenceDiagramViewModelSelection::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
    EXIT_MULTI_OWNED_PASSIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(SequenceDiagramViewModel, SequenceDiagramViewModel, SequenceDiagramViewModelSelection, Selected)
METHODS_MULTI_OWNED_PASSIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
