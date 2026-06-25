/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          CbViewLock.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'CbViewLock'
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
#ifndef _CBVIEWLOCK_H
#define _CBVIEWLOCK_H

//@START_USER1
//@END_USER1



class CbViewLock
{

//@START_USER2
//@END_USER2

// Members
private:
    DataModelDoc* _pDataModelDoc;
    bool _busyCursor;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();

protected:

public:
    CbViewLock(DataModelDoc* pDataModelDoc, bool busyCursor = true);
    ~CbViewLock();
};

#endif


#ifdef CB_INLINES
#ifndef _CBVIEWLOCK_H_INLINES
#define _CBVIEWLOCK_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
