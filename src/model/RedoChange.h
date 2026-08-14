/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RedoChange.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RedoChange'
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
#ifndef _REDOCHANGE_H
#define _REDOCHANGE_H

//@START_USER1
//@END_USER1


/*@NOTE_5242
An object state has been changed and undone, if Restore is called this change
is  redone. The member variable '_pX' holds a pointer to the object to
change.  Member variable '_pXSave' points to an unreferenced copy that
has the  redo state of '_pX'.
*/

class RedoChange
    : public RedoBase
{

//@START_USER2
//@END_USER2

// Members
private:
    DataModelDocObject* _pDataModelDocObjectSave;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    RedoChange(UndoChange* pUndoChange);
    virtual ~RedoChange();
    virtual void Restore();
    virtual DataModelDocObject* GetDataModelDocObjectSave();
};

#endif


#ifdef CB_INLINES
#ifndef _REDOCHANGE_H_INLINES
#define _REDOCHANGE_H_INLINES

/*@NOTE_5255
Returns the value of member '_pDataModelDocObjectSave'.
*/
inline DataModelDocObject* RedoChange::GetDataModelDocObjectSave()
{//@CODE_5255
    return _pDataModelDocObjectSave;
}//@CODE_5255



//@START_USER3
//@END_USER3

#endif
#endif
