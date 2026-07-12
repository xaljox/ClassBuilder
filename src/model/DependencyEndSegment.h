/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DependencyEndSegment.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DependencyEndSegment'
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
#ifndef _DEPENDENCYENDSEGMENT_H
#define _DEPENDENCYENDSEGMENT_H

//@START_USER1
//@END_USER1



class DependencyEndSegment
    : public ConnectionSegment
{
    CB_DECLARE_SERIAL(DependencyEndSegment)

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
    DependencyEndSegment();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DependencyEndSegment(ConnectionSegment* pOld);
    virtual ~DependencyEndSegment();
    virtual void Draw(CbPainter& painter);
    virtual CbPoint GetLineEndPoint();
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
#ifndef _DEPENDENCYENDSEGMENT_H_INLINES
#define _DEPENDENCYENDSEGMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
