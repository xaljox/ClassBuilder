/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Destructor.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Destructor'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _DESTRUCTOR_H
#define _DESTRUCTOR_H

//@START_USER1
//@END_USER1



class Destructor
    : public Method
{
    CB_DECLARE_SERIAL(Destructor)
    RELATION_SINGLE_OWNED_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)

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
    Destructor();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Destructor(BaseClass* pBaseClass);
    Destructor(BaseClass* pBaseClass, Destructor* pDestructor);
    virtual ~Destructor();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void InitCode();
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _DESTRUCTOR_H_INLINES
#define _DESTRUCTOR_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
