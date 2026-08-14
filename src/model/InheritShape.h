/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          InheritShape.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'InheritShape'
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
#ifndef _INHERITSHAPE_H
#define _INHERITSHAPE_H

//@START_USER1
//@END_USER1



class InheritShape
    : public ConnectionShape
{
    CB_DECLARE_SERIAL(InheritShape)
    RELATION_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    InheritShape(ClassDiagram* pClassDiagram, InheritShape* pInheritShape);
    void ConstructorInclude(Inherit* pInherit);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    InheritShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    InheritShape(ClassDiagram* pClassDiagram, Inherit* pInherit,
                 ClassShape* pFromClassShape, ClassShape* pToClassShape);
    InheritShape(InheritShape* pInheritShape);
    virtual ~InheritShape();
    void ConvertRouting();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual InheritShape* GetInheritShape();
    virtual void MakeNewRouting();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int UsesTextColor() const;
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _INHERITSHAPE_H_INLINES
#define _INHERITSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
