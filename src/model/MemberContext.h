/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberContext.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MemberContext'
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
#ifndef _MEMBERCONTEXT_H
#define _MEMBERCONTEXT_H

//@START_USER1
//@END_USER1



class MemberContext
    : public Context
{
    CB_DECLARE_SERIAL(MemberContext)
    RELATION_MULTI_OWNED_PASSIVE(Member, Member, MemberContext, MemberContext)

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
    MemberContext();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MemberContext(Member* pMember, ContextDeclaration* pContextDeclaration);
    virtual ~MemberContext();
    virtual Gti* GetContextObject();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MEMBERCONTEXT_H_INLINES
#define _MEMBERCONTEXT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
