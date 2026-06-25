/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberAndMethodGroup.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MemberAndMethodGroup'
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
#ifndef _MEMBERANDMETHODGROUP_H
#define _MEMBERANDMETHODGROUP_H

//@START_USER1
//@END_USER1



class MemberAndMethodGroup
    : public Group
{
    CB_DECLARE_SERIAL(MemberAndMethodGroup)
    RELATION_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    RELATION_MULTI_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)
    RELATION_MULTI_OWNED_ACTIVE(MemberAndMethodGroup, MemberAndMethodGroup, MemberAndMethodGroupContext, MemberAndMethodGroupContext)
    RELATION_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(BaseClass* pBaseClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    MemberAndMethodGroup();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MemberAndMethodGroup(BaseClass* pBaseClass);
    virtual ~MemberAndMethodGroup();
    virtual void Add();
    static int CompareName(MemberAndMethodGroup* pA, MemberAndMethodGroup* pB);
    static int ComparePhase(MemberAndMethodGroup* pA, MemberAndMethodGroup* pB);
    virtual Context* CreateContext(ContextDeclaration* pContextDeclaration);
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual Context* GetFirstContext();
    virtual Context* GetNextContext(Context* pContextPos);
    virtual int OnAddConstructor(bool checkOnly = false);
    virtual int OnAddIsClassMethods(bool checkOnly = false);
    virtual int OnAddMember(bool checkOnly = false);
    virtual int OnAddMethod(bool checkOnly = false);
    virtual int OnAddVirtuals(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    void SetIcon();
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MEMBERANDMETHODGROUP_H_INLINES
#define _MEMBERANDMETHODGROUP_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
