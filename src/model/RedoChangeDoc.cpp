/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RedoChangeDoc.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RedoChangeDoc'
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


/*@NOTE_34860
Constructor method.
*/
RedoChangeDoc::RedoChangeDoc(UndoChangeDoc* pUndoChangeDoc) //@INIT_34860
    : RedoBase(pUndoChangeDoc)
    , _pDataModelDocSave(NULL)
{//@CODE_34860
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
}//@CODE_34860


/*@NOTE_34836
Destructor method.
*/
RedoChangeDoc::~RedoChangeDoc()
{//@CODE_34836
    DestructorInclude();

    // Put in your own code
}//@CODE_34836


/*@NOTE_34862
Redo the change.
*/
void RedoChangeDoc::Restore()
{//@CODE_34862
    // Save the current state first
    (void)new UndoChangeDoc(this);

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
}//@CODE_34862


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_34835
Method which must be called first in a constructor.
*/
void RedoChangeDoc::ConstructorInclude()
{
}


/*@NOTE_34837
Method which must be called first in a destructor.
*/
void RedoChangeDoc::DestructorInclude()
{
}


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
