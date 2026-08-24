/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SaveReferencesMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SaveReferencesMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _SAVEREFERENCESMETHOD_H
#define _SAVEREFERENCESMETHOD_H

//@START_USER1
//@END_USER1



class SaveReferencesMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(SaveReferencesMethod)

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
    SaveReferencesMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SaveReferencesMethod(BaseClass* pBaseClass);
    virtual ~SaveReferencesMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SAVEREFERENCESMETHOD_H_INLINES
#define _SAVEREFERENCESMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
