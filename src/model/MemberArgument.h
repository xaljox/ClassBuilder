/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberArgument.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MemberArgument'
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
#ifndef _MEMBERARGUMENT_H
#define _MEMBERARGUMENT_H

//@START_USER1
//@END_USER1



class MemberArgument
    : public Argument
{
    CB_DECLARE_SERIAL(MemberArgument)
    RELATION_MULTI_OWNED_PASSIVE(Member, Member, MemberArgument, MemberArgument)

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
    MemberArgument();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MemberArgument(Method* pMethod, Member* pMember);
    virtual ~MemberArgument();
    void UpdateName();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MEMBERARGUMENT_H_INLINES
#define _MEMBERARGUMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
