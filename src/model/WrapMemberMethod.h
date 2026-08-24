/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          WrapMemberMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'WrapMemberMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _WRAPMEMBERMETHOD_H
#define _WRAPMEMBERMETHOD_H

//@START_USER1
//@END_USER1



class WrapMemberMethod
    : public MemberMethod
{
    CB_DECLARE_SERIAL(WrapMemberMethod)
    RELATION_MULTI_OWNED_PASSIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)

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
    WrapMemberMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    WrapMemberMethod(Member* pMember, Method* pMethod);
    virtual ~WrapMemberMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _WRAPMEMBERMETHOD_H_INLINES
#define _WRAPMEMBERMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
