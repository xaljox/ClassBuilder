/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SerializeConstructor.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SerializeConstructor'
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


/*@NOTE_373
Constructor needed for serialization, not meant to use for other purposes!
*/
SerializeConstructor::SerializeConstructor() //@INIT_373
    : Constructor()
{//@CODE_373
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_373


SerializeConstructor::SerializeConstructor(BaseClass* pBaseClass) //@INIT_970
    : Constructor(pBaseClass)
{//@CODE_970
    ConstructorInclude();

    // Put in your own code
    // ClassCount == 2 detects the document-class setup path. Background:
    // DataModel::InitSerialize creates DocumentObject first, then Document.
    // Each `new Class(this)` auto-adds the class to the model (via
    // INIT_MULTI_OWNED_PASSIVE in Class::ConstructorInclude) and, if the
    // model has Serialize enabled, the Class constructor immediately
    // creates a SerializeConstructor (Class.cpp ~105). So:
    //   - DocumentObject's SerializeConstructor sees ClassCount == 1
    //     (only DocumentObject just registered).
    //   - Document's SerializeConstructor sees ClassCount == 2
    //     (DocumentObject + Document registered).
    // Hence ClassCount == 2 here means "this SerializeConstructor belongs
    // to the document class". We can't use pBaseClass == GetDocument()
    // instead because AddDocument() runs AFTER `new Class()` returns.
    // Order-sensitive: relies on DocumentObject being created before
    // Document in InitSerialize.
    if (GetDataModelDoc()->GetDataModel()->GetClassCount() == 2)
    {
        // we are at the document class, we do things here slightly different
        SetAccess(PUBLIC);
        SetNote("Constructor needed for serialization, can also be used for default construction.");

        (void)new SerializeConstructorIncludeMethod(this);
        (void)new SerializeMethod(pBaseClass);
        (void)new SerializeRelationsMethod(pBaseClass);
    }
    else
    {
        SetAccess(PROTECTED); // PUBLIC if Document class
        SetNote("Constructor needed for serialization, not meant to use for other purposes!");

        (void)new SerializeConstructorIncludeMethod(this);
        (void)new SerializeMethod(pBaseClass);
        (void)new SerializeRelationsMethod(pBaseClass);
        if (GetDataModelDoc()->GetDataModel()->GetUndoRedo())
        {
            (void)new CleanupReferencesMethod(pBaseClass);
            (void)new RemoveReferencesMethod(pBaseClass);
            (void)new RestoreReferencesMethod(pBaseClass);
            (void)new SaveReferencesMethod(pBaseClass);
        }
    }
}//@CODE_970


/*@NOTE_371
Destructor method
*/
SerializeConstructor::~SerializeConstructor()
{//@CODE_371
    DestructorInclude();

    // Put in your own code
}//@CODE_371


void SerializeConstructor::InitCode()
{//@CODE_973
    _code.Empty();
    _code += GetIndent() + "SerializeConstructorInclude();" NL NL;
    _code += GetIndent() + "// Put in your own code" NL;
}//@CODE_973


void SerializeConstructor::InitInit()
{//@CODE_972
    _init.Empty();

    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        bool first = true;
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            if (first) 
            { 
                first = false; 
                _init += GetIndent() + ": "; 
            } 
            else  
                _init += GetIndent() + ", ";

            _init += inherit->GetBaseName() + "()" NL;
        }

        Class::MemberIterator member(pClass);
        while (++member)
        {
            if (!member->GetInitialization().IsEmpty() && 
                !member->GetStatic() && !member->GetArray() &&
                (!member->GetSerialize() || pClass->GetDocument()))
            {
                _init += member->GetStartContextImplementation();
                if (first) 
                { 
                    first = false; 
                    _init += GetIndent() + ": "; 
                } 
                else  
                    _init += GetIndent() + ", ";

                _init += member->GetPrefixedName() + "(";
                _init += member->GetInitialization() + ")" NL;
                _init += member->GetEndContextImplementation();
            }
        }
    }
}//@CODE_972


int SerializeConstructor::OnAddArgument(bool checkOnly)
{//@CODE_975
    return 0;
}//@CODE_975


int SerializeConstructor::OnDelete(bool checkOnly)
{//@CODE_974
    return 0;
}//@CODE_974


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5528
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SerializeConstructor::CleanupReferences()
{
    Constructor::CleanupReferences();
}


/*@NOTE_370
Method which must be called first in a constructor
*/
void SerializeConstructor::ConstructorInclude()
{
    INIT_SINGLE_OWNED_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_372
Method which must be called first in a destructor
*/
void SerializeConstructor::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_5529
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SerializeConstructor::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
    Constructor::RemoveReferences();
}


/*@NOTE_5530
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SerializeConstructor::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Constructor::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5532
Save the state of the current object relations to pDataModelDocObject.
*/
void SerializeConstructor::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Constructor::SaveReferences(pDataModelDocObject);
}


/*@NOTE_375
Serialize the members only to a CbObject object
*/
void SerializeConstructor::Serialize(CbArchive& archive)
{
    Constructor::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_374
Method which must be called first in a serialize constructor
*/
void SerializeConstructor::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
}


/*@NOTE_377
Serialize the relations to a CbObject object
*/
void SerializeConstructor::SerializeRelations(CbArchive& archive,
                                              DataModelDocObject* pointerArray[])
{
    Constructor::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(SerializeConstructor)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(SerializeConstructor, SerializeConstructor, SerializeConstructorIncludeMethod, SerializeConstructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
