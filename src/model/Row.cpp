/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Row.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Row'
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


/*@NOTE_11971
Constructor method.
*/
Row::Row(Grid* pGrid, bool first) //@INIT_11971
    : _row(pGrid->GetRowCount())
    , _height(0)
{//@CODE_11971
    ConstructorInclude(pGrid);

    if (first)
    {
        pGrid->MoveRowFirst(this);
        Grid::ColumnIterator iColumn(pGrid);
        while (++iColumn)
        {
            iColumn->MoveGridPointFirst(new GridPoint(iColumn, this));
        }
        pGrid->RenumberRows();
    }
    else
    {
        Grid::ColumnIterator iColumn(pGrid);
        while (++iColumn)
        {
            (void)new GridPoint(iColumn, this);
        }
    }
}//@CODE_11971


/*@NOTE_11300
Destructor method.
*/
Row::~Row()
{//@CODE_11300
    Grid* pGrid = GetGrid();
    DestructorInclude();

    // Put in your own code
    pGrid->RenumberRows();
}//@CODE_11300


/*@NOTE_12003
Returns the value of member '_height'.
*/
int Row::GetHeight()
{//@CODE_12003
    if (_height == 0)
    {
        GridPointIterator iGridPoint(this);
        while (++iGridPoint)
        {
            if (iGridPoint->GetGridObject())
            {
                CbRect rect = iGridPoint->GetGridObject()->GetClassShape()->GetRect();
                if (_height < rect.Height())
                {
                    _height = rect.Height();
                }
            }
        }
    }

    return _height;
}//@CODE_12003


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_11299
Method which must be called first in a constructor.
*/
void Row::ConstructorInclude(Grid* pGrid)
{
    INIT_MULTI_OWNED_ACTIVE(Row, Row, GridPoint, GridPoint)
    INIT_MULTI_OWNED_PASSIVE(Grid, Grid, Row, Row)
}


/*@NOTE_11301
Method which must be called first in a destructor.
*/
void Row::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Row, Row, GridPoint, GridPoint)
    EXIT_MULTI_OWNED_PASSIVE(Grid, Grid, Row, Row)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Row, Row, GridPoint, GridPoint)
METHODS_ITERATOR_MULTI_ACTIVE(Row, Row, GridPoint, GridPoint)
METHODS_MULTI_OWNED_PASSIVE(Grid, Grid, Row, Row)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
