/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          GetMemberMethod.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'GetMemberMethod'
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
#ifndef _GETMEMBERMETHOD_H
#define _GETMEMBERMETHOD_H

//@START_USER1
//@END_USER1



class GetMemberMethod
    : public MemberMethod
{
    CB_DECLARE_SERIAL(GetMemberMethod)
    RELATION_SINGLE_OWNED_PASSIVE(Member, Member, GetMemberMethod, GetMemberMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Member* pMember);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    GetMemberMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    GetMemberMethod(Member* pMember);
    virtual ~GetMemberMethod();
    virtual void InitCode();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _GETMEMBERMETHOD_H_INLINES
#define _GETMEMBERMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
