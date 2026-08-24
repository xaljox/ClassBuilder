/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Grid.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Grid'
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


/*@NOTE_11943
Constructor method.
*/
Grid::Grid(ClassDiagram* pClassDiagram) //@INIT_11943
    : _pClassDiagram(pClassDiagram)
{//@CODE_11943
    ConstructorInclude();
    
    // Put in your own code
    (void)new Column(this);
    (void)new Row(this);
    
    ClassDiagram::ClassDiagramShapeIterator iClassDiagramShape(pClassDiagram);
    while (++iClassDiagramShape)
    {
        ClassShape* pClassShape = dynamic_cast<ClassShape*>(iClassDiagramShape.Get());
        if (pClassShape)
        {
            (void)new GridObject(this, pClassShape);
        }
    }
    
    GridObjectIterator iGridObject(this);
    while (++iGridObject)
    {
        double inheritCnt = 0;
        ClassShape::FromConnectionShapeIterator iFromConnectionShape(iGridObject->GetClassShape());
        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->IsInheritShape())
            {
                inheritCnt += 1.0;
            }
        }
        
        iFromConnectionShape.Reset();
        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->GetToClassShape() != iGridObject->GetClassShape())
            {
                GridObject* pGridObject = FindGridObject(iFromConnectionShape->GetToClassShape());
                if (pGridObject)
                {
                    if (iFromConnectionShape->IsInheritShape())
                    {
                        (void)new GridRelation(iGridObject, pGridObject, 1.0/inheritCnt);
                    }
                    else
                    {
                        (void)new GridRelation(iGridObject, pGridObject);
                    }
                    iFromConnectionShape->SaveState();
                    iFromConnectionShape->SetInitial(true);
                }
            }
        }
    }
}//@CODE_11943


/*@NOTE_11060
Destructor method.
*/
Grid::~Grid()
{//@CODE_11060
    DestructorInclude();

    // Put in your own code
}//@CODE_11060


void Grid::EnsureEmptyColumns()
{//@CODE_11949
    // Check if there is always an empty first & last column
	ColumnIterator iColumn(this);
	while (++iColumn)
	{
		Column::GridPointIterator iGridPoint(iColumn);
		while (++iGridPoint)
		{
			if (iGridPoint->GetGridObject())
			{
				break;
			}
		}

		// Column is not empty
		if (iGridPoint)
		{
			if (iColumn.IsFirst())
			{
				(void)new Column(this, true);
			}
			if (iColumn.IsLast())
			{
				(void)new Column(this);
			}
		}
		else if (!iColumn.IsFirst() && !iColumn.IsLast())
		{
			// Empty Column which isn't first or last
			delete iColumn;
		}
	}
}//@CODE_11949


void Grid::EnsureEmptyRows()
{//@CODE_11950
    // Check if there is always an empty first & last row
	RowIterator iRow(this);
	while (++iRow)
	{
		Row::GridPointIterator iGridPoint(iRow);
		while (++iGridPoint)
		{
			if (iGridPoint->GetGridObject())
			{
				break;
			}
		}

		// Row is not empty
		if (iGridPoint)
		{
			if (iRow.IsFirst())
			{
				(void)new Row(this, true);
			}
			if (iRow.IsLast())
			{
				(void)new Row(this);
			}
		}
		else if (!iRow.IsFirst() && !iRow.IsLast())
		{
			// Empty row which isn't first or last
			delete iRow;
		}
	}
}//@CODE_11950


double Grid::Evaluate()
{//@CODE_11951
    double value = 0;

    GridObjectIterator iGridObject(this);
    while (++iGridObject)
    {
        value += iGridObject->Evaluate();
    }
    
    return value;
}//@CODE_11951


GridObject* Grid::FindGridObject(ClassShape* pClassShape)
{//@CODE_11996
    GridObjectIterator iGridObject(this);
    while (++iGridObject)
    {
        if (pClassShape == iGridObject->GetClassShape())
        {
            return iGridObject;
        }
    }

    return 0;
}//@CODE_11996


void Grid::ImproveBySwap()
{//@CODE_11945
    double value = Evaluate();
        
    // As long as there is improvement loop, if no improvement, allow a round
    // of swapping with equal value, to get out of local minimum
    for (GoType nextGo, go = IMPROVE; go != STOP; go = nextGo)
    {
        if (go == IMPROVE)
            nextGo = FIRSTEQUAL;
        else
            nextGo = STOP;
        
        RowIterator iRow(this);
        while (++iRow)
        {
            Row::GridPointIterator iGridPoint(iRow);
            while (++iGridPoint)
            {
                Row::GridPointIterator iGridPointSameRow(iGridPoint);
                while (++iGridPointSameRow)
                {
                    TrySwap(iGridPoint, iGridPointSameRow, value, go, nextGo);
                }
                
                RowIterator iOtherRow(iRow);
                while (++iOtherRow)
                {
                    Row::GridPointIterator iGridPointOtherRow(iOtherRow);
                    while (++iGridPointOtherRow)
                    {
                        TrySwap(iGridPoint, iGridPointOtherRow, value, go, nextGo);
                    }
                }
            }
        }
    }
}//@CODE_11945


void Grid::ImproveRouting()
{//@CODE_14322
    GridObjectIterator iGridObject(this);
    while (++iGridObject)
    {
        iGridObject->GetClassShape()->OptimizeConnectionPlacement();
    }
}//@CODE_14322


