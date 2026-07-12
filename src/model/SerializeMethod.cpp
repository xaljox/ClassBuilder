/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SerializeMethod.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SerializeMethod'
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


SerializeMethod::SerializeMethod(BaseClass* pBaseClass) //@INIT_1006
    : FixedMethod(pBaseClass, pBaseClass->GetDataModelDoc()->FindType("void"))
{//@CODE_1006
    ConstructorInclude();

    // Put in your own code
    SetName("Serialize");
    SetNote("Serialize the members only to a CbObject object.");
    if (GetDataModelDoc()->GetDataModel()->GetUndoRedo())
    {
        SetAccess(PUBLIC);
    }
    else
    {
        SetAccess(PROTECTED);
    }
    SetVirtual(1);

    Argument* pArgument = new Argument(this, GetDataModelDoc()->FindType("CbObject"));
    pArgument->SetReference(1);
    pArgument->SetName("archive");
}//@CODE_1006


/*@NOTE_477
Constructor needed for serialization, not meant to use for other purposes!
*/
SerializeMethod::SerializeMethod() //@INIT_477
    : FixedMethod()
{//@CODE_477
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_477


/*@NOTE_475
Destructor method
*/
SerializeMethod::~SerializeMethod()
{//@CODE_475
    DestructorInclude();

    // Put in your own code
}//@CODE_475


void SerializeMethod::InitCode()
{//@CODE_1008
    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code.Empty();
        DataModel* pDataModel = GetDataModelDoc()->GetDataModel();

		Argument* pArgument = GetFirstArgument();
        Class::InheritIterator inherit(pClass);
        while (++inherit)
        {
            if (inherit->GetBaseName() != "CbObject")
                _code += GetIndent() + inherit->GetBaseName() + "::Serialize(" + pArgument->GetName() + ");" NL;
        }

		
        _code += GetIndent() + "if (" + pArgument->GetName() + ".IsStoring())" NL;
        _code += GetIndent() + "{" NL;

        // Non-static members, flat (store side has no per-version gates).
        Class::MemberIterator member(pClass, &Member::GetSerialize);
        while (++member)
        {
            if (!member->GetStatic())
            {
                OtherType* pOtherType = dynamic_cast<OtherType*>(member->GetType());
                if (pOtherType && pOtherType->GetSerializeMap())
                {
                    if (pOtherType->GetSerializeMap() == 1)
                    {
                        _code += member->GetStartContextImplementation();
                        _code += GetIndent(2) + pArgument->GetName() + " << int(" + member->GetPrefixedName() + ");" NL;
                        _code += member->GetEndContextImplementation();
                    }
                }
                else
                {
                    _code += member->GetStartContextImplementation();
                    _code += GetIndent(2) + pArgument->GetName() + " << " + member->GetPrefixedName() + ";" NL;
                    _code += member->GetEndContextImplementation();
                }
            }
        }
        _code += GetIndent() + "}" NL;
        _code += GetIndent() + "else" NL;
        _code += GetIndent() + "{" NL;

        // Document: _version must be read unconditionally before any gate uses
        // it. _version's own InitialVersion in the model isn't always 0, so we
        // can't rely on the i==0 ungated group to cover it. Emit it explicitly,
        // propagate to the shared static, and skip it during the iteration.
        if (pClass == pDataModel->GetDocument())
        {
            _code += GetIndent(2) + pArgument->GetName() + " >> _version;" NL;
            _code += GetIndent(2) + pDataModel->GetDocumentObject()->GetName() +
                     "::_objectVersion = _version;" NL;
        }

        // Non-static members, version-grouped. i==0 reads ungated.
        // i>=1 reads are gated: Document uses its own _version, non-Document
        // classes use the shared static _objectVersion (set by Document load).
        CbString gateVar = (pClass == pDataModel->GetDocument())
            ? "_version" : "_objectVersion";
        for (int i = 0; i <= GetDataModelDoc()->GetVersion(); i++)
        {
            bool first = true;
            member.Reset();
            while (++member)
            {
                if (!member->GetStatic() && member->GetInitialVersion() == i)
                {
                    // Skip _version for Document — already read above.
                    if (pClass == pDataModel->GetDocument() &&
                        member->GetPrefixedName() == "_version")
                        continue;

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
                    int memberIndent = (i > 0) ? 3 : 2;
                    OtherType* pOtherType = dynamic_cast<OtherType*>(member->GetType());
                    if (pOtherType && pOtherType->GetSerializeMap())
                    {
                        if (pOtherType->GetSerializeMap() == 1)
                        {
                            _code += member->GetStartContextImplementation();
                            _code += GetIndent(memberIndent) + "int " + member->GetName() + "Tmp;" NL;
                            _code += GetIndent(memberIndent) + pArgument->GetName() + " >> " + member->GetName() + "Tmp;" NL;
                            _code += GetIndent(memberIndent) + member->GetPrefixedName() + " = (" +
                                     member->GetType()->GetName() + ")" + member->GetName() + "Tmp;" NL;
                            _code += member->GetEndContextImplementation();
                        }
                    }
                    else
                    {
                        _code += member->GetStartContextImplementation();
                        _code += GetIndent(memberIndent) + pArgument->GetName() + " >> " + member->GetPrefixedName() + ";" NL;
                        _code += member->GetEndContextImplementation();
                    }
                }
            }
            if (!first && i > 0)
                _code += GetIndent(2) + "}" NL;
        }
        _code += GetIndent() + "}" NL;

        if (pClass == pDataModel->GetDocument())
        {
            Relation* pRelation = pDataModel->GetDocument()->GetFirstFromRelation();

            CbString str = "SERIALIZE_ALL_OBJECTS(";
            str += "CbObject, ";
            str += pRelation->GetFromClassName() + ", ";
            str += pRelation->GetFromName() + ", ";
            str += pRelation->GetToClassName() + ", ";
            str += pRelation->GetToName() + ");";

            // Skip SERIALIZE_ALL_OBJECTS when SerializeMembersOnly is in flight,
            // i.e. snapshotting only the doc's own members for UndoChangeDoc /
            // RedoChangeDoc memory-stream round trips.
            _code += GetIndent() + "if (!_membersOnly)" NL;
            _code += GetIndent() + "{" NL;
            _code += GetIndent(2) + str + NL;
            _code += GetIndent() + "}" NL;
        }
    }
}//@CODE_1008


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5540
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SerializeMethod::CleanupReferences()
{
    FixedMethod::CleanupReferences();
}


/*@NOTE_474
Method which must be called first in a constructor
*/
void SerializeMethod::ConstructorInclude()
{
}


/*@NOTE_476
Method which must be called first in a destructor
*/
void SerializeMethod::DestructorInclude()
{
}


/*@NOTE_5541
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SerializeMethod::RemoveReferences()
{
    FixedMethod::RemoveReferences();
}


/*@NOTE_5542
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SerializeMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5544
Save the state of the current object relations to pDataModelDocObject.
*/
void SerializeMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    FixedMethod::SaveReferences(pDataModelDocObject);
}


/*@NOTE_479
Serialize the members only to a CbObject object
*/
void SerializeMethod::Serialize(CbArchive& archive)
{
    FixedMethod::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_478
Method which must be called first in a serialize constructor
*/
void SerializeMethod::SerializeConstructorInclude()
{
}


/*@NOTE_481
Serialize the relations to a CbObject object
*/
void SerializeMethod::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(SerializeMethod)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
