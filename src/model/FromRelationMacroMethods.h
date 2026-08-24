/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          FromRelationMacroMethods.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FromRelationMacroMethods'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _FROMRELATIONMACROMETHODS_H
#define _FROMRELATIONMACROMETHODS_H

//@START_USER1
//@END_USER1



class FromRelationMacroMethods
    : public MacroMethods
{
    CB_DECLARE_SERIAL(FromRelationMacroMethods)
    RELATION_SINGLE_OWNED_PASSIVE(FromRelation, FromRelation, FromRelationMacroMethods, FromRelationMacroMethods)

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
    void ConstructorInclude(FromRelation* pFromRelation);
    void DestructorInclude();
    void ReplaceConstructorInclude(FromRelationMacroMethods* pOld);
    void SerializeConstructorInclude();

protected:
    FromRelationMacroMethods();
    FromRelationMacroMethods(FromRelationMacroMethods* pOld);
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FromRelationMacroMethods(FromRelation* pFromRelation);
    virtual ~FromRelationMacroMethods();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _FROMRELATIONMACROMETHODS_H_INLINES
#define _FROMRELATIONMACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
