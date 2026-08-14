/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MultiMacroMethods.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MultiMacroMethods'
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
#ifndef _MULTIMACROMETHODS_H
#define _MULTIMACROMETHODS_H

//@START_USER1
//@END_USER1



class MultiMacroMethods
    : public FromRelationMacroMethods
{
    CB_DECLARE_SERIAL(MultiMacroMethods)

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
    MultiMacroMethods();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MultiMacroMethods(FromRelationMacroMethods* pOld);
    virtual ~MultiMacroMethods();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MULTIMACROMETHODS_H_INLINES
#define _MULTIMACROMETHODS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
