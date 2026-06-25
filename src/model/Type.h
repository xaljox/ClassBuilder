/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Type.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Type'
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
#ifndef _TYPE_H
#define _TYPE_H

//@START_USER1
//@END_USER1



class Type
    : public Gti
{
    CB_DECLARE_SERIAL(Type)
    RELATION_MULTI_OWNED_ACTIVE(Type, Type, Variable, Variable)
    RELATION_MULTI_OWNED_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;

protected:

public:

// Methods
private:
    void DropOnClass(bool ctrlKeyDown, Gti* pGtiDrop);
    void DropOnMethod(bool ctrlKeyDown, Gti* pGtiDrop);
    void DropOnOtherTypes(bool ctrlKeyDown, Gti* pGtiDrop);
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void ReplaceConstructorInclude(Type* pOld);
    void SerializeConstructorInclude();

protected:
    Type();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Type(DataModelDoc* pDataModelDoc);
    Type(Type* pOld);
    Type(DataModelDoc* pDataModelDoc, Type* pType);
    virtual ~Type();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    CbString GetFirstLowerName();
    CbString GetFirstUpperName();
    virtual CbString GetTemplate();
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual CbString GetName();
    void SetName(const CbString& rName);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _TYPE_H_INLINES
#define _TYPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
