/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          FindMethod.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'FindMethod'
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
#ifndef _FINDMETHOD_H
#define _FINDMETHOD_H

//@START_USER1
//@END_USER1



class FindMethod
    : public FromRelationMethod
{
    CB_DECLARE_SERIAL(FindMethod)

//@START_USER2
//@END_USER2

// Members
private:
    bool _next;
    bool _reverse;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    FindMethod();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    FindMethod(FromRelation* pFromRelation, bool reverse = false);
    virtual ~FindMethod();
    virtual void InitCode();
    virtual int OnEditAttributes(bool checkOnly = false);
    bool GetNext();
    void SetNext(bool next);
    bool GetReverse();
    void SetReverse(bool reverse);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _FINDMETHOD_H_INLINES
#define _FINDMETHOD_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
