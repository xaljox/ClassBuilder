/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          NoteShapePoint.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'NoteShapePoint'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _NOTESHAPEPOINT_H
#define _NOTESHAPEPOINT_H

//@START_USER1
//@END_USER1


/*@NOTE_5009
A point to which this note connects.
*/

class NoteShapePoint
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(NoteShapePoint)
    RELATION_MULTI_OWNED_PASSIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)

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
    void ConstructorInclude(NoteShape* pNoteShape);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    NoteShapePoint();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    NoteShapePoint(NoteShape* pNoteShape, CbPoint point);
    virtual ~NoteShapePoint();
    bool IsNotJustMoved() const;
    void MarkAsJustMoved();
    bool PointInShape(CbPoint pointLP);
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
#ifndef _NOTESHAPEPOINT_H_INLINES
#define _NOTESHAPEPOINT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
