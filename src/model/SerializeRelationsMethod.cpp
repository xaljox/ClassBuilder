/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SerializeRelationsMethod.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SerializeRelationsMethod'
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


/*@NOTE_490
Constructor needed for serialization, not meant to use for other purposes!
*/
SerializeRelationsMethod::SerializeRelationsMethod() //@INIT_490
    : FixedMethod()
{//@CODE_490
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_490


SerializeRelationsMethod::SerializeRelationsMethod(BaseClass* pBaseClass) //@INIT_1009
    : FixedMethod(pBaseClass, pBaseClass->GetDataModelDoc()->FindType("void"))
{//@CODE_1009
    ConstructorInclude();

    // Put in your own code
    SetName("SerializeRelations");
    SetNote("Serialize the relations to a CbObject object.");
    SetAccess(PROTECTED);
    SetVirtual(1);

    Argument* pArgument = new Argument(this, GetDataModelDoc()->FindType("CbObject"));
    pArgument->SetReference(1);
    pArgument->SetName("archive");

    Class* pDocumentObject = GetDataModelDoc()->GetDataModel()->GetDocumentObject();
    if (!pDocumentObject)
        pDocumentObject = (Class*)pBaseClass;

    pArgument = new Argument(this, pDocumentObject);
    pArgument->SetPointer(1);
    pArgument->SetArray(1);
    pArgument->SetName("pointerArray");
}//@CODE_1009


/*@NOTE_488
Destructor method
*/
SerializeRelationsMethod::~SerializeRelationsMethod()
{//@CODE_488
    DestructorInclude();

    // Put in your own code
}//@CODE_488


void SerializeRelationsMethod::InitCode()
{//@CODE_1011
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code.Empty();
        DataModel* pDataModel = GetDataModelDoc()->GetDataModel();

		Argument* pArgument = GetFirstArgument();
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            if (inherit->GetBaseClass()->IsClass())
            {
                _code += GetIndent() + inherit->GetBaseName() + 
                         "::SerializeRelations(" + pArgument->GetName() + ", pointerArray);" NL;
            }
        }

        _code += GetIndent() + "if (" + pArgument->GetName() + ".IsStoring())" NL;
        _code += GetIndent() + "{" NL;
        Class::FromRelationIterator relation(pClass);
        while (++relation)
        {
            if (relation->GetToClass()->GetSerialize())
                relation->WriteFromMacro(_code, GetIndent(2) + "WRITE_", 0);
        }
        _code += GetIndent() + "}" NL;
        _code += GetIndent() + "else" NL;
        _code += GetIndent() + "{" NL;

        // Relations, version-grouped. i==0 reads ungated; i>=1 reads gated.
        // Document uses its own _version, non-Document classes use the shared
        // static _objectVersion (already set by Document's Serialize earlier).
        CbString gateVar = (pClass == pDataModel->GetDocument())
            ? "_version" : "_objectVersion";
        for (int i = 0; i <= GetDataModelDoc()->GetVersion(); i++)
        {
            bool first = true;
            relation.Reset();
            while (++relation)
            {
                if (relation->GetToClass()->GetSerialize())
                {
                    if (relation->GetFromRelation()->GetInitialVersion() == i)
                    {
                        if (first)
                        {
                            first = false;
                            if (i > 0)
                            {
                                CbString str;
                                str.Format("if (%d <= %s)", i, (LPCTSTR)gateVar);
                                _code += GetIndent(2) + str + NL;
                                _code += GetIndent(2) + "{" NL;
                            }
                        }
                        int macroIndent = (i > 0) ? 3 : 2;
                        // Always emit READ_* (static_cast). CbArchive's
                        // CbClassRegistration creates the right concrete type
                        // on read, so a static_cast back to ClassTo is safe
                        // and side-steps the C++ dynamic_cast path that was
                        // returning NULL for some inheritance shapes after
                        // the CObject->CbObject base flip — which corrupted
                        // the linked-list reconstitution and showed up as
                        // wrong tree order / drag-drop appending to end.
                        relation->WriteFromMacro(_code, GetIndent(macroIndent) + "READ_", 0);
                    }
                }
            }
            if (!first && i > 0)
                _code += GetIndent(2) + "}" NL;
        }
        _code += GetIndent() + "}" NL;
    }
}//@CODE_1011


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5546
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SerializeRelationsMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
}


/*@NOTE_487
Method which must be called first in a constructor
*/
void SerializeRelationsMethod::ConstructorInclude()
{
}


/*@NOTE_489
Method which must be called first in a destructor
*/
void SerializeRelationsMethod::DestructorInclude()
{
}


/*@NOTE_5547
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SerializeRelationsMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
}


/*@NOTE_5548
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SerializeRelationsMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5550
Save the state of the current object relations to pDataModelDocObject.
*/
void SerializeRelationsMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_492
Serialize the members only to a CbObject object
*/
void SerializeRelationsMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_491
Method which must be called first in a serialize constructor
*/
void SerializeRelationsMethod::SerializeConstructorInclude()
{
}


/*@NOTE_494
Serialize the relations to a CbObject object
*/
void SerializeRelationsMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(SerializeRelationsMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
