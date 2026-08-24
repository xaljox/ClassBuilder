/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ExceptionSpecification.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ExceptionSpecification'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _EXCEPTIONSPECIFICATION_H
#define _EXCEPTIONSPECIFICATION_H

//@START_USER1
//@END_USER1



class ExceptionSpecification
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(ExceptionSpecification)
    RELATION_MULTI_OWNED_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    RELATION_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)

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
    ExceptionSpecification();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ExceptionSpecification(Method* pMethod);
    virtual ~ExceptionSpecification();
    CbString GetThrowString() const;
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoRemoved();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _EXCEPTIONSPECIFICATION_H_INLINES
#define _EXCEPTIONSPECIFICATION_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
