/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoChangeDoc.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'UndoChangeDoc'
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
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
#include <sstream>
//@END_USER2


// Static members


/*@NOTE_34855
Constructor method.
*/
UndoChangeDoc::UndoChangeDoc(DataModelDoc* pDataModelDoc) //@INIT_34855
    : UndoBase(pDataModelDoc)
    , _pDataModelDocSave(NULL)
{//@CODE_34855
    ConstructorInclude();

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive store(static_cast<std::ostream&>(ss));
        pDataModelDoc->SerializeMembersOnly(store);
    }
    _pDataModelDocSave = new DataModelDoc();
    ss.seekg(0);
    {
        CbArchive load(static_cast<std::istream&>(ss));
        _pDataModelDocSave->SerializeMembersOnly(load);
    }
}//@CODE_34855


/*@NOTE_34864
Constructor method.
*/
UndoChangeDoc::UndoChangeDoc(RedoChangeDoc* pRedoChangeDoc) //@INIT_34864
    : UndoBase(pRedoChangeDoc)
    , _pDataModelDocSave(NULL)
{//@CODE_34864
    ConstructorInclude();

    DataModelDoc* pDataModelDoc = (DataModelDoc*)_pDataModelDocObject;
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive store(static_cast<std::ostream&>(ss));
        pDataModelDoc->SerializeMembersOnly(store);
    }
    _pDataModelDocSave = new DataModelDoc();
    ss.seekg(0);
    {
        CbArchive load(static_cast<std::istream&>(ss));
        _pDataModelDocSave->SerializeMembersOnly(load);
    }
}//@CODE_34864


/*@NOTE_34817
Destructor method.
*/
UndoChangeDoc::~UndoChangeDoc()
{//@CODE_34817
    DestructorInclude();

    // Put in your own code
    delete _pDataModelDocSave;
}//@CODE_34817


/*@NOTE_34857
Undo the change.
*/
void UndoChangeDoc::Restore()
{//@CODE_34857
    // Save the current state first
    (void)new RedoChangeDoc(this);

    DataModelDoc* pDataModelDoc = (DataModelDoc*)_pDataModelDocObject;
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        CbArchive store(static_cast<std::ostream&>(ss));
        _pDataModelDocSave->SerializeMembersOnly(store);
    }
    ss.seekg(0);
    {
        CbArchive load(static_cast<std::istream&>(ss));
        pDataModelDoc->SerializeMembersOnly(load);
    }

    delete this;
}//@CODE_34857


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_34816
Method which must be called first in a constructor.
*/
void UndoChangeDoc::ConstructorInclude()
{
}


/*@NOTE_34818
Method which must be called first in a destructor.
*/
void UndoChangeDoc::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
