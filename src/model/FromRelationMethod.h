/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FromRelationMethod.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FromRelationMethod'
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
#ifndef _FROMRELATIONMETHOD_H
#define _FROMRELATIONMETHOD_H

//@START_USER1
//@END_USER1



class FromRelationMethod
    : public Method
{
    CB_DECLARE_SERIAL(FromRelationMethod)
    RELATION_MULTI_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMethod, Method)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(FromRelation* pFromRelation);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    FromRelationMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FromRelationMethod(FromRelation* pFromRelation, Type* pType);
    virtual ~FromRelationMethod();
    virtual void Add();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual int OnAddArgument(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _FROMRELATIONMETHOD_H_INLINES
#define _FROMRELATIONMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
