/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ConnectionShape.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ConnectionShape'
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
#ifndef _CONNECTIONSHAPE_H
#define _CONNECTIONSHAPE_H

//@START_USER1
//@END_USER1



class ConnectionShape
    : public ClassDiagramShape
{
    RELATION_MULTI_OWNED_ACTIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)
    RELATION_MULTI_OWNED_PASSIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    RELATION_MULTI_OWNED_PASSIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    RELATION_MULTI_PASSIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)

//@START_USER2
//@END_USER2

// Members
private:
    CbPoint _startPoint;
    CbPoint _endPoint;
    CbSize _minStartSize;
    CbSize _minEndSize;
    bool _initial;

protected:

public:

// Methods
private:
    CbSize GetMinSize(const CbRect& rect, const CbPoint& point);
    void ConstructorInclude(ClassShape* pFromClassShape,
                            ClassShape* pToClassShape);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ConnectionShape();
    ConnectionShape(ClassDiagram* pClassDiagram,
                    ConnectionShape* pConnectionShape);
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ConnectionShape(ClassDiagram* pClassDiagram, ClassShape* pFromClassShape,
                    ClassShape* pToClassShape, CbColorRef penColor = Cb_RGB(0,
                    0, 0), CbColorRef textColor = Cb_RGB(0, 0, 0));
    ConnectionShape(ConnectionShape* pConnectionShape);
    virtual ~ConnectionShape();
    int CountNewRouting();
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual ConnectionShape* GetConnectionShape();
    virtual int GetOwned() const;
    void Hide();
    bool IsFromAtBottom() const;
    bool IsFromAtLeft() const;
    bool IsFromAtRight() const;
    bool IsFromAtTop() const;
    bool IsToAtBottom() const;
    bool IsToAtLeft() const;
    bool IsToAtRight() const;
    bool IsToAtTop() const;
    virtual void MakeNewRouting();
    virtual int OnEditAttributes(bool checkOnly = false) = 0;
    virtual bool PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                              CbPoint pointLP);
    void RecalculateRect();
    virtual void ReplaceReference(ConnectionSegment* pOld,
                                  ConnectionSegment* pNew);
    void Show();
    bool SlideEndpoint(bool atStart, CbPoint target, bool moveSiblings);
    void UpdateEndPoint(const CbPoint& endPoint);
    void UpdateStartPoint(const CbPoint& startPoint);
    const CbPoint& GetEndPoint() const;
    void SetEndPoint(const CbPoint& rEndPoint);
    bool GetInitial();
    void SetInitial(bool initial);
    CbSize GetMinEndSize();
    void SetMinEndSize(const CbSize& rMinEndSize);
    CbSize GetMinStartSize();
    void SetMinStartSize(const CbSize& rMinStartSize);
    const CbPoint& GetStartPoint() const;
    void SetStartPoint(const CbPoint& rStartPoint);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CONNECTIONSHAPE_H_INLINES
#define _CONNECTIONSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
