/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          GridObject.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'GridObject'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _GRIDOBJECT_H
#define _GRIDOBJECT_H

//@START_USER1
//@END_USER1



class GridObject
{
    RELATION_MULTI_OWNED_ACTIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
    RELATION_MULTI_OWNED_ACTIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)
    RELATION_MULTI_OWNED_PASSIVE(Grid, Grid, GridObject, GridObject)
    RELATION_SINGLE_PASSIVE(GridPoint, GridPoint, GridObject, GridObject)

//@START_USER2
//@END_USER2

// Members
private:
    ClassShape* _pClassShape;

protected:

public:

// Methods
private:
    int GetColumn();
    int GetRow();
    void ConstructorInclude(Grid* pGrid);
    void DestructorInclude();

protected:

public:
    GridObject(Grid* pGrid, ClassShape* pClassShape);
    virtual ~GridObject();
    static int Compare(GridObject* a, GridObject* b);
    double Evaluate();
    void PutOnGridPoint();
    ClassShape* GetClassShape() const;
};

#endif


#ifdef CB_INLINES
#ifndef _GRIDOBJECT_H_INLINES
#define _GRIDOBJECT_H_INLINES

/*@NOTE_11998
Returns the value of member '_pClassShape'.
*/
inline ClassShape* GridObject::GetClassShape() const
{//@CODE_11998
    return _pClassShape;
}//@CODE_11998



//@START_USER3
//@END_USER3

#endif
#endif
