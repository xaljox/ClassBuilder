/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DestructorIncludeMethod.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DestructorIncludeMethod'
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
#ifndef _DESTRUCTORINCLUDEMETHOD_H
#define _DESTRUCTORINCLUDEMETHOD_H

//@START_USER1
//@END_USER1



class DestructorIncludeMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(DestructorIncludeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Destructor* pDestructor);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    DestructorIncludeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DestructorIncludeMethod(Destructor* refDestructor);
    virtual ~DestructorIncludeMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _DESTRUCTORINCLUDEMETHOD_H_INLINES
#define _DESTRUCTORINCLUDEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
