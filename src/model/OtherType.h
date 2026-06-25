/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          OtherType.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'OtherType'
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
#ifndef _OTHERTYPE_H
#define _OTHERTYPE_H

//@START_USER1
//@END_USER1



class OtherType
    : public Type
{
    CB_DECLARE_SERIAL(OtherType)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _declaration;
    bool _serializeMap;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    OtherType();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    OtherType(DataModelDoc* pDataModelDoc);
    OtherType(ExternClass* pOld);
    OtherType(DataModelDoc* pDataModelDoc, OtherType* pOtherType);
    virtual ~OtherType();
    virtual void Add();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    virtual void Update();
    const CbString& GetDeclaration();
    void SetDeclaration(const CbString& rDeclaration);
    bool GetSerializeMap();
    void SetSerializeMap(bool serializeMap);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _OTHERTYPE_H_INLINES
#define _OTHERTYPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
