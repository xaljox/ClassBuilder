/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ExceptionSpecification.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ExceptionSpecification'
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


/*@NOTE_22286
Constructor method.
*/
ExceptionSpecification::ExceptionSpecification(Method* pMethod) //@INIT_22286
    : DataModelDocObject(pMethod->GetDataModelDoc())
{//@CODE_22286
    ConstructorInclude(pMethod);

    // Put in your own code
}//@CODE_22286


/*@NOTE_22216
Constructor needed for serialization, not meant to use for other purposes!
*/
ExceptionSpecification::ExceptionSpecification() //@INIT_22216
    : DataModelDocObject()
{//@CODE_22216
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_22216


/*@NOTE_22214
Destructor method.
*/
ExceptionSpecification::~ExceptionSpecification()
{//@CODE_22214
    DestructorInclude();

    // Put in your own code
}//@CODE_22214


CbString ExceptionSpecification::GetThrowString() const
{//@CODE_22479
    CbString str = " throw(";

    ExceptionSpecificationTypeIterator iExceptionSpecificationType(this);
    while (++iExceptionSpecificationType)
    {
        str += iExceptionSpecificationType->GetTypeName();
        
        if (!iExceptionSpecificationType.IsLast())
        {
            str += ", ";
        }
    }
    str += ")";
        
    return str;
}//@CODE_22479


/*@NOTE_23507
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void ExceptionSpecification::OnUndoRedoAdded()
{//@CODE_23507
    GetMethod()->Update();
}//@CODE_23507


/*@NOTE_23508
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called after the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void ExceptionSpecification::OnUndoRedoRemoved()
{//@CODE_23508
    GetMethod()->Update();
}//@CODE_23508


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_22223
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ExceptionSpecification::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
}


/*@NOTE_22213
Method which must be called first in a constructor.
*/
void ExceptionSpecification::ConstructorInclude(Method* pMethod)
{
    INIT_MULTI_OWNED_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    INIT_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
}


/*@NOTE_22215
Method which must be called first in a destructor.
*/
void ExceptionSpecification::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    EXIT_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
}


/*@NOTE_22224
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ExceptionSpecification::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    DataModelDocObject::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
}


/*@NOTE_22225
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ExceptionSpecification::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ExceptionSpecification* pExceptionSpecification = (ExceptionSpecification*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_22227
Save the state of the current object relations to pDataModelDocObject.
*/
void ExceptionSpecification::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    ExceptionSpecification* pExceptionSpecification = (ExceptionSpecification*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
}


/*@NOTE_22218
Serialize the members only to a CbObject object.
*/
void ExceptionSpecification::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_22217
Method which must be called first in a serialize constructor.
*/
void ExceptionSpecification::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    INIT_SINGLE_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
}


/*@NOTE_22220
Serialize the relations to a CbObject object.
*/
void ExceptionSpecification::SerializeRelations(CbArchive& archive,
                                                DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ExceptionSpecification)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
METHODS_ITERATOR_MULTI_ACTIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
METHODS_SINGLE_OWNED_PASSIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
