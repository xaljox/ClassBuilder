/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindAvlTreeMethod.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindAvlTreeMethod'
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
#ifndef _FINDAVLTREEMETHOD_H
#define _FINDAVLTREEMETHOD_H

//@START_USER1
//@END_USER1



class FindAvlTreeMethod
    : public FindMethod
{
    CB_DECLARE_SERIAL(FindAvlTreeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(AvlTree, AvlTree, FindAvlTreeMethod, FindAvlTreeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(AvlTree* pAvlTree);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    FindAvlTreeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FindAvlTreeMethod(AvlTree* pAvlTree);
    virtual ~FindAvlTreeMethod();
    virtual void InitCode();
    virtual int IsFixed() const;
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _FINDAVLTREEMETHOD_H_INLINES
#define _FINDAVLTREEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
