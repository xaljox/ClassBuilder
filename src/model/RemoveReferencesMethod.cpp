/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RemoveReferencesMethod.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RemoveReferencesMethod'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
//@END_USER2


// Static members


/*@NOTE_4821
Constructor needed for serialization, not meant to use for other purposes!
*/
RemoveReferencesMethod::RemoveReferencesMethod() //@INIT_4821
    : FixedMethod()
{//@CODE_4821
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4821


RemoveReferencesMethod::RemoveReferencesMethod(BaseClass* pBaseClass) //@INIT_4871
    : FixedMethod(pBaseClass, pBaseClass->GetDataModelDoc()->FindType("void"))
{//@CODE_4871
    ConstructorInclude();

    SetName("RemoveReferences");
    SetAccess(PUBLIC);
    SetVirtual(1);
    SetNote(
        "Remove all references to the current object, but keep the references from this" NL
        "object, so the state can be restored.");
}//@CODE_4871


/*@NOTE_4819
Destructor method.
*/
RemoveReferencesMethod::~RemoveReferencesMethod()
{//@CODE_4819
    DestructorInclude();

    // Put in your own code
}//@CODE_4819


void RemoveReferencesMethod::InitCode()
{//@CODE_4878
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code.Empty();
        DataModel* pDataModel = GetDataModelDoc()->GetDataModel();
        
        Class::FromRelationIterator fromRel((Class*)GetBaseClass());
        while (--fromRel)
        {
            if (fromRel->GetToClass()->GetSerialize())
            {
                fromRel->WriteFromMacro(_code, GetIndent() + "REMOVE_");
            }
            else
            {
                fromRel->WriteFromMacro(_code, GetIndent() + "EXIT_");
            }
        }
        
        Class::InheritIterator inherit(pClass);
        while (--inherit)
        {
            if (inherit->GetBaseClass()->IsClass())
            {
                _code += GetIndent() + inherit->GetBaseName() + 
                    "::RemoveReferences();" NL;
            }
        }
        
        Class::ToRelationIterator toRel((Class*)GetBaseClass());
        while (--toRel)
        {
            toRel->WriteToMacro(_code, GetIndent() + "REMOVE_");
        }
    }
}//@CODE_4878


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5786
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RemoveReferencesMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
}


/*@NOTE_4818
Method which must be called first in a constructor.
*/
void RemoveReferencesMethod::ConstructorInclude()
{
}


/*@NOTE_4820
Method which must be called first in a destructor.
*/
void RemoveReferencesMethod::DestructorInclude()
{
}


/*@NOTE_5787
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RemoveReferencesMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
}


/*@NOTE_5788
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RemoveReferencesMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5790
Save the state of the current object relations to pDataModelDocObject.
*/
void RemoveReferencesMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4823
Serialize the members only to a CbObject object.
*/
void RemoveReferencesMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4822
Method which must be called first in a serialize constructor.
*/
void RemoveReferencesMethod::SerializeConstructorInclude()
{
}


/*@NOTE_4825
Serialize the relations to a CbObject object.
*/
void RemoveReferencesMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(RemoveReferencesMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
