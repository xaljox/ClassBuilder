/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoDelete.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RedoDelete'
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
#ifndef _REDODELETE_H
#define _REDODELETE_H

//@START_USER1
//@END_USER1


/*@NOTE_5193
An object has been deleted and undone, if Restore is called this delete is
redone. The member variable '_pX' holds a pointer to the object to
redelete.
*/

class RedoDelete
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
    RedoDelete(UndoDelete* pUndoDelete);
    virtual ~RedoDelete();
    virtual void Restore();
};

#endif


#ifdef CB_INLINES
#ifndef _REDODELETE_H_INLINES
#define _REDODELETE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
