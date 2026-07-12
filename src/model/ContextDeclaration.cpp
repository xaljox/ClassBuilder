/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ContextDeclaration.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ContextDeclaration'
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
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
#include "ClassBuilderDoc.h"
//@END_USER2


// Static members


/*@NOTE_25044
Constructor method.
*/
ContextDeclaration::ContextDeclaration(DataModel* pDataModel,
                                       CbString name) //@INIT_25044
    : DataModelDocObject(pDataModel->GetDataModelDoc())
    , _startContextDeclaration("")
    , _endContextDeclaration("")
    , _startContextImplementation("")
    , _endContextImplementation("")
    , _name(name)
    , _defineDeclaration()
    , _enableDefineDeclaration(false)
    , _note()
{//@CODE_25044
    ConstructorInclude(pDataModel);

    // Put in your own code
}//@CODE_25044


/*@NOTE_23515
Constructor needed for serialization, not meant to use for other purposes!
*/
ContextDeclaration::ContextDeclaration() //@INIT_23515
    : DataModelDocObject()
    , _name()
    , _defineDeclaration()
    , _enableDefineDeclaration(false)
    , _note()
{//@CODE_23515
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23515


/*@NOTE_23513
Destructor method.
*/
ContextDeclaration::~ContextDeclaration()
{//@CODE_23513
    DestructorInclude();

    // Put in your own code
}//@CODE_23513


/*@NOTE_26123
Returns a pointer to the context object who associates the current context declaration 
with 'pGti'.
*/
Context* ContextDeclaration::FindContext(Gti* pGti)
{//@CODE_26123
    ContextIterator iContext(this);
    while (++iContext)
    {
        if (iContext->GetContextObject() == pGti)
        {
            return iContext;
        }
    }

    return 0;
}//@CODE_26123


/*@NOTE_27289
Set the value of member '_defineDeclaration' to 'rDefineDeclaration'.
*/
void ContextDeclaration::SetDefineDeclaration(const CbString& rDefineDeclaration)
{//@CODE_27289
    if (_defineDeclaration != rDefineDeclaration)
    {
        SaveState();
        _defineDeclaration = rDefineDeclaration;
    }
}//@CODE_27289


/*@NOTE_23538
Set the value of member '_endContextDeclaration' to 'rEndContextDeclaration'.
*/
void ContextDeclaration::SetEndContextDeclaration(const CbString& rEndContextDeclaration)
{//@CODE_23538
    if (_endContextDeclaration != rEndContextDeclaration)
    {
        SaveState();
        _endContextDeclaration = rEndContextDeclaration;

        if (!rEndContextDeclaration.IsEmpty())
        {
            if (rEndContextDeclaration[rEndContextDeclaration.GetLength()-1] != '\n')
                _endContextDeclaration += NL;
        }
    }
}//@CODE_23538


/*@NOTE_23546
Set the value of member '_endContextImplementation' to 'rEndContextImplementation'.
*/
void ContextDeclaration::SetEndContextImplementation(const CbString& rEndContextImplementation)
{//@CODE_23546
    if (_endContextImplementation != rEndContextImplementation)
    {
        SaveState();
        _endContextImplementation = rEndContextImplementation;

        if (!rEndContextImplementation.IsEmpty())
        {
            if (rEndContextImplementation[rEndContextImplementation.GetLength()-1] != '\n')
                _endContextImplementation += NL;
        }
    }
}//@CODE_23546


/*@NOTE_26153
Set the value of member '_name' to 'rName'.
*/
void ContextDeclaration::SetName(const CbString& rName)
{//@CODE_26153
    if (_name != rName)
    {
        SaveState();
        _name = rName;
    }
}//@CODE_26153


/*@NOTE_27297
Set the value of member '_note' to 'rNote'.
*/
void ContextDeclaration::SetNote(const CbString& rNote)
{//@CODE_27297
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_27297


/*@NOTE_23533
Set the value of member '_endContextDeclaration' to 'rStartContextDeclaration'.
*/
void ContextDeclaration::SetStartContextDeclaration(const CbString& rStartContextDeclaration)
{//@CODE_23533
    if (_startContextDeclaration != rStartContextDeclaration)
    {
        SaveState();
        _startContextDeclaration = rStartContextDeclaration;

        if (!rStartContextDeclaration.IsEmpty())
        {
            if (rStartContextDeclaration[rStartContextDeclaration.GetLength()-1] != '\n')
                _startContextDeclaration += NL;
        }
    }
}//@CODE_23533


/*@NOTE_23542
Set the value of member '_endContextImplementation' to 'rStartContextImplementation'.
*/
void ContextDeclaration::SetStartContextImplementation(const CbString& rStartContextImplementation)
{//@CODE_23542
    if (_startContextImplementation != rStartContextImplementation)
    {
        SaveState();
        _startContextImplementation = rStartContextImplementation;

        if (!rStartContextImplementation.IsEmpty())
        {
            if (rStartContextImplementation[rStartContextImplementation.GetLength()-1] != '\n')
                _startContextImplementation += NL;
        }
    }
}//@CODE_23542


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_23522
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ContextDeclaration::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
}


/*@NOTE_23512
Method which must be called first in a constructor.
*/
void ContextDeclaration::ConstructorInclude(DataModel* pDataModel)
{
    INIT_MULTI_OWNED_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    INIT_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
}


/*@NOTE_23514
Method which must be called first in a destructor.
*/
void ContextDeclaration::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    EXIT_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
}


/*@NOTE_23523
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ContextDeclaration::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
}


/*@NOTE_23524
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ContextDeclaration::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ContextDeclaration* pContextDeclaration = (ContextDeclaration*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_23526
Save the state of the current object relations to pDataModelDocObject.
*/
void ContextDeclaration::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    ContextDeclaration* pContextDeclaration = (ContextDeclaration*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
}


/*@NOTE_23517
Serialize the members only to a CbObject object.
*/
void ContextDeclaration::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _startContextDeclaration;
        archive << _endContextDeclaration;
        archive << _startContextImplementation;
        archive << _endContextImplementation;
        archive << _name;
        archive << _defineDeclaration;
        archive << _enableDefineDeclaration;
        archive << _note;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _startContextDeclaration;
            archive >> _endContextDeclaration;
            archive >> _startContextImplementation;
            archive >> _endContextImplementation;
            archive >> _name;
            archive >> _defineDeclaration;
            archive >> _enableDefineDeclaration;
            archive >> _note;
        }
    }
}


/*@NOTE_23516
Method which must be called first in a serialize constructor.
*/
void ContextDeclaration::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    INIT_MULTI_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
}


/*@NOTE_23519
Serialize the relations to a CbObject object.
*/
void ContextDeclaration::SerializeRelations(CbArchive& archive,
                                            DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ContextDeclaration)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
METHODS_ITERATOR_MULTI_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
METHODS_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
