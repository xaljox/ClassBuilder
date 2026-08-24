/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FindEqualOrBiggerAvlTreeMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindEqualOrBiggerAvlTreeMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
