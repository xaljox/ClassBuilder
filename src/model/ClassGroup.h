/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassGroup.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassGroup'
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
#ifndef _CLASSGROUP_H
#define _CLASSGROUP_H

//@START_USER1
//@END_USER1



class ClassGroup
    : public Group
{
    CB_DECLARE_SERIAL(ClassGroup)
    RELATION_MULTI_ACTIVE(ClassGroup, ClassGroup, Class, Class)
    RELATION_MULTI_OWNED_ACTIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    RELATION_MULTI_PASSIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    RELATION_MULTI_PASSIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)

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
    ClassGroup();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassGroup(DataModel* pDataModel);
    ClassGroup(MetaGroup* pMetaGroup);
    virtual ~ClassGroup();
    virtual void Add();
    static int CompareName(ClassGroup* pA, ClassGroup* pB);
    static int ComparePhase(ClassGroup* pA, ClassGroup* pB);
    virtual Context* CreateContext(ContextDeclaration* pContextDeclaration);
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual Context* GetFirstContext();
    virtual Context* GetNextContext(Context* pContextPos);
    virtual int OnAddClass(bool checkOnly = false);
    virtual int OnAddClassDiagram(bool checkOnly = false);
    virtual int OnAddSequenceDiagram(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    virtual int SortOnName(bool checkOnly = false);
    virtual int SortOnPhase(bool checkOnly = false);
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASSGROUP_H_INLINES
#define _CLASSGROUP_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
