/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MacroMethod.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MacroMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
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
