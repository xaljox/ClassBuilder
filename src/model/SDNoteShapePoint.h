/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SDNoteShapePoint.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SDNoteShapePoint'
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
#ifndef _SDNOTESHAPEPOINT_H
#define _SDNOTESHAPEPOINT_H

//@START_USER1
//@END_USER1



class SDNoteShapePoint
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(SDNoteShapePoint)
    RELATION_MULTI_OWNED_PASSIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)

//@START_USER2
//@END_USER2

// Members
private:
    CbPoint _point;
    UndoBase* _pLastUndoBase;

protected:

public:

// Methods
private:
    void ConstructorInclude(SDNoteShape* pSDNoteShape);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    SDNoteShapePoint();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SDNoteShapePoint(SDNoteShape* pSDNoteShape, const CbPoint& point);
    virtual ~SDNoteShapePoint();
    bool IsNotJustMoved() const;
    void MarkAsJustMoved();
    bool PointInShape(const CbPoint& pointLP);
    const CbPoint& GetPoint();
    void SetPoint(const CbPoint& rPoint);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SDNOTESHAPEPOINT_H_INLINES
#define _SDNOTESHAPEPOINT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
