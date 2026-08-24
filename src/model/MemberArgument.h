/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MemberArgument.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MemberArgument'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
