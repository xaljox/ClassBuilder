/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Group.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Group'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _GROUP_H
#define _GROUP_H

//@START_USER1
//@END_USER1



class Group
    : public Gti
{
    CB_DECLARE_SERIAL(Group)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;
    CbString _note;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Group();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Group(DataModelDoc* refDataModelDoc);
    virtual ~Group();
    virtual int OnEditAttributes(bool checkOnly = false);
    const CbString& GetName();
    void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _GROUP_H_INLINES
#define _GROUP_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
