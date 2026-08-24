/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          UndoChange.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoChange'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _UNDOCHANGE_H
#define _UNDOCHANGE_H

//@START_USER1
//@END_USER1


/*@NOTE_5225
An object state has been changed, if Restore is called this change is undone.
The  member variable '_pX' holds a pointer to the changed object.
Member variable '_pXSave' points to an unreferenced copy that has the
previous state of '_pX'.
*/

class UndoChange
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
    UndoChange(DataModelDocObject* pDataModelDocObject);
    UndoChange(RedoChange* pRedoChange);
    virtual ~UndoChange();
    virtual void Restore();
    DataModelDocObject* GetDataModelDocObjectSave();
};

#endif


#ifdef CB_INLINES
#ifndef _UNDOCHANGE_H_INLINES
#define _UNDOCHANGE_H_INLINES

/*@NOTE_5238
Returns the value of member '_pDataModelDocObjectSave'.
*/
inline DataModelDocObject* UndoChange::GetDataModelDocObjectSave()
{//@CODE_5238
    return _pDataModelDocObjectSave;
}//@CODE_5238



//@START_USER3
//@END_USER3

#endif
#endif
