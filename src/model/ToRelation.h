/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ToRelation.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ToRelation'
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
#ifndef _TORELATION_H
#define _TORELATION_H

//@START_USER1
//@END_USER1



class ToRelation
    : public Gti
{
    CB_DECLARE_SERIAL(ToRelation)
    RELATION_SINGLE_OWNED_ACTIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)
    RELATION_SINGLE_OWNED_PASSIVE(Relation, Relation, ToRelation, ToRelation)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Relation* pRelation);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ToRelation();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ToRelation(Relation* pRelation);
    virtual ~ToRelation();
    virtual void Add();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    void SetIcon();
    void SetItemText();
    virtual bool ShownByFilter(TreeViewModel* pTreeViewModel);
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _TORELATION_H_INLINES
#define _TORELATION_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
