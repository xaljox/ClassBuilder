/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassDiagramViewModelSelection.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassDiagramViewModelSelection'
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


/*@NOTE_40233
Constructor method.
*/
ClassDiagramViewModelSelection::ClassDiagramViewModelSelection(ClassDiagramViewModel* pClassDiagramViewModel,
                                                               ClassDiagramShape* pClassDiagramShape) //@INIT_40233
{//@CODE_40233
    ConstructorInclude(pClassDiagramViewModel, pClassDiagramShape);

    // Put in your own code
}//@CODE_40233


/*@NOTE_39859
Destructor method.
*/
ClassDiagramViewModelSelection::~ClassDiagramViewModelSelection()
{//@CODE_39859
    DestructorInclude();

    // Put in your own code
}//@CODE_39859


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_39860
Method which must be called first in a constructor.
*/
void ClassDiagramViewModelSelection::ConstructorInclude(ClassDiagramViewModel* pClassDiagramViewModel,
                                                        ClassDiagramShape* pClassDiagramShape)
{
    INIT_MULTI_OWNED_PASSIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
    INIT_MULTI_OWNED_PASSIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
}


/*@NOTE_39861
Method which must be called first in a destructor.
*/
void ClassDiagramViewModelSelection::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
    EXIT_MULTI_OWNED_PASSIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
METHODS_MULTI_OWNED_PASSIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
