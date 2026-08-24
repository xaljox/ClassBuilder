/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FindUniqueValueTreeMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindUniqueValueTreeMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _FINDUNIQUEVALUETREEMETHOD_H
#define _FINDUNIQUEVALUETREEMETHOD_H

//@START_USER1
//@END_USER1



class FindUniqueValueTreeMethod
    : public FindMethod
{
    CB_DECLARE_SERIAL(FindUniqueValueTreeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(UniqueValueTree* pUniqueValueTree);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    FindUniqueValueTreeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FindUniqueValueTreeMethod(UniqueValueTree* pUniqueValueTree);
    virtual ~FindUniqueValueTreeMethod();
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
#ifndef _FINDUNIQUEVALUETREEMETHOD_H_INLINES
#define _FINDUNIQUEVALUETREEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
