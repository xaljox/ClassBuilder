/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          MultiOwnedMacroMethods.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MultiOwnedMacroMethods'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _MULTIOWNEDMACROMETHODS_H
#define _MULTIOWNEDMACROMETHODS_H

//@START_USER1
//@END_USER1



class MultiOwnedMacroMethods
    : public FromRelationMacroMethods
{
    CB_DECLARE_SERIAL(MultiOwnedMacroMethods)

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
    MultiOwnedMacroMethods();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MultiOwnedMacroMethods(FromRelationMacroMethods* pOld);
    virtual ~MultiOwnedMacroMethods();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MULTIOWNEDMACROMETHODS_H_INLINES
#define _MULTIOWNEDMACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
