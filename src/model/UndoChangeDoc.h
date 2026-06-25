/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoChangeDoc.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoChangeDoc'
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
#ifndef _UNDOCHANGEDOC_H
#define _UNDOCHANGEDOC_H

//@START_USER1
//@END_USER1



class UndoChangeDoc
    : public UndoBase
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
    UndoChangeDoc(DataModelDoc* pDataModelDoc);
    UndoChangeDoc(RedoChangeDoc* pRedoChangeDoc);
    virtual ~UndoChangeDoc();
    virtual void Restore();
    DataModelDoc* GetDataModelDocSave() const;
};

#endif


#ifdef CB_INLINES
#ifndef _UNDOCHANGEDOC_H_INLINES
#define _UNDOCHANGEDOC_H_INLINES

/*@NOTE_34859
Returns the value of member '_pDataModelDocSave'.
*/
inline DataModelDoc* UndoChangeDoc::GetDataModelDocSave() const
{//@CODE_34859
    return _pDataModelDocSave;
}//@CODE_34859



//@START_USER3
//@END_USER3

#endif
#endif
