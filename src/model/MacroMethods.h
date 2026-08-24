/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MacroMethods.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MacroMethods'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
