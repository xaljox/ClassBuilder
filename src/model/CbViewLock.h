/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          CbViewLock.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'CbViewLock'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
