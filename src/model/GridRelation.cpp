/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          GridRelation.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'GridRelation'
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


/*@NOTE_11991
Constructor method.
*/
GridRelation::GridRelation(GridObject* pFromGridObject,
                           GridObject* pToGridObject, double weight,
                           double directionWeight) //@INIT_11991
    : _directionWeight(directionWeight)
    , _weight(weight)
{//@CODE_11991
    ConstructorInclude(pFromGridObject, pToGridObject);

    // Put in your own code
}//@CODE_11991


/*@NOTE_11357
Destructor method.
*/
GridRelation::~GridRelation()
{//@CODE_11357
    DestructorInclude();

    // Put in your own code
}//@CODE_11357


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_11356
Method which must be called first in a constructor.
*/
void GridRelation::ConstructorInclude(GridObject* pFromGridObject,
                                      GridObject* pToGridObject)
{
    INIT_MULTI_OWNED_PASSIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
    INIT_MULTI_OWNED_PASSIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
}


/*@NOTE_11358
Method which must be called first in a destructor.
*/
void GridRelation::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
    EXIT_MULTI_OWNED_PASSIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
METHODS_MULTI_OWNED_PASSIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
