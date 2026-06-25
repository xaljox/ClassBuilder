/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoNew.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoNew'
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
#ifndef _UNDONEW_H
#define _UNDONEW_H

//@START_USER1
//@END_USER1


/*@NOTE_5146
A new object has been added, if Restore is called this new is undone. 
The member variable '_pDocObject' holds a pointer to the new object.
*/

class UndoNew
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
    UndoNew(DataModelDocObject* pDataModelDocObject);
    UndoNew(RedoNew* pRedoNew);
    virtual ~UndoNew();
    virtual void Restore();
};

#endif


#ifdef CB_INLINES
#ifndef _UNDONEW_H_INLINES
#define _UNDONEW_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
