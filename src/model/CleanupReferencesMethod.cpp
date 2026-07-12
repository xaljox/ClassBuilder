/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          CleanupReferencesMethod.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'CleanupReferencesMethod'
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


/*@NOTE_4808
Constructor needed for serialization, not meant to use for other purposes!
*/
CleanupReferencesMethod::CleanupReferencesMethod() //@INIT_4808
    : FixedMethod()
{//@CODE_4808
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_4808


CleanupReferencesMethod::CleanupReferencesMethod(BaseClass* pBaseClass) //@INIT_4869
    : FixedMethod(pBaseClass, pBaseClass->GetDataModelDoc()->FindType("void"))
{//@CODE_4869
    ConstructorInclude();

    SetName("CleanupReferences");
    SetAccess(PUBLIC);
    SetVirtual(1);
    SetNote(
        "Pre condition: The current object isn't part of the active data structure, but" NL
        "is on either the undo or the redo stack." NL
        "" NL
        "The current object isn't needed any longer and is scheduled to be deleted, a" NL
        "direct normal delete will fail, since the current object can contain" NL
        "references to the active part of the data structure. It is the task of this" NL
        "routine to cleanup those references, so the object can be safely removed.");
}//@CODE_4869


/*@NOTE_4806
Destructor method.
*/
CleanupReferencesMethod::~CleanupReferencesMethod()
{//@CODE_4806
    DestructorInclude();

    // Put in your own code
}//@CODE_4806


void CleanupReferencesMethod::InitCode()
{//@CODE_4877
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
                    "::CleanupReferences();" NL;
            }
        }
        
        Class::ToRelationIterator toRel((Class*)GetBaseClass());
        while (++toRel)
        {
            toRel->WriteToMacro(_code, GetIndent() + "CLEANUP_");
        }
    }
}//@CODE_4877


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5780
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void CleanupReferencesMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
}


/*@NOTE_4805
Method which must be called first in a constructor.
*/
void CleanupReferencesMethod::ConstructorInclude()
{
}


/*@NOTE_4807
Method which must be called first in a destructor.
*/
void CleanupReferencesMethod::DestructorInclude()
{
}


/*@NOTE_5781
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void CleanupReferencesMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
}


/*@NOTE_5782
Bring the current object relations into the same state as pDataModelDocObject.
*/
void CleanupReferencesMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5784
Save the state of the current object relations to pDataModelDocObject.
*/
void CleanupReferencesMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_4810
Serialize the members only to a CbObject object.
*/
void CleanupReferencesMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_4809
Method which must be called first in a serialize constructor.
*/
void CleanupReferencesMethod::SerializeConstructorInclude()
{
}


/*@NOTE_4812
Serialize the relations to a CbObject object.
*/
void CleanupReferencesMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(CleanupReferencesMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
