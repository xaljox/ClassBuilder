/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MacroMethod.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MacroMethod'
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
#ifndef _MACROMETHOD_H
#define _MACROMETHOD_H

//@START_USER1
//@END_USER1



class MacroMethod
    : public Method
{
    CB_DECLARE_SERIAL(MacroMethod)
    RELATION_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(MacroMethods* pMacroMethods);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    MacroMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MacroMethod(MacroMethods* pMacroMethods, BaseClass* pBaseClass, Type* pType);
    virtual ~MacroMethod();
    virtual void Add();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void InitCode();
    virtual int IsFixed() const;
    virtual bool IsNonMacroMethod() const;
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnEditExceptionSpecification(bool checkOnly = false);
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
#ifndef _MACROMETHOD_H_INLINES
#define _MACROMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
