/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          GridPoint.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'GridPoint'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _GRIDPOINT_H
#define _GRIDPOINT_H

//@START_USER1
//@END_USER1



class GridPoint
{
    RELATION_SINGLE_ACTIVE(GridPoint, GridPoint, GridObject, GridObject)
    RELATION_MULTI_OWNED_PASSIVE(Column, Column, GridPoint, GridPoint)
    RELATION_MULTI_OWNED_PASSIVE(Row, Row, GridPoint, GridPoint)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Column* pColumn, Row* pRow);
    void DestructorInclude();

protected:

public:
    GridPoint(Column* pColumn, Row* pRow);
    virtual ~GridPoint();
    bool Swap(GridPoint* pGridPoint);
};

#endif


#ifdef CB_INLINES
#ifndef _GRIDPOINT_H_INLINES
#define _GRIDPOINT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
