/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Actors.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Actors'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _ACTORS_H
#define _ACTORS_H

//@START_USER1
//@END_USER1



class Actors
    : public Gti
{
    CB_DECLARE_SERIAL(Actors)
    RELATION_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Actors, Actors)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Actors();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Actors(DataModelDoc* pDataModelDoc);
    virtual ~Actors();
    virtual void Add();
    virtual Gti* GetNext(Gti* pGti = 0);
    virtual void Update();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _ACTORS_H_INLINES
#define _ACTORS_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
