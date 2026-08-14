/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DependencyStartSegment.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DependencyStartSegment'
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
