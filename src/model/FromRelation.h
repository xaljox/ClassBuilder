/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FromRelation.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FromRelation'
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
#ifndef _FROMRELATION_H
#define _FROMRELATION_H

//@START_USER1
//@END_USER1



class FromRelation
    : public Gti
{
    CB_DECLARE_SERIAL(FromRelation)
    RELATION_MULTI_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMethod, Method)
    RELATION_SINGLE_OWNED_ACTIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)
    RELATION_SINGLE_OWNED_PASSIVE(Relation, Relation, FromRelation, FromRelation)

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
    FromRelation();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FromRelation(Relation* pRelation);
    virtual ~FromRelation();
    virtual void Add();
    virtual int OnAddMethod(bool checkOnly = false);
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
#ifndef _FROMRELATION_H_INLINES
#define _FROMRELATION_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
