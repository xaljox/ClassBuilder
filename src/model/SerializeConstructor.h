/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SerializeConstructor.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SerializeConstructor'
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
