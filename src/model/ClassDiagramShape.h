/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassDiagramShape.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassDiagramShape'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSDIAGRAMSHAPE_H
#define _CLASSDIAGRAMSHAPE_H

//@START_USER1
//@END_USER1



class ClassDiagramShape
    : public Shape
{
    RELATION_MULTI_OWNED_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
    RELATION_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(ClassDiagram* pClassDiagram);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ClassDiagramShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassDiagramShape(ClassDiagram* pClassDiagram, const CbPoint& point,
                      CbColorRef penColor = Cb_RGB(0, 0, 0),
                      CbColorRef textColor = Cb_RGB(0, 0, 0));
    ClassDiagramShape(ClassDiagram* pClassDiagram,
                      CbColorRef penColor = Cb_RGB(0, 0, 0),
                      CbColorRef textColor = Cb_RGB(0, 0, 0));
    virtual ~ClassDiagramShape();
    void AlignBottom(Shape* pShape);
    void AlignCenter(Shape* pShape);
    void AlignLeft(Shape* pShape);
    void AlignMiddle(Shape* pShape);
    void AlignRight(Shape* pShape);
    void AlignTop(Shape* pShape);
    virtual void CopyShape(ClassDiagram* pClassDiagram) = 0;
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected) = 0;
    bool DrawDirect() const;
    void DrawSelectBox(CbPainter& painter);
    ClassDiagramViewModelSelection* FindClassDiagramViewModelSelection(ClassDiagramViewModel* pClassDiagramViewModel);
    virtual CbRect GetBoundingRect();
    virtual ClassShape* GetClassShape() const;
    virtual ConnectionShape* GetConnectionShape();
    virtual DependencyShape* GetDependencyShape();
    virtual Gti* GetGti();
    virtual ClassDiagramShape* GetHitShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                           CbPoint pointLP, bool nested);
    virtual InheritShape* GetInheritShape();
    virtual MemberShape* GetMemberShape();
    virtual MethodShape* GetMethodShape();
    virtual NoteShape* GetNoteShape();
    virtual ClassDiagramShape* GetOuterClassDiagramShape();
    virtual RelationDiagramOnlyShape* GetRelationDiagramOnlyShape();
    virtual RelationShape* GetRelationShape();
    virtual int IsAlignShape() const;
    int IsSelectedIn(ClassDiagramViewModel* pClassDiagramViewModel);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false) = 0;
    virtual int OnOpen(bool checkOnly = false);
    virtual bool PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                              CbPoint pointLP);
    virtual int UsesPenColor() const;
    virtual int UsesTextColor() const;
    virtual void CleanupReferences();
    bool IsClassShape() const;
    bool IsConnectionShape() const;
    bool IsDependencyShape() const;
    bool IsInheritShape() const;
    bool IsMemberShape() const;
    bool IsMethodShape() const;
    bool IsNoteShape() const;
    bool IsRelationDiagramOnlyShape() const;
    bool IsRelationShape() const;
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSDIAGRAMSHAPE_H_INLINES
#define _CLASSDIAGRAMSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
