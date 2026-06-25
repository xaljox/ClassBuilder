/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MacroMethods.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MacroMethods'
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
#ifndef _MACROMETHODS_H
#define _MACROMETHODS_H

//@START_USER1
//@END_USER1



class MacroMethods
    : public Gti
{
    RELATION_MULTI_OWNED_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    virtual Gti* GetGtiParent() = 0;
    virtual Relation* GetRelation() = 0;
    void ConstructorInclude();
    void DestructorInclude();
    void ReplaceConstructorInclude(MacroMethods* pOld);
    void SerializeConstructorInclude();

protected:
    MacroMethods();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MacroMethods(DataModelDoc* pDataModelDoc);
    MacroMethods(MacroMethods* pOld);
    virtual ~MacroMethods();
    virtual void Add();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MACROMETHODS_H_INLINES
#define _MACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
