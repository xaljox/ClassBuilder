/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          UndoDelete.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoDelete'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _UNDODELETE_H
#define _UNDODELETE_H

//@START_USER1
//@END_USER1


/*@NOTE_5178
An object has been deleted, if Restore is called this delete is undone. The
member variable '_pX' holds a pointer to the deleted object. Note that
by calling Delete on X no actual delete is performed, but al references
within the document to it are removed.
*/

class UndoDelete
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
    UndoDelete(DataModelDocObject* pDataModelDocObject);
    UndoDelete(RedoDelete* pRedoDelete);
    virtual ~UndoDelete();
    virtual void Restore();
};

#endif


#ifdef CB_INLINES
#ifndef _UNDODELETE_H_INLINES
#define _UNDODELETE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
