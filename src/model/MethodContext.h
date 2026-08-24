/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MethodContext.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MethodContext'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _METHODCONTEXT_H
#define _METHODCONTEXT_H

//@START_USER1
//@END_USER1



class MethodContext
    : public Context
{
    CB_DECLARE_SERIAL(MethodContext)
    RELATION_MULTI_OWNED_PASSIVE(Method, Method, MethodContext, MethodContext)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Method* pMethod);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    MethodContext();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MethodContext(Method* pMethod, ContextDeclaration* pContextDeclaration);
    virtual ~MethodContext();
    virtual Gti* GetContextObject();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _METHODCONTEXT_H_INLINES
#define _METHODCONTEXT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
