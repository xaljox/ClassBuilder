/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SingleMacroMethods.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SingleMacroMethods'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _SINGLEMACROMETHODS_H
#define _SINGLEMACROMETHODS_H

//@START_USER1
//@END_USER1



class SingleMacroMethods
    : public FromRelationMacroMethods
{
    CB_DECLARE_SERIAL(SingleMacroMethods)

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
    SingleMacroMethods();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SingleMacroMethods(FromRelationMacroMethods* pOld);
    virtual ~SingleMacroMethods();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SINGLEMACROMETHODS_H_INLINES
#define _SINGLEMACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
