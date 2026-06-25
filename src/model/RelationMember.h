/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationMember.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RelationMember'
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
#ifndef _RELATIONMEMBER_H
#define _RELATIONMEMBER_H

//@START_USER1
//@END_USER1


/*@NOTE_1677
For relations implemented differently e.g. binary tree, relate the member with the
relation.
*/

class RelationMember
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(RelationMember)
    RELATION_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    RELATION_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Relation* pRelation, Member* pMember);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    RelationMember();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    RelationMember(Relation* pRelation, Member* pMember);
    virtual ~RelationMember();
    virtual void Delete();
    virtual CbString EpilogSetMemberMethod();
    virtual int GetImplementation();
    virtual CbString InitCodeFindMethod(FindMethod* pFindMethod,
                                        Argument* pArgument);
    virtual CbString PrologueSetMemberMethod();
    virtual void CleanupReferences();
    bool IsAvlTree() const;
    bool IsUniqueValueTree() const;
    bool IsValueTree() const;
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _RELATIONMEMBER_H_INLINES
#define _RELATIONMEMBER_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
