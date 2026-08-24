/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RelationAssociationStartSegment.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RelationAssociationStartSegment'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _RELATIONASSOCIATIONSTARTSEGMENT_H
#define _RELATIONASSOCIATIONSTARTSEGMENT_H

//@START_USER1
//@END_USER1



class RelationAssociationStartSegment
    : public ConnectionSegment
{
    CB_DECLARE_SERIAL(RelationAssociationStartSegment)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    RelationAssociationStartSegment();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    RelationAssociationStartSegment(ConnectionSegment* pOld);
    virtual ~RelationAssociationStartSegment();
    virtual void Draw(CbPainter& painter);
    virtual CbPoint GetSelectedPoint();
    virtual bool IsReplaced();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _RELATIONASSOCIATIONSTARTSEGMENT_H_INLINES
#define _RELATIONASSOCIATIONSTARTSEGMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
