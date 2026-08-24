/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ReplaceConstructor.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ReplaceConstructor'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _REPLACECONSTRUCTOR_H
#define _REPLACECONSTRUCTOR_H

//@START_USER1
//@END_USER1



class ReplaceConstructor
    : public Constructor
{
    CB_DECLARE_SERIAL(ReplaceConstructor)
    RELATION_SINGLE_OWNED_ACTIVE(ReplaceConstructor, ReplaceConstructor, ReplaceConstructorIncludeMethod, ReplaceConstructorIncludeMethod)

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
    ReplaceConstructor();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ReplaceConstructor(BaseClass* pBaseClass);
    virtual ~ReplaceConstructor();
    virtual void InitCode();
    virtual void InitInit();
    virtual void NotifyAddMember(Member* pMember);
    virtual void NotifyRemoveMember(Member* pMember);
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _REPLACECONSTRUCTOR_H_INLINES
#define _REPLACECONSTRUCTOR_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
