/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SerializeConstructorIncludeMethod.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SerializeConstructorIncludeMethod'
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
#ifndef _SERIALIZECONSTRUCTORINCLUDEMETHOD_H
#define _SERIALIZECONSTRUCTORINCLUDEMETHOD_H

//@START_USER1
//@END_USER1



class SerializeConstructorIncludeMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(SerializeConstructorIncludeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(SerializeConstructor* pSerializeConstructor);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    SerializeConstructorIncludeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SerializeConstructorIncludeMethod(SerializeConstructor* pSerializeConstructor);
    virtual ~SerializeConstructorIncludeMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SERIALIZECONSTRUCTORINCLUDEMETHOD_H_INLINES
#define _SERIALIZECONSTRUCTORINCLUDEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
