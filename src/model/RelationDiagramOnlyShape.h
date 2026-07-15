/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationDiagramOnlyShape.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RelationDiagramOnlyShape'
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
#ifndef _RELATIONDIAGRAMONLYSHAPE_H
#define _RELATIONDIAGRAMONLYSHAPE_H

//@START_USER1
//@END_USER1



class RelationDiagramOnlyShape
    : public ConnectionShape
{
    CB_DECLARE_SERIAL(RelationDiagramOnlyShape)

//@START_USER2
//@END_USER2

// Members
private:
    CbPoint _toUmlPoint;
    CbPoint _toNamePoint;
    CbPoint _fromUmlPoint;
    CbPoint _fromNamePoint;
    CbString _umlFrom;
    CbString _umlTo;
    CbString _fromName;
    CbString _toName;
    bool _multi;
    int _owned;
    bool _static;

protected:

public:

// Methods
private:
    RelationDiagramOnlyShape(ClassDiagram* pClassDiagram,
                             RelationDiagramOnlyShape* pRelationDiagramOnlyShape);
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    RelationDiagramOnlyShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    RelationDiagramOnlyShape(ClassDiagram* pClassDiagram,
                             ClassShape* pFromClassShape,
                             ClassShape* pToClassShape);
    RelationDiagramOnlyShape(RelationDiagramOnlyShape* pRelationDiagramOnlyShape);
    virtual ~RelationDiagramOnlyShape();
    void ConvertRouting();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual RelationDiagramOnlyShape* GetRelationDiagramOnlyShape();
    virtual void MakeNewRouting();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual bool PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                              CbPoint pointLP);
    const CbString& GetFromName() const;
    void SetFromName(const CbString& rFromName);
    const CbPoint& GetFromNamePoint() const;
    void SetFromNamePoint(const CbPoint& rFromNamePoint);
    const CbPoint& GetFromUmlPoint() const;
    void SetFromUmlPoint(const CbPoint& rFromUmlPoint);
    bool GetMulti() const;
    void SetMulti(bool multi);
    virtual int GetOwned() const;
    void SetOwned(int owned);
    bool GetStatic() const;
    void SetStatic(bool val);
    const CbString& GetToName() const;
    void SetToName(const CbString& rToName);
    const CbPoint& GetToNamePoint() const;
    void SetToNamePoint(const CbPoint& rToNamePoint);
    const CbPoint& GetToUmlPoint() const;
    void SetToUmlPoint(const CbPoint& rToUmlPoint);
    const CbString& GetUmlFrom() const;
    void SetUmlFrom(const CbString& rUmlFrom);
    const CbString& GetUmlTo() const;
    void SetUmlTo(const CbString& rUmlTo);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _RELATIONDIAGRAMONLYSHAPE_H_INLINES
#define _RELATIONDIAGRAMONLYSHAPE_H_INLINES

/*@NOTE_23161
Returns the value of member '_fromName'.
*/
inline const CbString& RelationDiagramOnlyShape::GetFromName() const
{//@CODE_23161
    return _fromName;
}//@CODE_23161



/*@NOTE_23162
Set the value of member '_fromName' to 'rFromName'.
*/
inline void RelationDiagramOnlyShape::SetFromName(const CbString& rFromName)
{//@CODE_23162
    _fromName = rFromName;
}//@CODE_23162



/*@NOTE_23164
Returns the value of member '_fromNamePoint'.
*/
inline const CbPoint& RelationDiagramOnlyShape::GetFromNamePoint() const
{//@CODE_23164
    return _fromNamePoint;
}//@CODE_23164



/*@NOTE_23165
Set the value of member '_fromNamePoint' to 'rFromNamePoint'.
*/
inline void RelationDiagramOnlyShape::SetFromNamePoint(const CbPoint& rFromNamePoint)
{//@CODE_23165
    _fromNamePoint = rFromNamePoint;
}//@CODE_23165



/*@NOTE_23167
Returns the value of member '_fromUmlPoint'.
*/
inline const CbPoint& RelationDiagramOnlyShape::GetFromUmlPoint() const
{//@CODE_23167
    return _fromUmlPoint;
}//@CODE_23167



