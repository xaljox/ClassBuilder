/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FromRelation.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FromRelation'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
