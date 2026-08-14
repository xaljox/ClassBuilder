/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          GridPoint.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'GridPoint'
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


/*@NOTE_11975
Constructor method.
*/
GridPoint::GridPoint(Column* pColumn, Row* pRow) //@INIT_11975
{//@CODE_11975
    ConstructorInclude(pColumn, pRow);

    // Put in your own code
}//@CODE_11975


/*@NOTE_11319
Destructor method.
*/
GridPoint::~GridPoint()
{//@CODE_11319
    DestructorInclude();

    // Put in your own code
}//@CODE_11319


bool GridPoint::Swap(GridPoint* pGridPoint)
{//@CODE_11978
    bool value = false;
    
    GridObject* pGridObjectThis = GetGridObject();
    GridObject* pGridObject = pGridPoint->GetGridObject();
    
    if (pGridObjectThis)
    {
        value = true;
        
        if (pGridObject)
        {
            RemoveGridObject(pGridObjectThis);
            pGridPoint->RemoveGridObject(pGridObject);
            AddGridObject(pGridObject);
            pGridPoint->AddGridObject(pGridObjectThis);
        }
        else
        {
            RemoveGridObject(pGridObjectThis);
            pGridPoint->AddGridObject(pGridObjectThis);
        }
    }
    else if (pGridObject)
    {
        value = true;
        
        pGridPoint->RemoveGridObject(pGridObject);
        AddGridObject(pGridObject);
    }
    
    return value;
}//@CODE_11978


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_11318
Method which must be called first in a constructor.
*/
void GridPoint::ConstructorInclude(Column* pColumn, Row* pRow)
{
    INIT_SINGLE_ACTIVE(GridPoint, GridPoint, GridObject, GridObject)
    INIT_MULTI_OWNED_PASSIVE(Column, Column, GridPoint, GridPoint)
    INIT_MULTI_OWNED_PASSIVE(Row, Row, GridPoint, GridPoint)
}


/*@NOTE_11320
Method which must be called first in a destructor.
*/
void GridPoint::DestructorInclude()
{
    EXIT_SINGLE_ACTIVE(GridPoint, GridPoint, GridObject, GridObject)
    EXIT_MULTI_OWNED_PASSIVE(Column, Column, GridPoint, GridPoint)
    EXIT_MULTI_OWNED_PASSIVE(Row, Row, GridPoint, GridPoint)
}


// Methods for the relation(s) of the class
METHODS_SINGLE_ACTIVE(GridPoint, GridPoint, GridObject, GridObject)
METHODS_MULTI_OWNED_PASSIVE(Column, Column, GridPoint, GridPoint)
METHODS_MULTI_OWNED_PASSIVE(Row, Row, GridPoint, GridPoint)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
