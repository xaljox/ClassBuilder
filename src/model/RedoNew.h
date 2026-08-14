/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoNew.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RedoNew'
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
#ifndef _REDONEW_H
#define _REDONEW_H

//@START_USER1
//@END_USER1


/*@NOTE_5161
A new object has been added and undone, if Restore is called this new is
redone.  The member variable '_pX' holds a pointer to the object
removed by the undo operation.
*/

class RedoNew
    : public RedoBase
{

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    RedoNew(UndoNew* pUndoNew);
    virtual ~RedoNew();
    virtual void Restore();
};

#endif


#ifdef CB_INLINES
#ifndef _REDONEW_H_INLINES
#define _REDONEW_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
