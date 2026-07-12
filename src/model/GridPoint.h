/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          GridPoint.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'GridPoint'
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
