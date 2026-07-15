/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SerializeRelationsMethod.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SerializeRelationsMethod'
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
#ifndef _SERIALIZERELATIONSMETHOD_H
#define _SERIALIZERELATIONSMETHOD_H

//@START_USER1
//@END_USER1



class SerializeRelationsMethod
    : public FixedMethod
{
    CB_DECLARE_SERIAL(SerializeRelationsMethod)

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
    SerializeRelationsMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SerializeRelationsMethod(BaseClass* pBaseClass);
    virtual ~SerializeRelationsMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SERIALIZERELATIONSMETHOD_H_INLINES
#define _SERIALIZERELATIONSMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
