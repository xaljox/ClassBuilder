/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ExternClass.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ExternClass'
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
#ifndef _EXTERNCLASS_H
#define _EXTERNCLASS_H

//@START_USER1
//@END_USER1



class ExternClass
    : public BaseClass
{
    CB_DECLARE_SERIAL(ExternClass)
    RELATION_MULTI_OWNED_ACTIVE(ExternClass, ExternClass, Inherit, Inherit)

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
    void ReplaceConstructorInclude(ExternClass* pOld);
    void SerializeConstructorInclude();

protected:
    ExternClass();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ExternClass(DataModelDoc* pDataModelDoc);
    ExternClass(Class* pOld);
    ExternClass(OtherType* pOld);
    ExternClass(DataModelDoc* pDataModelDoc, ExternClass* pExternClass);
    virtual ~ExternClass();
    virtual void Add();
    int IsBaseClass(BaseClass* pBaseClass);
    virtual int OnAddInherit(bool checkOnly = false);
    virtual int OnAddVirtuals(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _EXTERNCLASS_H_INLINES
#define _EXTERNCLASS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
