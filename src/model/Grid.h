/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Grid.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Grid'
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
#ifndef _GRID_H
#define _GRID_H

//@START_USER1
//@END_USER1



class Grid
{
    RELATION_MULTI_OWNED_ACTIVE(Grid, Grid, Column, Column)
    RELATION_MULTI_OWNED_ACTIVE(Grid, Grid, Row, Row)
    RELATION_MULTI_OWNED_ACTIVE(Grid, Grid, GridObject, GridObject)

//@START_USER2
//@END_USER2

// Members
private:
    ClassDiagram* _pClassDiagram;

protected:

public:

// Methods
private:
    void EnsureEmptyColumns();
    void EnsureEmptyRows();
    void ImproveBySwap();
    void ImproveRouting();
    void MoveClassShapes();
    void SizeRowsAndColumns();
    void TrySwap(GridPoint* pGridPoint1, GridPoint* pGridPoint2, double& value,
                 GoType go, GoType& nextGo);
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    Grid(ClassDiagram* pClassDiagram);
    virtual ~Grid();
    double Evaluate();
    GridObject* FindGridObject(ClassShape* pClassShape);
    void NextPlace();
    void Place();
    void RenumberColumns();
    void RenumberRows();
    ClassDiagram* GetClassDiagram() const;
};

#endif


#ifdef CB_INLINES
#ifndef _GRID_H_INLINES
#define _GRID_H_INLINES

/*@NOTE_11942
Returns the value of member '_pClassDiagram'.
*/
inline ClassDiagram* Grid::GetClassDiagram() const
{//@CODE_11942
    return _pClassDiagram;
}//@CODE_11942



//@START_USER3
//@END_USER3

#endif
#endif
