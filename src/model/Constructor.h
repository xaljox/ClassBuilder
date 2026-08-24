/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Constructor.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Constructor'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CONSTRUCTOR_H
#define _CONSTRUCTOR_H

//@START_USER1
//@END_USER1



class Constructor
    : public Method
{
    CB_DECLARE_SERIAL(Constructor)

//@START_USER2
//@END_USER2

// Members
private:
    bool _explicit;

protected:
    CbString _init;

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Constructor();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Constructor(BaseClass* pBaseClass);
    Constructor(BaseClass* pBaseClass, Constructor* pConstructor);
    virtual ~Constructor();
    void CreateArguments();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    int ExplicitAllowed();
    virtual CbString GetShapeText(VerbosityType verbosity = VERBOSITY_OFF);
    virtual int HasStringInCode(const CbString& string);
    virtual void InitCode();
    virtual void InitInit();
    virtual void NotifyAddMember(Member* pMember);
    virtual void NotifyRemoveMember(Member* pMember);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void ReplaceInCode(const CbString& oldString,
                               const CbString& newString);
    void ReplaceInInit(const CbString& oldString, const CbString& newString);
    virtual void SetItemText();
    bool GetExplicit() const;
    void SetExplicit(bool value);
    const CbString& GetInit();
    void SetInit(const CbString& rInit);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CONSTRUCTOR_H_INLINES
#define _CONSTRUCTOR_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
