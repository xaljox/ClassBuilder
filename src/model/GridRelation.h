/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          GridRelation.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'GridRelation'
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
#ifndef _GRIDRELATION_H
#define _GRIDRELATION_H

//@START_USER1
//@END_USER1



class GridRelation
{
    RELATION_MULTI_OWNED_PASSIVE(GridObject, FromGridObject, GridRelation, FromGridRelation)
    RELATION_MULTI_OWNED_PASSIVE(GridObject, ToGridObject, GridRelation, ToGridRelation)

//@START_USER2
//@END_USER2

// Members
private:
    double _directionWeight;
    double _weight;

protected:

public:

// Methods
private:
    void ConstructorInclude(GridObject* pFromGridObject,
                            GridObject* pToGridObject);
    void DestructorInclude();

protected:

public:
    GridRelation(GridObject* pFromGridObject, GridObject* pToGridObject,
                 double weight = 1.0, double directionWeight = 1.0);
    virtual ~GridRelation();
    double GetDirectionWeight() const;
    double GetWeight() const;
};

#endif


#ifdef CB_INLINES
#ifndef _GRIDRELATION_H_INLINES
#define _GRIDRELATION_H_INLINES

/*@NOTE_12195
Returns the value of member '_directionWeight'.
*/
inline double GridRelation::GetDirectionWeight() const
{//@CODE_12195
    return _directionWeight;
}//@CODE_12195



/*@NOTE_12196
Returns the value of member '_weight'.
*/
inline double GridRelation::GetWeight() const
{//@CODE_12196
    return _weight;
}//@CODE_12196



//@START_USER3
//@END_USER3

#endif
#endif
