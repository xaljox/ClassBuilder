/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassGroupContext.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassGroupContext'
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


/*@NOTE_26540
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassGroupContext::ClassGroupContext() //@INIT_26540
    : Context()
{//@CODE_26540
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_26540


/*@NOTE_26876
Constructor method.
*/
ClassGroupContext::ClassGroupContext(ClassGroup* pClassGroup,
                                     ContextDeclaration* pContextDeclaration) //@INIT_26876
    : Context(pContextDeclaration)
{//@CODE_26876
    ConstructorInclude(pClassGroup);

    // Put in your own code
}//@CODE_26876


/*@NOTE_26538
Destructor method.
*/
ClassGroupContext::~ClassGroupContext()
{//@CODE_26538
    DestructorInclude();

    // Put in your own code
}//@CODE_26538


/*@NOTE_27062
Returns a pointer to the object to which this context applies.
*/
Gti* ClassGroupContext::GetContextObject()
{//@CODE_27062
    return GetClassGroup();
}//@CODE_27062


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_26547
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassGroupContext::CleanupReferences()
{
    Context::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
}


/*@NOTE_26537
Method which must be called first in a constructor.
*/
void ClassGroupContext::ConstructorInclude(ClassGroup* pClassGroup)
{
    INIT_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
}


/*@NOTE_26539
Method which must be called first in a destructor.
*/
void ClassGroupContext::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
}


/*@NOTE_26548
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassGroupContext::RemoveReferences()
{
    Context::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
}


/*@NOTE_26549
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassGroupContext::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassGroupContext* pClassGroupContext = (ClassGroupContext*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
    Context::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_26551
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassGroupContext::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Context::SaveReferences(pDataModelDocObject);
    ClassGroupContext* pClassGroupContext = (ClassGroupContext*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
}


/*@NOTE_26542
Serialize the members only to a CbObject object.
*/
void ClassGroupContext::Serialize(CbArchive& archive)
{
    Context::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_26541
Method which must be called first in a serialize constructor.
*/
void ClassGroupContext::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)
}


/*@NOTE_26544
Serialize the relations to a CbObject object.
*/
void ClassGroupContext::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(ClassGroupContext)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(ClassGroup, ClassGroup, ClassGroupContext, ClassGroupContext)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
