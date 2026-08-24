/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassGroupContext.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassGroupContext'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSGROUPCONTEXT_H
#define _CLASSGROUPCONTEXT_H

//@START_USER1
//@END_USER1



class ClassGroupContext
    : public Context
{
    CB_DECLARE_SERIAL(ClassGroupContext)
    RELATION_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(ClassGroup* pClassGroup);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ClassGroupContext();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassGroupContext(ClassGroup* pClassGroup,
                      ContextDeclaration* pContextDeclaration);
    virtual ~ClassGroupContext();
    virtual Gti* GetContextObject();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSGROUPCONTEXT_H_INLINES
#define _CLASSGROUPCONTEXT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
