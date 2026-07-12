/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MetaGroup.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MetaGroup'
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
#ifndef _METAGROUP_H
#define _METAGROUP_H

//@START_USER1
//@END_USER1


/*@NOTE_28960
Class to give an extra posibilty for structuring the classes in the tree and in the 
documentation.
*/

class MetaGroup
    : public Group
{
    CB_DECLARE_SERIAL(MetaGroup)
    RELATION_MULTI_ACTIVE(MetaGroup, MetaGroup, ClassGroup, ClassGroup)
    RELATION_MULTI_OWNED_PASSIVE(DataModel, DataModel, MetaGroup, MetaGroup)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModel* pDataModel);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    MetaGroup();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MetaGroup(DataModel* pDataModel);
    virtual ~MetaGroup();
    virtual void Add();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual Gti* GetNext(Gti* pGti = 0);
    virtual int OnAddClassDiagram(bool checkOnly = false);
    virtual int OnAddGroup(bool checkOnly = false);
    virtual int OnAddMetaGroup(bool checkOnly = false);
    virtual int OnAddSequenceDiagram(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
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
#ifndef _METAGROUP_H_INLINES
#define _METAGROUP_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
