/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassDiagramViewModelSelection.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassDiagramViewModelSelection'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSDIAGRAMVIEWMODELSELECTION_H
#define _CLASSDIAGRAMVIEWMODELSELECTION_H

//@START_USER1
//@END_USER1



class ClassDiagramViewModelSelection
{
    RELATION_MULTI_OWNED_PASSIVE(ClassDiagramViewModel, ClassDiagramViewModel, ClassDiagramViewModelSelection, Selected)
    RELATION_MULTI_OWNED_PASSIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(ClassDiagramViewModel* pClassDiagramViewModel,
                            ClassDiagramShape* pClassDiagramShape);
    void DestructorInclude();

protected:

public:
    ClassDiagramViewModelSelection(ClassDiagramViewModel* pClassDiagramViewModel,
                                   ClassDiagramShape* pClassDiagramShape);
    virtual ~ClassDiagramViewModelSelection();
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSDIAGRAMVIEWMODELSELECTION_H_INLINES
#define _CLASSDIAGRAMVIEWMODELSELECTION_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
