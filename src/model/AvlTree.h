/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          AvlTree.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'AvlTree'
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
#ifndef _AVLTREE_H
#define _AVLTREE_H

//@START_USER1
//@END_USER1



class AvlTree
    : public RelationMember
{
    CB_DECLARE_SERIAL(AvlTree)
    RELATION_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)
    RELATION_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindReverseAvlTreeMethod, FindReverseAvlTreeMethod)
    RELATION_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrSmallerAvlTreeMethod, FindEqualOrSmallerAvlTreeMethod)
    RELATION_SINGLE_OWNED_ACTIVE(AvlTree, AvlTree, FindEqualOrBiggerAvlTreeMethod, FindEqualOrBiggerAvlTreeMethod)

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
    AvlTree();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    AvlTree(Relation* pRelation, Member* pMember);
    virtual ~AvlTree();
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
#ifndef _AVLTREE_H_INLINES
#define _AVLTREE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
