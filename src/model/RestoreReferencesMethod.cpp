/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RestoreReferencesMethod.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RestoreReferencesMethod'
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


/*@NOTE_4834
Constructor needed for serialization, not meant to use for other purposes!
*/
RestoreReferencesMethod::RestoreReferencesMethod() //@INIT_4834
    : FixedMethod()
{//@CODE_4834
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4834


RestoreReferencesMethod::RestoreReferencesMethod(BaseClass* pBaseClass) //@INIT_4873
    : FixedMethod(pBaseClass, pBaseClass->GetDataModelDoc()->FindType("void"))
{//@CODE_4873
    ConstructorInclude();

    SetName("RestoreReferences");
    SetAccess(PUBLIC);
    SetVirtual(1);

    Class* pDocumentObject = GetDataModelDoc()->GetDataModel()->GetDocumentObject();
    if (!pDocumentObject)
    {
        pDocumentObject = (Class*)pBaseClass;
    }

    Argument* pArgument = new Argument(this, pDocumentObject);
    pArgument->SetPointer(1);
    pArgument->SetName("p" + pDocumentObject->GetName());
    SetNote("Bring the current object relations into the same state as " + pArgument->GetName() + ".");
}//@CODE_4873


/*@NOTE_4832
Destructor method.
*/
RestoreReferencesMethod::~RestoreReferencesMethod()
{//@CODE_4832
    DestructorInclude();

    // Put in your own code
}//@CODE_4832


void RestoreReferencesMethod::InitCode()
{//@CODE_4879
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code.Empty();
        DataModel* pDataModel = GetDataModelDoc()->GetDataModel();
        
        Class::ToRelationIterator toRel((Class*)GetBaseClass());
        while (++toRel)
        {
            if (toRel.IsFirst() && GetFirstArgument()->GetType() != pClass)
            {
                _code += GetIndent() + pClass->GetName() + "* p" + pClass->GetName();
                _code += " = (" + pClass->GetName() + "*)" + GetFirstArgument()->GetName() + ";" NL;
            }
            
            toRel->WriteToMacro(_code, GetIndent() + "RESTORE_");
        }
        
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            if (inherit->GetBaseClass()->IsClass())
            {
                _code += GetIndent() + inherit->GetBaseName() + 
                    "::RestoreReferences(" + GetFirstArgument()->GetName() + ");" NL;
            }
        }
    }
}//@CODE_4879


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5792
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RestoreReferencesMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
}


/*@NOTE_4831
Method which must be called first in a constructor.
*/
void RestoreReferencesMethod::ConstructorInclude()
{
}


/*@NOTE_4833
Method which must be called first in a destructor.
*/
void RestoreReferencesMethod::DestructorInclude()
{
}


/*@NOTE_5793
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RestoreReferencesMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
}


/*@NOTE_5794
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RestoreReferencesMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5796
Save the state of the current object relations to pDataModelDocObject.
*/
void RestoreReferencesMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4836
Serialize the members only to a CbObject object.
*/
void RestoreReferencesMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4835
Method which must be called first in a serialize constructor.
*/
void RestoreReferencesMethod::SerializeConstructorInclude()
{
}


/*@NOTE_4838
Serialize the relations to a CbObject object.
*/
void RestoreReferencesMethod::SerializeRelations(CbArchive& archive,
                                                 DataModelDocObject* pointerArray[])
{
    FixedMethod::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(RestoreReferencesMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
