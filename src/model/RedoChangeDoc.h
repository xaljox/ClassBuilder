/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoChangeDoc.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RedoChangeDoc'
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
