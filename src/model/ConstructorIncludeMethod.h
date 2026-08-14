/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ConstructorIncludeMethod.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ConstructorIncludeMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
* Philips Digital Video Systems, Eindhoven, The Netherlands.
* Distributed under the GNU General Public License (GPL)
*
\******************************************************************************/
#ifndef _CONSTRUCTORINCLUDEMETHOD_H
#define _CONSTRUCTORINCLUDEMETHOD_H

//@START_USER1
//@END_USER1



class ConstructorIncludeMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(ConstructorIncludeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)

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
    ConstructorIncludeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ConstructorIncludeMethod(Class* pClass);
    virtual ~ConstructorIncludeMethod();
    virtual void InitCode();
    void UpdateArguments();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CONSTRUCTORINCLUDEMETHOD_H_INLINES
#define _CONSTRUCTORINCLUDEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
