/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassDiagramViewModel.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassDiagramViewModel'
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
#ifndef _CLASSDIAGRAMVIEWMODEL_H
#define _CLASSDIAGRAMVIEWMODEL_H

//@START_USER1
//@END_USER1



class ClassDiagramViewModel
{
    RELATION_MULTI_OWNED_ACTIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
    RELATION_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)

//@START_USER2
//@END_USER2

// Members
private:
    RefreshCallback _closeFn;
    void* _refreshCtx;
    RefreshCallback _refreshFn;

protected:

public:

// Methods
private:
    void ConstructorInclude(ClassDiagram* pClassDiagram);
    void DestructorInclude();

protected:

public:
    ClassDiagramViewModel(ClassDiagram* pClassDiagram,
                          RefreshCallback refreshFn, RefreshCallback closeFn,
                          void* refreshCtx);
    virtual ~ClassDiagramViewModel();
    void Refresh() const;
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSDIAGRAMVIEWMODEL_H_INLINES
#define _CLASSDIAGRAMVIEWMODEL_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
