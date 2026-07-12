/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationShape.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RelationShape'
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
#ifndef _RELATIONSHAPE_H
#define _RELATIONSHAPE_H

//@START_USER1
//@END_USER1



class RelationShape
    : public ConnectionShape
{
    CB_DECLARE_SERIAL(RelationShape)
    RELATION_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)

//@START_USER2
//@END_USER2

// Members
private:
    CbPoint _fromNamePoint;
    CbPoint _toNamePoint;
    int _verbosity;
    CbPoint _fromUmlPoint;
    CbPoint _toUmlPoint;
    CbColorRef _criticalPenColor;

protected:

public:

// Methods
private:
    RelationShape(ClassDiagram* pClassDiagram, RelationShape* pRelationShape);
    CbString GetUmlFrom();
    CbString GetUmlTo();
    void ConstructorInclude(Relation* pRelation);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    RelationShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    RelationShape(ClassDiagram* pClassDiagram, Relation* pRelation,
                  ClassShape* pFromClassShape, ClassShape* pToClassShape);
    RelationShape(RelationShape* pRelationShape);
    virtual ~RelationShape();
    void ConvertRouting();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual int GetOwned() const;
    virtual CbColorRef GetPenColor() const;
    virtual RelationShape* GetRelationShape();
    virtual void MakeNewRouting();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual bool PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                              CbPoint pointLP);
    virtual void SetPenColor(CbColorRef penColor);
    CbColorRef GetCriticalPenColor() const;
    void SetCriticalPenColor(CbColorRef criticalPenColor);
    const CbPoint& GetFromNamePoint();
    void SetFromNamePoint(const CbPoint& rFromNamePoint);
    const CbPoint& GetFromUmlPoint();
    void SetFromUmlPoint(const CbPoint& rFromUmlPoint);
    const CbPoint& GetToNamePoint();
    void SetToNamePoint(const CbPoint& rToNamePoint);
    const CbPoint& GetToUmlPoint();
    void SetToUmlPoint(const CbPoint& rToUmlPoint);
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
#ifndef _RELATIONSHAPE_H_INLINES
#define _RELATIONSHAPE_H_INLINES

/*@NOTE_35064
Returns the value of member '_textColor'.
*/
inline CbColorRef RelationShape::GetPenColor() const
{//@CODE_35064
    if (!GetRelation()->GetCritical())
    {
        return Shape::GetPenColor();
    }
    else
    {
        return _criticalPenColor;
    }
    
}//@CODE_35064



/*@NOTE_35045
Returns the value of member '_criticalPenColor'.
*/
inline CbColorRef RelationShape::GetCriticalPenColor() const
{//@CODE_35045
    return _criticalPenColor;
}//@CODE_35045



/*@NOTE_35046
Set the value of member '_criticalPenColor' to 'criticalPenColor'.
*/
inline void RelationShape::SetCriticalPenColor(CbColorRef criticalPenColor)
{//@CODE_35046
    _criticalPenColor = criticalPenColor;
}//@CODE_35046



//@START_USER3
//@END_USER3

#endif
#endif
