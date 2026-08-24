/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          InheritEndSegment.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'InheritEndSegment'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _INHERITENDSEGMENT_H
#define _INHERITENDSEGMENT_H

//@START_USER1
//@END_USER1



class InheritEndSegment
    : public ConnectionSegment
{
    CB_DECLARE_SERIAL(InheritEndSegment)

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
    InheritEndSegment();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    InheritEndSegment(ConnectionSegment* pOld);
    virtual ~InheritEndSegment();
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
#ifndef _INHERITENDSEGMENT_H_INLINES
#define _INHERITENDSEGMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
