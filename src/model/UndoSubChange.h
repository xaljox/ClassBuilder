/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoSubChange.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoSubChange'
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
#ifndef _UNDOSUBCHANGE_H
#define _UNDOSUBCHANGE_H

//@START_USER1
//@END_USER1



class UndoSubChange
    : public UndoBase
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
    UndoSubChange(DataModelDocObject* pDataModelDocObject);
    virtual ~UndoSubChange();
    virtual void Restore();
    DataModelDocObject* GetDataModelDocObjectSave();
};

#endif


#ifdef CB_INLINES
#ifndef _UNDOSUBCHANGE_H_INLINES
#define _UNDOSUBCHANGE_H_INLINES

/*@NOTE_5844
Returns the value of member '_pDataModelDocObjectSave'.
*/
inline DataModelDocObject* UndoSubChange::GetDataModelDocObjectSave()
{//@CODE_5844
    return _pDataModelDocObjectSave;
}//@CODE_5844



//@START_USER3
//@END_USER3

#endif
#endif
