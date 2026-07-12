/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindValueTreeMethod.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindValueTreeMethod'
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
#ifndef _FINDVALUETREEMETHOD_H
#define _FINDVALUETREEMETHOD_H

//@START_USER1
//@END_USER1



class FindValueTreeMethod
    : public FindMethod
{
    CB_DECLARE_SERIAL(FindValueTreeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindValueTreeMethod, FindValueTreeMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(ValueTree* pValueTree);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    FindValueTreeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FindValueTreeMethod(ValueTree* pValueTree);
    virtual ~FindValueTreeMethod();
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
#ifndef _FINDVALUETREEMETHOD_H_INLINES
#define _FINDVALUETREEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
