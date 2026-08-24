/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Column.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Column'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _COLUMN_H
#define _COLUMN_H

//@START_USER1
//@END_USER1



class Column
{
    RELATION_MULTI_OWNED_ACTIVE(Column, Column, GridPoint, GridPoint)
    RELATION_MULTI_OWNED_PASSIVE(Grid, Grid, Column, Column)

//@START_USER2
//@END_USER2

// Members
private:
    int _column;
    int _width;

protected:

public:

// Methods
private:
    void ConstructorInclude(Grid* pGrid);
    void DestructorInclude();

protected:

public:
    Column(Grid* pGrid, bool first = false);
    virtual ~Column();
    int GetColumn() const;
    void SetColumn(int column);
    int GetWidth();
};

#endif


#ifdef CB_INLINES
#ifndef _COLUMN_H_INLINES
#define _COLUMN_H_INLINES

/*@NOTE_11960
Returns the value of member '_column'.
*/
inline int Column::GetColumn() const
{//@CODE_11960
    return _column;
}//@CODE_11960



/*@NOTE_11961
Set the value of member '_column' to 'column'.
*/
inline void Column::SetColumn(int column)
{//@CODE_11961
    _column = column;
}//@CODE_11961



//@START_USER3
//@END_USER3

#endif
#endif
