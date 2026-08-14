/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Row.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Row'
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
#ifndef _ROW_H
#define _ROW_H

//@START_USER1
//@END_USER1



class Row
{
    RELATION_MULTI_OWNED_ACTIVE(Row, Row, GridPoint, GridPoint)
    RELATION_MULTI_OWNED_PASSIVE(Grid, Grid, Row, Row)

//@START_USER2
//@END_USER2

// Members
private:
    int _row;
    int _height;

protected:

public:

// Methods
private:
    void ConstructorInclude(Grid* pGrid);
    void DestructorInclude();

protected:

public:
    Row(Grid* pGrid, bool first = false);
    virtual ~Row();
    int GetHeight();
    int GetRow() const;
    void SetRow(int row);
};

#endif


#ifdef CB_INLINES
#ifndef _ROW_H_INLINES
#define _ROW_H_INLINES

/*@NOTE_11968
Returns the value of member '_row'.
*/
inline int Row::GetRow() const
{//@CODE_11968
    return _row;
}//@CODE_11968



/*@NOTE_11969
Set the value of member '_row' to 'row'.
*/
inline void Row::SetRow(int row)
{//@CODE_11969
    _row = row;
}//@CODE_11969



//@START_USER3
//@END_USER3

#endif
#endif
