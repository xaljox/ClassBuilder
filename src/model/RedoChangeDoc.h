/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RedoChangeDoc.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RedoChangeDoc'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _REDOCHANGEDOC_H
#define _REDOCHANGEDOC_H

//@START_USER1
//@END_USER1



class RedoChangeDoc
    : public RedoBase
{

//@START_USER2
//@END_USER2

// Members
private:
    DataModelDoc* _pDataModelDocSave;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    RedoChangeDoc(UndoChangeDoc* pUndoChangeDoc);
    virtual ~RedoChangeDoc();
    virtual void Restore();
};

#endif


#ifdef CB_INLINES
#ifndef _REDOCHANGEDOC_H_INLINES
#define _REDOCHANGEDOC_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
