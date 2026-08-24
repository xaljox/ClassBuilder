/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Column.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Column'
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


/*@NOTE_11963
Constructor method.
*/
Column::Column(Grid* pGrid, bool first) //@INIT_11963
    : _column(pGrid->GetColumnCount())
    , _width(0)
{//@CODE_11963
    ConstructorInclude(pGrid);

    if (first)
    {
        pGrid->MoveColumnFirst(this);
        Grid::RowIterator iRow(pGrid);
        while (++iRow)
        {
            iRow->MoveGridPointFirst(new GridPoint(this, iRow));
        }
        pGrid->RenumberColumns();
    }
    else
    {
        Grid::RowIterator iRow(pGrid);
        while (++iRow)
        {
            (void)new GridPoint(this, iRow);
        }
    }
}//@CODE_11963


/*@NOTE_11281
Destructor method.
*/
Column::~Column()
{//@CODE_11281
    Grid* pGrid = GetGrid();
    DestructorInclude();

    // Put in your own code
    pGrid->RenumberColumns();
}//@CODE_11281


/*@NOTE_12001
Returns the value of member '_width'.
*/
int Column::GetWidth()
{//@CODE_12001
    if (_width == 0)
    {
        GridPointIterator iGridPoint(this);
        while (++iGridPoint)
        {
            if (iGridPoint->GetGridObject())
            {
                CbRect rect = iGridPoint->GetGridObject()->GetClassShape()->GetRect();
                if (_width < rect.Width())
                {
                    _width = rect.Width();
                }
            }
        }
    }
    
    return _width;
}//@CODE_12001


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_11280
Method which must be called first in a constructor.
*/
void Column::ConstructorInclude(Grid* pGrid)
{
    INIT_MULTI_OWNED_ACTIVE(Column, Column, GridPoint, GridPoint)
    INIT_MULTI_OWNED_PASSIVE(Grid, Grid, Column, Column)
}


/*@NOTE_11282
Method which must be called first in a destructor.
*/
void Column::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Column, Column, GridPoint, GridPoint)
    EXIT_MULTI_OWNED_PASSIVE(Grid, Grid, Column, Column)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Column, Column, GridPoint, GridPoint)
METHODS_ITERATOR_MULTI_ACTIVE(Column, Column, GridPoint, GridPoint)
METHODS_MULTI_OWNED_PASSIVE(Grid, Grid, Column, Column)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
