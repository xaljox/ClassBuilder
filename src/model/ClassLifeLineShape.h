/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassLifeLineShape.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassLifeLineShape'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSLIFELINESHAPE_H
#define _CLASSLIFELINESHAPE_H

//@START_USER1
//@END_USER1



class ClassLifeLineShape
    : public LifeLineShape
{
    CB_DECLARE_SERIAL(ClassLifeLineShape)
    RELATION_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)

//@START_USER2
//@END_USER2

// Members
private:
    bool _autoWidth;
    CbString _template;
    CbRect _templateRect;

protected:

public:

// Methods
private:
    CbColorRef GetPenColor(CbPainter& painter);
    CbRect GetTemplateRect(CbPainter& painter, bool draw = false);
    int RecalculateRectWidth();
    bool WrongCreationOrDestruction();
    void ConstructorInclude(BaseClass* pBaseClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ClassLifeLineShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassLifeLineShape(SequenceDiagram* pSequenceDiagram, BaseClass* pBaseClass,
                       const CbPoint& point);
    virtual ~ClassLifeLineShape();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected);
    void DrawSelectedRect(CbPainter& painter, CbColorRef color);
    virtual CbRect GetBoundingRect();
    virtual ClassLifeLineShape* GetClassLifeLine();
    CbPoint GetLeftSelectedPoint();
    virtual int GetLifeLineLength();
    CbPoint GetRightSelectedPoint();
    virtual CbString GetTypeName();
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual bool PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              const CbPoint& pointLP);
    bool GetAutoWidth() const;
    void SetAutoWidth(bool autoWidth);
    const CbString& GetTemplate() const;
    void SetTemplate(const CbString& rTemplate);
    CbRect GetTemplateRect() const;
    void SetTemplateRect(const CbRect& rTemplateRect);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSLIFELINESHAPE_H_INLINES
#define _CLASSLIFELINESHAPE_H_INLINES

/*@NOTE_33493
Returns the value of member '_autoWidth'.
*/
inline bool ClassLifeLineShape::GetAutoWidth() const
{//@CODE_33493
    return _autoWidth;
}//@CODE_33493



/*@NOTE_33494
Set the value of member '_autoWidth' to 'autoWidth'.
*/
inline void ClassLifeLineShape::SetAutoWidth(bool autoWidth)
{//@CODE_33494
    _autoWidth = autoWidth;
}//@CODE_33494



/*@NOTE_34528
Returns the value of member '_template'.
*/
inline const CbString& ClassLifeLineShape::GetTemplate() const
{//@CODE_34528
    return _template;
}//@CODE_34528



/*@NOTE_34529
Set the value of member '_template' to 'rTemplate'.
*/
inline void ClassLifeLineShape::SetTemplate(const CbString& rTemplate)
{//@CODE_34529
    _template = rTemplate;
}//@CODE_34529



/*@NOTE_35261
Returns the value of member '_templateRect'.
*/
inline CbRect ClassLifeLineShape::GetTemplateRect() const
{//@CODE_35261
    return _templateRect;
}//@CODE_35261



//@START_USER3
//@END_USER3

#endif
#endif
