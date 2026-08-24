/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          IsClassMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'IsClassMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _ISCLASSMETHOD_H
#define _ISCLASSMETHOD_H

//@START_USER1
//@END_USER1



class IsClassMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(IsClassMethod)
    RELATION_MULTI_OWNED_PASSIVE(Class, RefClass, IsClassMethod, IsClassMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Class* pRefClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    IsClassMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    IsClassMethod(Class* pClass, Class* pRefClass);
    virtual ~IsClassMethod();
    virtual void InitCode();
    virtual int OnDelete(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _ISCLASSMETHOD_H_INLINES
#define _ISCLASSMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
