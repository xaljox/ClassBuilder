/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          DependencyStartSegment.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DependencyStartSegment'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _DEPENDENCYSTARTSEGMENT_H
#define _DEPENDENCYSTARTSEGMENT_H

//@START_USER1
//@END_USER1



class DependencyStartSegment
    : public ConnectionSegment
{
    CB_DECLARE_SERIAL(DependencyStartSegment)

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
    DependencyStartSegment();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DependencyStartSegment(ConnectionSegment* pOld);
    virtual ~DependencyStartSegment();
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
#ifndef _DEPENDENCYSTARTSEGMENT_H_INLINES
#define _DEPENDENCYSTARTSEGMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
