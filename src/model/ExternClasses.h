/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ExternClasses.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ExternClasses'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _EXTERNCLASSES_H
#define _EXTERNCLASSES_H

//@START_USER1
//@END_USER1



class ExternClasses
    : public Gti
{
    CB_DECLARE_SERIAL(ExternClasses)
    RELATION_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ExternClasses();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ExternClasses(DataModelDoc* pDataModelDoc);
    virtual ~ExternClasses();
    virtual void Add();
    virtual Gti* GetNext(Gti* pGti = 0);
    int OnAddExternClass(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _EXTERNCLASSES_H_INLINES
#define _EXTERNCLASSES_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
