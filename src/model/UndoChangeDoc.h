/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          UndoChangeDoc.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoChangeDoc'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
