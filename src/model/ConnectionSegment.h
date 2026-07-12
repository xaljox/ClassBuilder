/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ConnectionSegment.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ConnectionSegment'
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
#ifndef _CONNECTIONSEGMENT_H
#define _CONNECTIONSEGMENT_H

//@START_USER1
//@END_USER1



class ConnectionSegment
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(ConnectionSegment)
    RELATION_MULTI_OWNED_PASSIVE(ConnectionShape, ConnectionShape, ConnectionSegment, ConnectionSegment)

//@START_USER2
//@END_USER2

// Members
private:
    CbSize _size;

protected:
    UndoBase* _pUndoBase;

public:

// Methods
private:
    void ConstructorInclude(ConnectionShape* pConnectionShape);
    void DestructorInclude();
    void ReplaceConstructorInclude(ConnectionSegment* pOld);
    void SerializeConstructorInclude();

protected:
    ConnectionSegment();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ConnectionSegment(ConnectionShape* pConnectionShape, const CbSize& size);
    ConnectionSegment(ConnectionShape* pConnectionShape);
    ConnectionSegment(ConnectionShape* pConnectionShape,
                      ConnectionSegment* pConnectionSegment);
    ConnectionSegment(ConnectionSegment* pOld);
    virtual ~ConnectionSegment();
    bool CanMove(const CbSize& size);
    virtual void Draw(CbPainter& painter);
    void DrawSelectedRect(CbPainter& painter, CbColorRef color);
    CbPoint GetEndPoint();
    virtual CbPoint GetLineEndPoint();
    virtual CbPoint GetLineStartPoint();
    virtual CbPoint GetSelectedPoint();
    CbPoint GetStartPoint();
    virtual bool IsReplaced();
    bool Move(const CbSize& size);
    bool MoveAndAdjust(const CbSize& delta, bool moveCollinearSiblings);
    bool PointInShape(CbPoint pointLP);
    virtual void ReplaceReference(ConnectionSegment* pOld,
                                  ConnectionSegment* pNew);
    bool SetCursor(const CbRect& rect);
    CbSize GetSize();
    void SetSize(const CbSize& rSize);
    virtual void CleanupReferences();
    bool IsDependencyEndSegment() const;
    bool IsDependencyStartSegment() const;
    bool IsInheritEndSegment() const;
    bool IsInheritStartSegment() const;
    bool IsRelationAggregationStartSegment() const;
    bool IsRelationAssociationStartSegment() const;
    bool IsRelationMultiEndSegment() const;
    bool IsRelationSingleEndSegment() const;
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CONNECTIONSEGMENT_H_INLINES
#define _CONNECTIONSEGMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
