/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ReplaceConstructorIncludeMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ReplaceConstructorIncludeMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _REPLACECONSTRUCTORINCLUDEMETHOD_H
#define _REPLACECONSTRUCTORINCLUDEMETHOD_H

//@START_USER1
//@END_USER1



class ReplaceConstructorIncludeMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(ReplaceConstructorIncludeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(ReplaceConstructor* pReplaceConstructor);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ReplaceConstructorIncludeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ReplaceConstructorIncludeMethod(ReplaceConstructor* pReplaceConstructor);
    virtual ~ReplaceConstructorIncludeMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _REPLACECONSTRUCTORINCLUDEMETHOD_H_INLINES
#define _REPLACECONSTRUCTORINCLUDEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
