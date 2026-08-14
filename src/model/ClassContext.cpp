/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassContext.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassContext'
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
//@END_USER2


// Static members


/*@NOTE_26384
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassContext::ClassContext() //@INIT_26384
    : Context()
{//@CODE_26384
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_26384


/*@NOTE_26866
Constructor method.
*/
ClassContext::ClassContext(Class* pClass,
                           ContextDeclaration* pContextDeclaration) //@INIT_26866
    : Context(pContextDeclaration)
{//@CODE_26866
    ConstructorInclude(pClass);

    // Put in your own code
}//@CODE_26866


/*@NOTE_26382
Destructor method.
*/
ClassContext::~ClassContext()
{//@CODE_26382
    DestructorInclude();

    // Put in your own code
}//@CODE_26382


/*@NOTE_26890
Returns a pointer to the object to which this context applies.
*/
Gti* ClassContext::GetContextObject()
{//@CODE_26890
    return GetClass();
}//@CODE_26890


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_26391
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassContext::CleanupReferences()
{
    Context::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)
}


/*@NOTE_26381
Method which must be called first in a constructor.
*/
void ClassContext::ConstructorInclude(Class* pClass)
{
    INIT_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)
}


/*@NOTE_26383
Method which must be called first in a destructor.
*/
void ClassContext::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)
}


/*@NOTE_26392
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassContext::RemoveReferences()
{
    Context::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)
}


/*@NOTE_26393
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassContext::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassContext* pClassContext = (ClassContext*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)
    Context::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_26395
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassContext::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Context::SaveReferences(pDataModelDocObject);
    ClassContext* pClassContext = (ClassContext*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)
}


/*@NOTE_26386
Serialize the members only to a CbObject object.
*/
void ClassContext::Serialize(CbArchive& archive)
{
    Context::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_26385
Method which must be called first in a serialize constructor.
*/
void ClassContext::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Class, Class, ClassContext, ClassContext)
}


/*@NOTE_26388
Serialize the relations to a CbObject object.
*/
void ClassContext::SerializeRelations(CbArchive& archive,
                                      DataModelDocObject* pointerArray[])
{
    Context::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ClassContext)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Class, Class, ClassContext, ClassContext)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
