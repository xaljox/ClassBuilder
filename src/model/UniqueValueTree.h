/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UniqueValueTree.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UniqueValueTree'
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
#ifndef _UNIQUEVALUETREE_H
#define _UNIQUEVALUETREE_H

//@START_USER1
//@END_USER1


/*@NOTE_1701
Implement a relation as binary tree. The depth of the tree is at most 32 levels, it 
branches on the value of a certain bit, level one uses the first bit, level 2 the second
bit, ... etc. It uses an associated member as key. A key may exists only once in a relation.
*/

class UniqueValueTree
    : public RelationMember
{
    CB_DECLARE_SERIAL(UniqueValueTree)
    RELATION_SINGLE_OWNED_ACTIVE(UniqueValueTree, UniqueValueTree, FindUniqueValueTreeMethod, FindUniqueValueTreeMethod)

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
    UniqueValueTree();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    UniqueValueTree(Relation* pRelation, Member* pMember);
    virtual ~UniqueValueTree();
    virtual int GetImplementation();
    virtual CbString InitCodeFindMethod(FindMethod* pFindMethod,
                                        Argument* pArgument);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _UNIQUEVALUETREE_H_INLINES
#define _UNIQUEVALUETREE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
