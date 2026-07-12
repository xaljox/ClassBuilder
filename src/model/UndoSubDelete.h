/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoSubDelete.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoSubDelete'
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
#ifndef _UNDOSUBDELETE_H
#define _UNDOSUBDELETE_H

//@START_USER1
//@END_USER1


/*@NOTE_5210
A resulting delete from another delete, a redo of it is unneccesarry and
unwanted.
*/

class UndoSubDelete
    : public UndoBase
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
    UndoSubDelete(DataModelDocObject* pDataModelDocObject);
    virtual ~UndoSubDelete();
    virtual void Restore();
};

#endif


#ifdef CB_INLINES
#ifndef _UNDOSUBDELETE_H_INLINES
#define _UNDOSUBDELETE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
