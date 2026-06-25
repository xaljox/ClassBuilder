/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Argument.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Argument'
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
#ifndef _ARGUMENT_H
#define _ARGUMENT_H

//@START_USER1
//@END_USER1



class Argument
    : public Variable
{
    CB_DECLARE_SERIAL(Argument)
    RELATION_MULTI_OWNED_PASSIVE(Method, Method, Argument, Argument)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _default;
    CbString _path;

protected:

public:

// Methods
private:
    void ConstructorInclude(Method* pMethod);
    void DestructorInclude();
    void ReplaceConstructorInclude(Argument* pOld);
    void SerializeConstructorInclude();

protected:
    Argument();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Argument(Method* pMethod, Type* pType);
    Argument(Method* pMethod, Argument* pArgument);
    Argument(Argument* pOld);
    Argument(Method* pMethod, Type* pType, Argument* pArgument);
    virtual ~Argument();
    virtual void Add();
    Argument& CopyValuesFrom(Argument& rArgument);
    virtual void Delete();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    int IsSimilar(Argument* pArgument);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void OnUndoRedoRemoved();
    virtual void ReplaceInPath(const CbString& oldString,
                               const CbString& newString);
    virtual void SetName(const CbString& rName);
    virtual void Update();
    void UpdatePhaseMethod();
    const CbString& GetDefault();
    void SetDefault(const CbString& rDefault);
    const CbString& GetPath();
    void SetPath(const CbString& rPath);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _ARGUMENT_H_INLINES
#define _ARGUMENT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
