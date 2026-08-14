/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ExceptionSpecificationType.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ExceptionSpecificationType'
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


/*@NOTE_22468
Constructor method.
*/
ExceptionSpecificationType::ExceptionSpecificationType(ExceptionSpecification* pExceptionSpecification,
                                                       Type* pType) //@INIT_22468
    : DataModelDocObject(pExceptionSpecification->GetDataModelDoc())
    , _array(0)
    , _arraySizeStr("")
    , _const(0)
    , _constPointer(0)
    , _pointer(0)
    , _pointerPointer(0)
    , _reference(0)
    , _template(pType->GetTemplate())
{//@CODE_22468
    ConstructorInclude(pExceptionSpecification, pType);

    // Put in your own code
}//@CODE_22468


/*@NOTE_22292
Constructor needed for serialization, not meant to use for other purposes!
*/
ExceptionSpecificationType::ExceptionSpecificationType() //@INIT_22292
    : DataModelDocObject()
    , _array(0)
    , _arraySizeStr("")
    , _const(0)
    , _constPointer(0)
    , _pointer(0)
    , _pointerPointer(0)
    , _reference(0)
    , _template("")
{//@CODE_22292
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_22292


/*@NOTE_22290
Destructor method.
*/
ExceptionSpecificationType::~ExceptionSpecificationType()
{//@CODE_22290
    DestructorInclude();

    // Put in your own code
}//@CODE_22290


const CbString ExceptionSpecificationType::GetTypeName()
{//@CODE_22858
    CbString typeName;
    if (GetConst())
        typeName += "const ";

    typeName += GetType()->Type::GetName() + GetTemplate();
    if (GetPointer())
        typeName += "*";
    if (GetConstPointer())
        typeName += " const ";
    if (GetPointerPointer())
        typeName += "*";
    if (GetReference())
        typeName += "&";
    if (GetArray())
        typeName += "[" + GetArraySizeStr() + "]";

    return typeName;
}//@CODE_22858


/*@NOTE_22947
This method is a hook to update the view in case the object appears because of
an Undo/Redo. It is called after the object is added again into the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void ExceptionSpecificationType::OnUndoRedoAdded()
{//@CODE_22947
    GetExceptionSpecification()->GetMethod()->Update();
}//@CODE_22947


/*@NOTE_22938
This method is a hook to update the view in case the object disappears because of
an Undo/Redo. It is called after the object is removed from the data structure
This method is empty, so overwrite this virtual method at derived classes if needed.

*/
void ExceptionSpecificationType::OnUndoRedoRemoved()
{//@CODE_22938
    GetExceptionSpecification()->GetMethod()->Update();
}//@CODE_22938


void ExceptionSpecificationType::ReplaceInX(const CbString& oldString,
                                            const CbString& newString)
{//@CODE_23084
    if (ReplaceInStr(_template, oldString, newString))
    {
        GetExceptionSpecification()->GetMethod()->Update();
    }
}//@CODE_23084


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_22299
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ExceptionSpecificationType::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    CLEANUP_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
}


/*@NOTE_22289
Method which must be called first in a constructor.
*/
void ExceptionSpecificationType::ConstructorInclude(ExceptionSpecification* pExceptionSpecification,
                                                    Type* pType)
{
    INIT_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    INIT_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
}


/*@NOTE_22291
Method which must be called first in a destructor.
*/
void ExceptionSpecificationType::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    EXIT_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
}


/*@NOTE_22300
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ExceptionSpecificationType::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    REMOVE_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
}


/*@NOTE_22301
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ExceptionSpecificationType::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ExceptionSpecificationType* pExceptionSpecificationType = (ExceptionSpecificationType*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    RESTORE_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_22303
Save the state of the current object relations to pDataModelDocObject.
*/
void ExceptionSpecificationType::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    ExceptionSpecificationType* pExceptionSpecificationType = (ExceptionSpecificationType*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    SAVE_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
}


/*@NOTE_22294
Serialize the members only to a CbObject object.
*/
void ExceptionSpecificationType::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _array;
        archive << _arraySizeStr;
        archive << _const;
        archive << _constPointer;
        archive << _pointer;
        archive << _pointerPointer;
        archive << _reference;
        archive << _template;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _array;
            archive >> _arraySizeStr;
            archive >> _const;
            archive >> _constPointer;
            archive >> _pointer;
            archive >> _pointerPointer;
            archive >> _reference;
            archive >> _template;
        }
    }
}


/*@NOTE_22293
Method which must be called first in a serialize constructor.
*/
void ExceptionSpecificationType::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    INIT_MULTI_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
}


/*@NOTE_22296
Serialize the relations to a CbObject object.
*/
void ExceptionSpecificationType::SerializeRelations(CbArchive& archive,
                                                    DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ExceptionSpecificationType)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
METHODS_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
