/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ToRelationMacroMethods.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ToRelationMacroMethods'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _TORELATIONMACROMETHODS_H
#define _TORELATIONMACROMETHODS_H

//@START_USER1
//@END_USER1



class ToRelationMacroMethods
    : public MacroMethods
{
    CB_DECLARE_SERIAL(ToRelationMacroMethods)
    RELATION_SINGLE_OWNED_PASSIVE(ToRelation, ToRelation, ToRelationMacroMethods, ToRelationMacroMethods)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    virtual Gti* GetGtiParent();
    virtual Relation* GetRelation();
    void ConstructorInclude(ToRelation* pToRelation);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ToRelationMacroMethods();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ToRelationMacroMethods(ToRelation* pToRelation);
    virtual ~ToRelationMacroMethods();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _TORELATIONMACROMETHODS_H_INLINES
#define _TORELATIONMACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
