/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          SaveReferencesMethod.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SaveReferencesMethod'
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


/*@NOTE_4847
Constructor needed for serialization, not meant to use for other purposes!
*/
SaveReferencesMethod::SaveReferencesMethod() //@INIT_4847
    : FixedMethod()
{//@CODE_4847
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4847


SaveReferencesMethod::SaveReferencesMethod(BaseClass* pBaseClass) //@INIT_4875
    : FixedMethod(pBaseClass, pBaseClass->GetDataModelDoc()->FindType("void"))
{//@CODE_4875
    ConstructorInclude();

    SetName("SaveReferences");
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
    SetNote("Save the state of the current object relations to " + pArgument->GetName() + ".");
}//@CODE_4875


/*@NOTE_4845
Destructor method.
*/
SaveReferencesMethod::~SaveReferencesMethod()
{//@CODE_4845
    DestructorInclude();

    // Put in your own code
}//@CODE_4845


void SaveReferencesMethod::InitCode()
{//@CODE_4880
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code.Empty();
        DataModel* pDataModel = GetDataModelDoc()->GetDataModel();
        
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            if (inherit->GetBaseClass()->IsClass())
            {
                _code += GetIndent() + inherit->GetBaseName() + 
                    "::SaveReferences(" + GetFirstArgument()->GetName() + ");" NL;
            }
        }
        
        Class::ToRelationIterator toRel((Class*)GetBaseClass());
        while (++toRel)
        {
            if (toRel.IsFirst() && GetFirstArgument()->GetType() != pClass)
            {
                _code += GetIndent() + pClass->GetName() + "* p" + pClass->GetName();
                _code += " = (" + pClass->GetName() + "*)" + GetFirstArgument()->GetName() + ";" NL;
            }
            
            toRel->WriteToMacro(_code, GetIndent() + "SAVE_");
        }
    }
}//@CODE_4880


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5798
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SaveReferencesMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
}


/*@NOTE_4844
Method which must be called first in a constructor.
*/
void SaveReferencesMethod::ConstructorInclude()
{
}


/*@NOTE_4846
Method which must be called first in a destructor.
*/
void SaveReferencesMethod::DestructorInclude()
{
}


/*@NOTE_5799
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SaveReferencesMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
}


/*@NOTE_5800
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SaveReferencesMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5802
Save the state of the current object relations to pDataModelDocObject.
*/
void SaveReferencesMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4849
Serialize the members only to a CbObject object.
*/
void SaveReferencesMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4848
Method which must be called first in a serialize constructor.
*/
void SaveReferencesMethod::SerializeConstructorInclude()
{
}


/*@NOTE_4851
Serialize the relations to a CbObject object.
*/
void SaveReferencesMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(SaveReferencesMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
