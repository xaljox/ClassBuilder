/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassShape.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassShape'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSSHAPE_H
#define _CLASSSHAPE_H

//@START_USER1
//@END_USER1



class ClassShape
    : public ClassDiagramShape
{
    CB_DECLARE_SERIAL(ClassShape)
    RELATION_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    RELATION_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    RELATION_MULTI_OWNED_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    RELATION_MULTI_OWNED_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    RELATION_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)

//@START_USER2
//@END_USER2

// Members
private:
    CbPoint _line1Point1;
    CbPoint _line1Point2;
    CbPoint _line2Point1;
    CbPoint _line2Point2;
    static bool _tracking;
    int _verbosity;
    bool _autoWidth;
    CbRect _templateRect;

protected:

public:

// Methods
private:
    ClassShape(ClassDiagram* pClassDiagram, ClassShape* pClassShape);
    CbRect GetTemplateRect(CbPainter& painter, bool draw = false);
    void OptimizeConnectionBottomPlacement();
    void OptimizeConnectionLeftPlacement();
    void OptimizeConnectionRightPlacement();
    void OptimizeConnectionTopPlacement();
    int RecalculateRectWidth();
    void ConstructorInclude(BaseClass* pBaseClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ClassShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassShape(ClassDiagram* pClassDiagram, BaseClass* pBaseClass,
               const CbPoint& point);
    virtual ~ClassShape();
    CbPoint ConnectionPoint(ClassShape* pClassShape);
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    void DrawSelectedRect(CbPainter& painter, CbColorRef color);
    MemberShape* FindMemberShape(Member* pMember);
    MethodShape* FindMethodShape(Method* pMethod);
    virtual CbRect GetBoundingRect();
    virtual ClassShape* GetClassShape() const;
    virtual Gti* GetGti();
    virtual ClassDiagramShape* GetHitShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                           CbPoint pointLP, bool nested);
    CbPoint GetLeftSelectedPoint();
    CbPoint GetRightSelectedPoint();
    virtual int IsAlignShape() const;
    void NotifyAddMember(Member* pMember);
    void NotifyAddMethod(Method* pMethod);
    virtual int OnEditAttributes(bool checkOnly = false);
    void OptimizeConnectionPlacement();
    void PopulateFromDiagramFlags();
    void RecalculateRect();
    virtual void SetRect(const CbRect& rRect);
    bool GetAutoWidth() const;
    void SetAutoWidth(bool autoWidth);
    CbRect GetTemplateRect() const;
    void SetTemplateRect(const CbRect& rTemplateRect);
    static bool GetTracking();
    int GetVerbosity();
    void SetVerbosity(int verbosity);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSSHAPE_H_INLINES
#define _CLASSSHAPE_H_INLINES

/*@NOTE_7520
Returns the value of member '_autoWidth'.
*/
inline bool ClassShape::GetAutoWidth() const
{//@CODE_7520
    return _autoWidth;
}//@CODE_7520



/*@NOTE_7521
Set the value of member '_autoWidth' to 'autoWidth'.
*/
inline void ClassShape::SetAutoWidth(bool autoWidth)
{//@CODE_7521
    _autoWidth = autoWidth;
}//@CODE_7521



/*@NOTE_35268
Returns the value of member '_templateRect'.
*/
inline CbRect ClassShape::GetTemplateRect() const
{//@CODE_35268
    return _templateRect;
}//@CODE_35268



//@START_USER3
//@END_USER3

#endif
#endif
