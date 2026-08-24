/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassDiagramViewModel.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassDiagramViewModel'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
