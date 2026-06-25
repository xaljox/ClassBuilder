/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          OtherTypes.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'OtherTypes'
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
#ifndef _OTHERTYPES_H
#define _OTHERTYPES_H

//@START_USER1
//@END_USER1



class OtherTypes
    : public Gti
{
    CB_DECLARE_SERIAL(OtherTypes)
    RELATION_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    OtherTypes();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    OtherTypes(DataModelDoc* pDataModelDoc);
    virtual ~OtherTypes();
    virtual void Add();
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _OTHERTYPES_H_INLINES
#define _OTHERTYPES_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