/*@NOTE_23168
Set the value of member '_fromUmlPoint' to 'rFromUmlPoint'.
*/
inline void RelationDiagramOnlyShape::SetFromUmlPoint(const CbPoint& rFromUmlPoint)
{//@CODE_23168
    _fromUmlPoint = rFromUmlPoint;
}//@CODE_23168



/*@NOTE_23170
Returns the value of member '_multi'.
*/
inline bool RelationDiagramOnlyShape::GetMulti() const
{//@CODE_23170
    return _multi;
}//@CODE_23170



/*@NOTE_23171
Set the value of member '_multi' to 'multi'.
*/
inline void RelationDiagramOnlyShape::SetMulti(bool multi)
{//@CODE_23171
    _multi = multi;
}//@CODE_23171



/*@NOTE_23174
Set the value of member '_owned' to 'owned'.
*/
inline void RelationDiagramOnlyShape::SetOwned(int owned)
{//@CODE_23174
    _owned = owned;
}//@CODE_23174



/*@NOTE_23176
Returns the value of member '_static'.
*/
inline bool RelationDiagramOnlyShape::GetStatic() const
{//@CODE_23176
    return _static;
}//@CODE_23176



/*@NOTE_23177
Set the value of member '_static' to 'val'.
*/
inline void RelationDiagramOnlyShape::SetStatic(bool val)
{//@CODE_23177
    _static = val;
}//@CODE_23177



/*@NOTE_23179
Returns the value of member '_toName'.
*/
inline const CbString& RelationDiagramOnlyShape::GetToName() const
{//@CODE_23179
    return _toName;
}//@CODE_23179



/*@NOTE_23180
Set the value of member '_toName' to 'rToName'.
*/
inline void RelationDiagramOnlyShape::SetToName(const CbString& rToName)
{//@CODE_23180
    _toName = rToName;
}//@CODE_23180



/*@NOTE_23182
Returns the value of member '_toNamePoint'.
*/
inline const CbPoint& RelationDiagramOnlyShape::GetToNamePoint() const
{//@CODE_23182
    return _toNamePoint;
}//@CODE_23182



/*@NOTE_23183
Set the value of member '_toNamePoint' to 'rToNamePoint'.
*/
inline void RelationDiagramOnlyShape::SetToNamePoint(const CbPoint& rToNamePoint)
{//@CODE_23183
    _toNamePoint = rToNamePoint;
}//@CODE_23183



/*@NOTE_23185
Returns the value of member '_toUmlPoint'.
*/
inline const CbPoint& RelationDiagramOnlyShape::GetToUmlPoint() const
{//@CODE_23185
    return _toUmlPoint;
}//@CODE_23185



/*@NOTE_23186
Set the value of member '_toUmlPoint' to 'rToUmlPoint'.
*/
inline void RelationDiagramOnlyShape::SetToUmlPoint(const CbPoint& rToUmlPoint)
{//@CODE_23186
    _toUmlPoint = rToUmlPoint;
}//@CODE_23186



/*@NOTE_23149
Returns the value of member '_umlTo'.
*/
inline const CbString& RelationDiagramOnlyShape::GetUmlFrom() const
{//@CODE_23149
    return _umlFrom;
}//@CODE_23149



/*@NOTE_23150
Set the value of member '_umlTo' to 'rUmlFrom'.
*/
inline void RelationDiagramOnlyShape::SetUmlFrom(const CbString& rUmlFrom)
{//@CODE_23150
    _umlFrom = rUmlFrom;
}//@CODE_23150



/*@NOTE_23153
Returns the value of member '_umlTo'.
*/
inline const CbString& RelationDiagramOnlyShape::GetUmlTo() const
{//@CODE_23153
    return _umlTo;
}//@CODE_23153



/*@NOTE_23154
Set the value of member '_umlTo' to 'rUmlTo'.
*/
inline void RelationDiagramOnlyShape::SetUmlTo(const CbString& rUmlTo)
{//@CODE_23154
    _umlTo = rUmlTo;
}//@CODE_23154



//@START_USER3
//@END_USER3

#endif
#endif
