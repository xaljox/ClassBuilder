/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Relation.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Relation'
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
#ifndef _RELATION_H
#define _RELATION_H

//@START_USER1
//@END_USER1



class Relation
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(Relation)
    RELATION_SINGLE_OWNED_ACTIVE(Relation, Relation, FromRelation, FromRelation)
    RELATION_SINGLE_OWNED_ACTIVE(Relation, Relation, ToRelation, ToRelation)
    RELATION_SINGLE_OWNED_ACTIVE(Relation, Relation, RelationMember, RelationMember)
    RELATION_MULTI_OWNED_ACTIVE(Relation, Relation, RelationShape, RelationShape)
    RELATION_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
    RELATION_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _fromName;
    CbString _note;
    CbString _toName;
    bool _critical;
    bool _multi;
    bool _owned;
    bool _single;
    bool _static;
    bool _filter;

protected:

public:

// Methods
private:
    void ConstructorInclude(Class* pFromClass, Class* pToClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Relation();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Relation(Class* pFromClass, Class* pToClass, const CbString& fromName,
             const CbString& toName, int staticX, int multi, int single,
             int owned, int critical);
    Relation(Class* pFromClass, Class* pToClass, int staticX, int multi,
             int single, int owned, int critical);
    virtual ~Relation();
    virtual void Add();
    virtual void Delete();
    RelationShape* FindRelationShape(ClassDiagram* pClassDiagram);
    CbString GetFromClassName();
    int GetImplementation();
    CbString GetNotation();
    CbString GetToClassName();
    virtual void Update();
    void WriteFromMacro(CbString& macro, const CbString start,
                        int enableOwned = 1);
    void WriteToMacro(CbString& macro, const CbString start,
                      int enableOwned = 1);
    bool GetCritical() const;
    void SetCritical(bool critical);
    bool GetFilter() const;
    void SetFilter(bool filter);
    const CbString& GetFromName();
    void SetFromName(const CbString& rFromName);
    bool GetMulti() const;
    void SetMulti(bool multi);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    bool GetOwned() const;
    void SetOwned(bool owned);
    bool GetSingle() const;
    void SetSingle(bool single);
    bool GetStatic() const;
    void SetStatic(bool val);
    const CbString& GetToName();
    void SetToName(const CbString& rToName);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _RELATION_H_INLINES
#define _RELATION_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
