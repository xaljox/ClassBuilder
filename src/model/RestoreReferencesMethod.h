/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RestoreReferencesMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RestoreReferencesMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _RESTOREREFERENCESMETHOD_H
#define _RESTOREREFERENCESMETHOD_H

//@START_USER1
//@END_USER1



class RestoreReferencesMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(RestoreReferencesMethod)

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
    RestoreReferencesMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    RestoreReferencesMethod(BaseClass* pBaseClass);
    virtual ~RestoreReferencesMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _RESTOREREFERENCESMETHOD_H_INLINES
#define _RESTOREREFERENCESMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
