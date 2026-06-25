/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Contrib.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Contrib'
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
//@END_USER2


// Static members


/*@NOTE_38552
Constructor method.
*/
Contrib::Contrib(LifeLineShape* pLLS1, LifeLineShape* pLLS2,
                 int needed) //@INIT_38552
    : _pLeft(pLLS1)
    , _pRight(pLLS2)
    , _needed(needed)
{//@CODE_38552
    ConstructorInclude();

    // Put in your own code
    SequenceDiagram* pSD = pLLS1->GetSequenceDiagram();
    for (LifeLineShape* p = pSD->GetPrevLifeLineShape(pLLS1); p; p = pSD->GetPrevLifeLineShape(p))
    {
        if (p == pLLS2)
        { 
            _pLeft  = pLLS2; 
            _pRight = pLLS1; 
            break; 
        }
    }
}//@CODE_38552


/*@NOTE_38451
Destructor method.
*/
Contrib::~Contrib()
{//@CODE_38451
    DestructorInclude();

    // Put in your own code
}//@CODE_38451


Contrib* Contrib::FindContrib(LifeLineShape* pLLS1, LifeLineShape* pLLS2)
{//@CODE_38556
    ContribIterator iContrib;
    while (++iContrib)
    {
        if ((pLLS1 == iContrib->GetLeft() &&
             pLLS2 == iContrib->GetRight()) ||
            (pLLS1 == iContrib->GetRight() &&
             pLLS2 == iContrib->GetLeft()))
        {
            return iContrib;
        }
    }

    return 0;
}//@CODE_38556


Contrib* Contrib::FindNextRightContrib(LifeLineShape* pRight,
                                       Contrib* startAfterContrib)
{//@CODE_38561
    ContribIterator iContrib(startAfterContrib);
    while (++iContrib)
    {
        if (pRight == iContrib->GetRight())
        {
            return iContrib;
        }
    }

    return 0;
}//@CODE_38561


Contrib* Contrib::FindRightContrib(LifeLineShape* pRight)
{//@CODE_38564
    ContribIterator iContrib;
    while (++iContrib)
    {
        if (pRight == iContrib->GetRight())
        {
            return iContrib;
        }
    }

    return 0;
}//@CODE_38564


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_38452
Method which must be called first in a constructor.
*/
void Contrib::ConstructorInclude()
{
    INIT_STATIC_MULTI_OWNED_ACTIVE(Contrib, Contrib, Contrib, Contrib)
    INIT_STATIC_MULTI_OWNED_PASSIVE(Contrib, Contrib, Contrib, Contrib)
}


/*@NOTE_38453
Method which must be called first in a destructor.
*/
void Contrib::DestructorInclude()
{
    EXIT_STATIC_MULTI_OWNED_ACTIVE(Contrib, Contrib, Contrib, Contrib)
    EXIT_STATIC_MULTI_OWNED_PASSIVE(Contrib, Contrib, Contrib, Contrib)
}


// Methods for the relation(s) of the class
METHODS_STATIC_MULTI_OWNED_ACTIVE(Contrib, Contrib, Contrib, Contrib)
METHODS_ITERATOR_NOFILTER_STATIC_MULTI_ACTIVE(Contrib, Contrib, Contrib, Contrib)
METHODS_STATIC_MULTI_OWNED_PASSIVE(Contrib, Contrib, Contrib, Contrib)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
