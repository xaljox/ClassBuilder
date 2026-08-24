/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SerializeConstructor.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SerializeConstructor'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _SERIALIZECONSTRUCTOR_H
#define _SERIALIZECONSTRUCTOR_H

//@START_USER1
//@END_USER1



class SerializeConstructor
    : public Constructor
{
    CB_DECLARE_SERIAL(SerializeConstructor)
    RELATION_SINGLE_OWNED_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)

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
    SerializeConstructor();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SerializeConstructor(BaseClass* pBaseClass);
    virtual ~SerializeConstructor();
    virtual void InitCode();
    virtual void InitInit();
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SERIALIZECONSTRUCTOR_H_INLINES
#define _SERIALIZECONSTRUCTOR_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
