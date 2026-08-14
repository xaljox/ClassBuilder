/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberMethod.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MemberMethod'
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
#ifndef _MEMBERMETHOD_H
#define _MEMBERMETHOD_H

//@START_USER1
//@END_USER1



class MemberMethod
    : public Method
{
    CB_DECLARE_SERIAL(MemberMethod)
    RELATION_MULTI_OWNED_PASSIVE(Member, Member, MemberMethod, Method)

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
    MemberMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MemberMethod(Member* pMember, Type* pType);
    virtual ~MemberMethod();
    virtual void Add();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual CbString GetEndContextDeclaration();
    virtual CbString GetEndContextImplementation();
    virtual CbString GetStartContextDeclaration();
    virtual CbString GetStartContextImplementation();
    virtual int OnAddArgument(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MEMBERMETHOD_H_INLINES
#define _MEMBERMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
