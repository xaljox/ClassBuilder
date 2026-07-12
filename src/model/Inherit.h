/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Inherit.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Inherit'
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
#ifndef _INHERIT_H
#define _INHERIT_H

//@START_USER1
//@END_USER1



class Inherit
    : public Gti
{
    CB_DECLARE_SERIAL(Inherit)
    RELATION_MULTI_OWNED_ACTIVE(Inherit, Inherit, InheritShape, InheritShape)
    RELATION_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Inherit, Inherit)
    RELATION_MULTI_OWNED_PASSIVE(ExternClass, ExternClass, Inherit, Inherit)

//@START_USER2
//@END_USER2

// Members
private:
    AccessType _access;
    CbString _note;
    bool _virtual;
    CbString _template;

protected:

public:

// Methods
private:
    void ConstructorInclude(BaseClass* pBaseClass, ExternClass* pExternClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Inherit();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Inherit(ExternClass* pExternClass, BaseClass* pBaseClass,
            AccessType access = PUBLIC);
    Inherit(ExternClass* pExternClass, BaseClass* pBaseClass, Inherit* pInherit);
    virtual ~Inherit();
    virtual void Add();
    virtual void Delete();
    InheritShape* FindInheritShape(ClassDiagram* pClassDiagram);
    CbString GetBaseName();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    virtual bool ShownByFilter(TreeViewModel* pTreeViewModel);
    virtual void Update();
    AccessType GetAccess();
    void SetAccess(AccessType access);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    CbString GetTemplate();
    void SetTemplate(const CbString& rTemplate);
    bool GetVirtual();
    void SetVirtual(bool val);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _INHERIT_H_INLINES
#define _INHERIT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
