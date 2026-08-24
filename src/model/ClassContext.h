/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassContext.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassContext'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSCONTEXT_H
#define _CLASSCONTEXT_H

//@START_USER1
//@END_USER1



class ClassContext
    : public Context
{
    CB_DECLARE_SERIAL(ClassContext)
    RELATION_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Class* pClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ClassContext();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassContext(Class* pClass, ContextDeclaration* pContextDeclaration);
    virtual ~ClassContext();
    virtual Gti* GetContextObject();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSCONTEXT_H_INLINES
#define _CLASSCONTEXT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
