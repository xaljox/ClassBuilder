/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FixedMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FixedMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _FIXEDMETHOD_H
#define _FIXEDMETHOD_H

//@START_USER1
//@END_USER1



class FixedMethod
    : public Method
{
    CB_DECLARE_SERIAL(FixedMethod)

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
    FixedMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FixedMethod(BaseClass* pBaseClass, Type* pType);
    virtual ~FixedMethod();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual int IsFixed() const;
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditExceptionSpecification(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _FIXEDMETHOD_H_INLINES
#define _FIXEDMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