void Grid::MoveClassShapes()
{//@CODE_12004
    int y = -150;
    RowIterator iRow(this);
    while (++iRow)
    {
        int x = 150;
        Row::GridPointIterator iGridPoint(iRow);
        while (++iGridPoint)
        {
            if (iGridPoint->GetGridObject())
            {
                CbRect orgRect = iGridPoint->GetGridObject()->GetClassShape()->GetRect();
				CbPoint org((iGridPoint->GetColumn()->GetWidth() - orgRect.Width())/2 + x, y);
				Shape::Round(org);

                CbRect newRect(org.x, org.y-orgRect.Height(), org.x+orgRect.Width(), org.y);
                iGridPoint->GetGridObject()->GetClassShape()->SetRect(newRect);
            }
            x += (iGridPoint->GetColumn()->GetWidth() + 150);
        }
        
        y -= iRow->GetHeight() + 210;
    }
}//@CODE_12004


void Grid::NextPlace()
{//@CODE_19878
    SortGridObject(GridObject::Compare);
    DeleteAllColumn();
    DeleteAllRow();
    (void)new Column(this);
    (void)new Row(this);

    // Optimize placement
    GridObjectIterator iGridObject(this);
    while (++iGridObject)
    {
        iGridObject->PutOnGridPoint();
        ImproveBySwap();
        EnsureEmptyColumns();
        EnsureEmptyRows();
    }

    // Calculate width and height needed and throw away empty rows and columns
    SizeRowsAndColumns();
    
    // Put ClassShapes at their new place on drawing
    MoveClassShapes();
    
    ImproveRouting();
}//@CODE_19878


void Grid::Place()
{//@CODE_11946
    // Optimize placement
    GridObjectIterator iGridObject(this);
    while (++iGridObject)
    {
        iGridObject->PutOnGridPoint();
        ImproveBySwap();
        EnsureEmptyColumns();
        EnsureEmptyRows();
    }

    // Calculate width and height needed and throw away empty rows and columns
    SizeRowsAndColumns();

	// Change order of objects, so we can generated a new type of placement
    SortGridObject(GridObject::Compare);
    iGridObject.Reset();
    while (--iGridObject)
    {
		iGridObject->GetClassShape()->SaveState(1);
		_pClassDiagram->MoveClassDiagramShapeFirst(iGridObject->GetClassShape());
	}
    
    // Put ClassShapes at their new place on drawing
    MoveClassShapes();
    
    ImproveRouting();
}//@CODE_11946


void Grid::RenumberColumns()
{//@CODE_11947
    int column = 0;
    ColumnIterator iColumn(this);
    while (++iColumn)
    {
        iColumn->SetColumn(column++);
    }
}//@CODE_11947


void Grid::RenumberRows()
{//@CODE_11948
    int row = 0;
    RowIterator iRow(this);
    while (++iRow)
    {
        iRow->SetRow(row++);
    }
}//@CODE_11948


void Grid::SizeRowsAndColumns()
{//@CODE_11999
    ColumnIterator iColumn(this);
    while (++iColumn)
    {
        if (iColumn->GetWidth() == 0)
        {
            delete iColumn;
        }
    }
    
    RowIterator iRow(this);
    while (++iRow)
    {
        if (iRow->GetHeight() == 0)
        {
            delete iRow;
        }
    }
}//@CODE_11999


void Grid::TrySwap(GridPoint* pGridPoint1, GridPoint* pGridPoint2,
                   double& value, GoType go, GoType& nextGo)
{//@CODE_11952
    if (pGridPoint1->Swap(pGridPoint2))
    {
        double newValue = Evaluate();
        if (newValue < value)
        {
            value = newValue;
            // cout << value << endl;
            nextGo = IMPROVE;
        }
        else if (go != IMPROVE && newValue == value)
        {
            // cout << value << endl;
            if (go == FIRSTEQUAL && nextGo == STOP)
                nextGo = LASTEQUAL;
        }
        else
        {
            pGridPoint1->Swap(pGridPoint2); // Undo swap
        }
    }
}//@CODE_11952


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_11059
Method which must be called first in a constructor.
*/
void Grid::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(Grid, Grid, Column, Column)
    INIT_MULTI_OWNED_ACTIVE(Grid, Grid, Row, Row)
    INIT_MULTI_OWNED_ACTIVE(Grid, Grid, GridObject, GridObject)
}


/*@NOTE_11061
Method which must be called first in a destructor.
*/
void Grid::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Grid, Grid, Column, Column)
    EXIT_MULTI_OWNED_ACTIVE(Grid, Grid, Row, Row)
    EXIT_MULTI_OWNED_ACTIVE(Grid, Grid, GridObject, GridObject)
}


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Grid, Grid, Column, Column)
METHODS_ITERATOR_MULTI_ACTIVE(Grid, Grid, Column, Column)
METHODS_MULTI_OWNED_ACTIVE(Grid, Grid, Row, Row)
METHODS_ITERATOR_MULTI_ACTIVE(Grid, Grid, Row, Row)
METHODS_MULTI_OWNED_ACTIVE(Grid, Grid, GridObject, GridObject)
METHODS_ITERATOR_MULTI_ACTIVE(Grid, Grid, GridObject, GridObject)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
