/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MethodShape.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MethodShape'
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
#ifndef _METHODSHAPE_H
#define _METHODSHAPE_H

//@START_USER1
//@END_USER1



class MethodShape
    : public ClassDiagramShape
{
    CB_DECLARE_SERIAL(MethodShape)
    RELATION_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
    RELATION_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Method* pMethod, ClassShape* pClassShape);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    MethodShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MethodShape(ClassShape* pClassShape, Method* pMethod);
    virtual ~MethodShape();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual Gti* GetGti();
    virtual MethodShape* GetMethodShape();
    virtual ClassDiagramShape* GetOuterClassDiagramShape();
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int UsesPenColor() const;
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _METHODSHAPE_H_INLINES
#define _METHODSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
