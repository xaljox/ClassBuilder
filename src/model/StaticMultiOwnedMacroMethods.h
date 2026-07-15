/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          StaticMultiOwnedMacroMethods.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'StaticMultiOwnedMacroMethods'
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
#ifndef _STATICMULTIOWNEDMACROMETHODS_H
#define _STATICMULTIOWNEDMACROMETHODS_H

//@START_USER1
//@END_USER1



class StaticMultiOwnedMacroMethods
    : public FromRelationMacroMethods
{
    CB_DECLARE_SERIAL(StaticMultiOwnedMacroMethods)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    StaticMultiOwnedMacroMethods();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    StaticMultiOwnedMacroMethods(FromRelationMacroMethods* pOld);
    virtual ~StaticMultiOwnedMacroMethods();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _STATICMULTIOWNEDMACROMETHODS_H_INLINES
#define _STATICMULTIOWNEDMACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
