/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          GridObject.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'GridObject'
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
#include <math.h>
//@END_USER2


// Static members


/*@NOTE_11982
Constructor method.
*/
GridObject::GridObject(Grid* pGrid, ClassShape* pClassShape) //@INIT_11982
    : _pClassShape(pClassShape)
{//@CODE_11982
    ConstructorInclude(pGrid);

    // Put in your own code
    
}//@CODE_11982


/*@NOTE_11338
Destructor method.
*/
GridObject::~GridObject()
{//@CODE_11338
    DestructorInclude();

    // Put in your own code
}//@CODE_11338


int GridObject::Compare(GridObject* a, GridObject* b)
{//@CODE_19875
	int rowMid = a->GetGrid()->GetLastRow()->GetRow()/2;
	int columnMid = a->GetGrid()->GetLastColumn()->GetColumn()/2;

	int aVal = ((a->GetRow()-rowMid)*(a->GetRow()-rowMid)) +
		((a->GetColumn()-columnMid)*(a->GetColumn()-columnMid));
	int bVal = ((b->GetRow()-rowMid)*(b->GetRow()-rowMid)) +
		((b->GetColumn()-columnMid)*(b->GetColumn()-columnMid));

	return aVal - bVal;

	/*
	int value = a->GetColumn() - b->GetColumn();
    if (value == 0)
    {
        value = a->GetRow() - b->GetRow();
    }

	return value;
	*/
}//@CODE_19875


double GridObject::Evaluate()
{//@CODE_11985
    double value = 0;
    
    if (GetGridPoint())
    {
        // If it is the first or last row
        if (GetGrid()->GetFirstRow() == GetGridPoint()->GetRow() ||
            GetGrid()->GetLastRow() == GetGridPoint()->GetRow() )
        {
            value += 0.04; // Add penalty for making the grid bigger
            
            // Try to make grid square
            if (GetGrid()->GetRowCount() > GetGrid()->GetColumnCount())
            {
                value += 0.02; 
            }
        }
        
        // If it is the first or last column
        if (GetGrid()->GetFirstColumn() == GetGridPoint()->GetColumn() ||
            GetGrid()->GetLastColumn() == GetGridPoint()->GetColumn() )
        {
            value += 0.05; // Add penalty for making the grid bigger
            
            // Try to make grid square
            if (GetGrid()->GetColumnCount() > GetGrid()->GetRowCount())
            {
                value += 0.01; 
            }
        }
        
        FromGridRelationIterator iFromGridRelation(this);
        while (++iFromGridRelation)
        {
            GridObject* pGridObject = iFromGridRelation->GetToGridObject();
            if (pGridObject->GetGridPoint())
            {
                int x = pGridObject->GetColumn() - GetColumn();
                int y = GetRow() - pGridObject->GetRow();
                double length = sqrt(x*x+y*y);
                
                int dirY = y;
                if (dirY < -1)
                    dirY = -1;
                else if (dirY > 1)
                    dirY = 1;
                
                value += (2.0*sqrt(length) + iFromGridRelation->GetDirectionWeight()*dirY)*iFromGridRelation->GetWeight();
            }
        }
    }
    
    return value;
}//@CODE_11985


int GridObject::GetColumn()
{//@CODE_11987
    return GetGridPoint()->GetColumn()->GetColumn();
}//@CODE_11987


int GridObject::GetRow()
{//@CODE_11988
    return GetGridPoint()->GetRow()->GetRow();
}//@CODE_11988


void GridObject::PutOnGridPoint()
{//@CODE_11986
	GridPoint* pBestGridPoint = 0;
	double bestEvaluate;

    if (!GetGridPoint())
    {
		Grid::RowIterator iRow(GetGrid());
        while (++iRow)
        {
            Row::GridPointIterator iGridPoint(iRow);
            while (++iGridPoint)
            {
                if (!iGridPoint->GetGridObject())
                {
                    iGridPoint->AddGridObject(this);
					double evaluate = GetGrid()->Evaluate();

					if (!pBestGridPoint || evaluate < bestEvaluate)
					{
						bestEvaluate = evaluate;
						pBestGridPoint = iGridPoint;
					}
					iGridPoint->RemoveGridObject(this);
                }
            }
        }

		if (pBestGridPoint)
		{
            pBestGridPoint->AddGridObject(this);
		}
    }
}//@CODE_11986


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_11337
Method which must be called first in a constructor.
*/
void GridObject::ConstructorInclude(Grid* pGrid)
{
    INIT_MULTI_OWNED_ACTIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
    INIT_MULTI_OWNED_ACTIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
    INIT_MULTI_OWNED_PASSIVE(Grid, Grid, GridObject, GridObject)
    INIT_SINGLE_PASSIVE(GridPoint, GridPoint, GridObject, GridObject)
}


/*@NOTE_11339
Method which must be called first in a destructor.
*/
void GridObject::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
    EXIT_MULTI_OWNED_ACTIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
    EXIT_MULTI_OWNED_PASSIVE(Grid, Grid, GridObject, GridObject)
    EXIT_SINGLE_PASSIVE(GridPoint, GridPoint, GridObject, GridObject)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
METHODS_ITERATOR_MULTI_ACTIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
METHODS_MULTI_OWNED_ACTIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
METHODS_ITERATOR_MULTI_ACTIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
METHODS_MULTI_OWNED_PASSIVE(Grid, Grid, GridObject, GridObject)
METHODS_SINGLE_PASSIVE(GridPoint, GridPoint, GridObject, GridObject)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
