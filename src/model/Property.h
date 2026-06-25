/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Property.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Property'
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
#ifndef _PROPERTY_H
#define _PROPERTY_H

//@START_USER1
//@END_USER1



class Property
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(Property)
    RELATION_AVLTREE_OWNED_PASSIVE(DataModelDocObject, DataModelDocObject, Property, Property)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDocObject* pDataModelDocObject);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Property();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Property(DataModelDocObject* pDataModelDocObject, CbString name);
    virtual ~Property();
    const CbString& GetName() const;
    void SetName(const CbString& rName);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _PROPERTY_H_INLINES
#define _PROPERTY_H_INLINES

/*@NOTE_36068
Returns the value of member '_name'.
*/
inline const CbString& Property::GetName() const
{//@CODE_36068
    return _name;
}//@CODE_36068



//@START_USER3
//@END_USER3

#endif
#endif
