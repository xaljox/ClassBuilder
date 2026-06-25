/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindReverseValueTreeMethod.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindReverseValueTreeMethod'
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
#ifndef _FINDREVERSEVALUETREEMETHOD_H
#define _FINDREVERSEVALUETREEMETHOD_H

//@START_USER1
//@END_USER1



class FindReverseValueTreeMethod
    : public FindMethod
{
    CB_DECLARE_SERIAL(FindReverseValueTreeMethod)
    RELATION_SINGLE_OWNED_PASSIVE(ValueTree, ValueTree, FindReverseValueTreeMethod, FindReverseValueTreeMethod)

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
    FindReverseValueTreeMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FindReverseValueTreeMethod(ValueTree* pValueTree);
    virtual ~FindReverseValueTreeMethod();
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
#ifndef _FINDREVERSEVALUETREEMETHOD_H_INLINES
#define _FINDREVERSEVALUETREEMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
