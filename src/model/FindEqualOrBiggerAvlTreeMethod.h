/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindEqualOrBiggerAvlTreeMethod.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindEqualOrBiggerAvlTreeMethod'
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
#ifndef _FINDEQUALORBIGGERAVLTREEMETHOD_H
#define _FINDEQUALORBIGGERAVLTREEMETHOD_H

//@START_USER1
//@END_USER1



class FindEqualOrBiggerAvlTreeMethod
    : public FindMethod
{
    CB_DECLARE_SERIAL(FindEqualOrBiggerAvlTreeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(AvlTree* pAvlTree);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    FindEqualOrBiggerAvlTreeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FindEqualOrBiggerAvlTreeMethod(AvlTree* pAvlTree);
    virtual ~FindEqualOrBiggerAvlTreeMethod();
    virtual void InitCode();
    virtual int IsFixed() const;
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _FINDEQUALORBIGGERAVLTREEMETHOD_H_INLINES
#define _FINDEQUALORBIGGERAVLTREEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
