/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Relation.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Relation'
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


Relation::Relation(Class* pFromClass, Class* pToClass, const CbString& fromName,
                   const CbString& toName, int staticX, int multi, int single,
                   int owned, int critical) //@INIT_856
    : DataModelDocObject(pFromClass->GetDataModelDoc())
    , _fromName(fromName)
    , _note("")
    , _toName(toName)
    , _critical(critical)
    , _multi(multi)
    , _owned(owned)
    , _single(single)
    , _static(staticX)
    , _filter(multi)
{//@CODE_856
    ConstructorInclude(pFromClass, pToClass);

    // Put in your own code
    (void)new FromRelation(this);
    (void)new ToRelation(this);

    // Create new macro methods.
    (void)new ToRelationMacroMethods(GetToRelation());
    (void)new FromRelationMacroMethods(GetFromRelation());
    
    ClassDiagram::AddRelation(this);
}//@CODE_856


Relation::Relation(Class* pFromClass, Class* pToClass, int staticX, int multi,
                   int single, int owned, int critical) //@INIT_4891
    : DataModelDocObject(pFromClass->GetDataModelDoc())
    , _fromName(pFromClass->GetName())
    , _note("")
    , _toName(pToClass->GetName())
    , _critical(critical)
    , _multi(multi)
    , _owned(owned)
    , _single(single)
    , _static(staticX)
    , _filter(multi)
{//@CODE_4891
    ConstructorInclude(pFromClass, pToClass);

    // Put in your own code
    (void)new FromRelation(this);
    (void)new ToRelation(this);
    
    // Create new macro methods.
    (void)new ToRelationMacroMethods(GetToRelation());
    (void)new FromRelationMacroMethods(GetFromRelation());
    
    ClassDiagram::AddRelation(this);
}//@CODE_4891


/*@NOTE_204
Constructor needed for serialization, not meant to use for other purposes!
*/
Relation::Relation() //@INIT_204
    : DataModelDocObject()
    , _filter(1)
{//@CODE_204
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_204


/*@NOTE_202
Destructor method
*/
Relation::~Relation()
{//@CODE_202
    DestructorInclude();

    // Put in your own code
}//@CODE_202


void Relation::Add()
{//@CODE_876
    GetFromRelation()->Add();
    GetToRelation()->Add();
}//@CODE_876


/*@NOTE_5823
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void Relation::Delete()
{//@CODE_5823
    // Remember some things,before the relations are destroyed
    int version = GetDataModelDoc()->GetVersion();
    Class* pToClass = GetToClass();
    if (GetFromClass()->GetVersion() <= version || 
        GetToClass()->GetVersion() <= version)
    {
        GetFromClass()->SetVersion(version + 1);
        GetToClass()->SetVersion(version + 1);

        CbString str;
        str.Format("@Deleted relation '%s'", GetNotation().c_str());
    
        GetFromClass()->AddModified(str);
        if (GetFromClass() != GetToClass())
            GetToClass()->AddModified(str);
    }

    DataModelDocObject::Delete();
    
    if (pToClass->GetConstructorIncludeMethod() && GetOwned() && !GetStatic())
        pToClass->GetConstructorIncludeMethod()->UpdateArguments();
}//@CODE_5823


RelationShape* Relation::FindRelationShape(ClassDiagram* pClassDiagram)
{//@CODE_4117
    RelationShapeIterator iFromRelationShape(this);
    while (++iFromRelationShape)
    {
        if (pClassDiagram == iFromRelationShape->GetClassDiagram())
        {
            return iFromRelationShape;
        }
    }

    return 0;
}//@CODE_4117


CbString Relation::GetFromClassName()
{//@CODE_866
    return GetFromClass()->GetName();
}//@CODE_866


int Relation::GetImplementation()
{//@CODE_1720
    int value = 0;
    
    if (GetRelationMember())
    {
        value = GetRelationMember()->GetImplementation();
    }

    return value;
}//@CODE_1720


/*@NOTE_3192
Build up string to have a notation of this relation.
*/
CbString Relation::GetNotation()
{//@CODE_3192
    CbString value;

    value += GetFromName();
    if (GetFromClassName() != GetFromName())
        value += "(" + GetFromClassName() + ")";

    CbString c = "-";
    if (GetStatic())
        c = "=";
    if (GetOwned())
        value += " <>" + c + c + ">";
    else
        value += " " + c + c + c + ">";
    if (GetMulti())
        value += "> ";
    else
        value += ' ';

    value += GetToName();
    if (GetToClassName() != GetToName())
        value += "(" + GetToClassName() + ")";
            
    return value;
}//@CODE_3192


CbString Relation::GetToClassName()
{//@CODE_867
    return GetToClass()->GetName();
}//@CODE_867


void Relation::Update()
{//@CODE_877
    GetFromRelation()->Update();
    GetToRelation()->Update();
}//@CODE_877


void Relation::WriteFromMacro(CbString& macro, const CbString start,
                              int enableOwned)
{//@CODE_868
    CbString str;
    
    int IsTemplate = 0;
    if (!GetFromClass()->GetTemplate().IsEmpty() || !GetToClass()->GetTemplate().IsEmpty())
        IsTemplate = 1;

    str += start;
    if (str == (GetIndent() + "RELATION_"))
    {
        if (IsTemplate)
            str += "TEMPLATE_";
        if (!GetFilter() && GetMulti())
            str += "NOFILTER_";
    }
    if (GetCritical() && 
        start != "METHODS_ITERATOR_" && 
        start != "METHODS_ITERATOR_NOFILTER_")
    {
        str += "CRITICAL_";
    }
    if (GetStatic())
        str += "STATIC_";
    
    if (GetRelationMember() && 
        start != "METHODS_ITERATOR_" && 
        start != "METHODS_ITERATOR_NOFILTER_")
    {
        if (GetRelationMember()->IsUniqueValueTree())
        {
            str += "UNIQUEVALUETREE_";
        }
        else if (GetRelationMember()->IsValueTree())
        {
            str += "VALUETREE_";
        }
        else if (GetRelationMember()->IsAvlTree())
        {
            str += "AVLTREE_";
        }
        else
        {
            str += "ERROR_"; // future proof?
        }
    }
    else if (GetMulti())
    {
        str += "MULTI_";
    }

    if (GetSingle())
        str += "SINGLE_";
    if (enableOwned && GetOwned())
        str += "OWNED_";
    str += "ACTIVE(";
    if (GetRelationMember())
    {
        if (start == "METHODS_" || (IsTemplate && start == (GetIndent() + "RELATION_")))
        {
            if (GetRelationMember()->IsUniqueValueTree() || 
                GetRelationMember()->IsValueTree() || 
                GetRelationMember()->IsAvlTree())
            {
                GetMemberMethod* pGetMemberMethod = 
                    GetRelationMember()->GetMember()->GetGetMemberMethod();
                if (pGetMemberMethod)
                {
                    str += pGetMemberMethod->GetName() + "(), ";
                }
                else
                {
                    str += GetRelationMember()->GetMember()->GetPrefixedName() + ", ";
                }
            }
        }
    }
    str += GetFromClass()->Type::GetName() + GetFromClass()->GetTemplateDefine() + ", " + GetFromName() + ", ";
    str += GetToClass()->Type::GetName() + GetToClass()->GetTemplateDefine() + ", " + GetToName() + ")";

    macro += str + NL;
}//@CODE_868


void Relation::WriteToMacro(CbString& macro, const CbString start,
                            int enableOwned)
{//@CODE_872
    CbString str;
    
    str += start;
    if (GetCritical())
        str += "CRITICAL_";
    if (GetStatic())
        str += "STATIC_";
    if (GetRelationMember())
    {
        if (GetRelationMember()->IsUniqueValueTree())
        {
            str += "UNIQUEVALUETREE_";
        }
        else if (GetRelationMember()->IsValueTree())
        {
            str += "VALUETREE_";
        }
        else if (GetRelationMember()->IsAvlTree())
        {
            str += "AVLTREE_";
        }
        else
        {
            str += "ERROR_"; // future proof?
        }
    }
    else if (GetMulti())
    {
        str += "MULTI_";
    }
    if (GetSingle())
        str += "SINGLE_";
    if (enableOwned && GetOwned())
        str += "OWNED_";
    str += "PASSIVE(";
    str += GetFromClass()->Type::GetName() + GetFromClass()->GetTemplateDefine() + ", " + GetFromName() + ", ";
    str += GetToClass()->Type::GetName() + GetToClass()->GetTemplateDefine() + ", " + GetToName() + ")";

    macro += str + NL;
}//@CODE_872


bool Relation::GetCritical() const
{//@CODE_1244
    return _critical;
}//@CODE_1244


void Relation::SetCritical(bool critical)
{//@CODE_1245
    _critical = critical;
}//@CODE_1245


bool Relation::GetFilter() const
{//@CODE_1361
    return _filter;
}//@CODE_1361


void Relation::SetFilter(bool filter)
{//@CODE_1362
    _filter = filter;
}//@CODE_1362


const CbString& Relation::GetFromName()
{//@CODE_1235
    return _fromName;
}//@CODE_1235


void Relation::SetFromName(const CbString& rFromName)
{//@CODE_1236
    if (_fromName != rFromName)
    {
        if (!_fromName.IsEmpty())
        {
            if (GetToRelation()->GetToRelationMacroMethods())
            {
                ToRelationMacroMethods::MacroMethodIterator 
                    iMacroMethod(GetToRelation()->GetToRelationMacroMethods());
                while (++iMacroMethod)
                {
                    iMacroMethod->ChangeName(_fromName, rFromName);
                }
            }
        }

        _fromName = rFromName;
    }
}//@CODE_1236


bool Relation::GetMulti() const
{//@CODE_1247
    return _multi;
}//@CODE_1247


void Relation::SetMulti(bool multi)
{//@CODE_1248
    _multi = multi;
}//@CODE_1248


const CbString& Relation::GetNote()
{//@CODE_1238
    return _note;
}//@CODE_1238


void Relation::SetNote(const CbString& rNote)
{//@CODE_1239
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_1239


bool Relation::GetOwned() const
{//@CODE_1250
    return _owned;
}//@CODE_1250


void Relation::SetOwned(bool owned)
{//@CODE_1251
    _owned = owned;
}//@CODE_1251


bool Relation::GetSingle() const
{//@CODE_1253
    return _single;
}//@CODE_1253


void Relation::SetSingle(bool single)
{//@CODE_1254
    _single = single;
}//@CODE_1254


bool Relation::GetStatic() const
{//@CODE_1256
    return _static;
}//@CODE_1256


void Relation::SetStatic(bool val)
{//@CODE_1257
    _static = val;
}//@CODE_1257


const CbString& Relation::GetToName()
{//@CODE_1241
    return _toName;
}//@CODE_1241


void Relation::SetToName(const CbString& rToName)
{//@CODE_1242
    if (_toName != rToName)
    {
        if (!_toName.IsEmpty())
        {
            CbString oldName[2];
            CbString newName[2];

            int cnt = 0;
            oldName[cnt]   = _toName + "Iterator";
            newName[cnt++] = rToName + "Iterator";
            oldName[cnt]   = "i" + _toName;
            newName[cnt++] = "i" + rToName;
            
            BaseClass::MethodIterator method(GetFromClass(), &Method::IsNonMacroMethod);
            while (++method)
            {
                for (int i = 0; i < cnt; i++)
                    method->ReplaceInCode(oldName[i], newName[i]);
            }
            
            oldName[0] = GetFromClass()->GetName() + "::" + oldName[0];
            newName[0] = GetFromClass()->GetName() + "::" + newName[0];

            DataModelDoc::BaseClassIterator baseClass(GetDataModelDoc());
            while (++baseClass)
            {
                BaseClass::MethodIterator method(baseClass, &Method::IsNonMacroMethod);
                while (++method)
                {
                    for (int i = 0; i < cnt; i++)
                        method->ReplaceInCode(oldName[i], newName[i]);
                }
            }

            FromRelation::MethodIterator iMethod(GetFromRelation());
            while (++iMethod)
            {
                iMethod->ChangeName(_toName, rToName);
            }

            if (GetFromRelation()->GetFromRelationMacroMethods())
            {
                FromRelationMacroMethods::MacroMethodIterator 
                    iMacroMethod(GetFromRelation()->GetFromRelationMacroMethods());
                while (++iMacroMethod)
                {
                    iMacroMethod->ChangeName(_toName, rToName);
                }
            }
        }

        _toName = rToName;
    }
}//@CODE_1242


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5510
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Relation::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
    CLEANUP_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)
}


/*@NOTE_201
Method which must be called first in a constructor
*/
void Relation::ConstructorInclude(Class* pFromClass, Class* pToClass)
{
    INIT_SINGLE_OWNED_ACTIVE(Relation, Relation, FromRelation, FromRelation)
    INIT_SINGLE_OWNED_ACTIVE(Relation, Relation, ToRelation, ToRelation)
    INIT_SINGLE_OWNED_ACTIVE(Relation, Relation, RelationMember, RelationMember)
    INIT_MULTI_OWNED_ACTIVE(Relation, Relation, RelationShape, RelationShape)
    INIT_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
    INIT_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)
}


/*@NOTE_203
Method which must be called first in a destructor
*/
void Relation::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(Relation, Relation, FromRelation, FromRelation)
    EXIT_SINGLE_OWNED_ACTIVE(Relation, Relation, ToRelation, ToRelation)
    EXIT_SINGLE_OWNED_ACTIVE(Relation, Relation, RelationMember, RelationMember)
    EXIT_MULTI_OWNED_ACTIVE(Relation, Relation, RelationShape, RelationShape)
    EXIT_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
    EXIT_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)
}


/*@NOTE_5511
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Relation::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(Relation, Relation, RelationShape, RelationShape)
    REMOVE_SINGLE_OWNED_ACTIVE(Relation, Relation, RelationMember, RelationMember)
    REMOVE_SINGLE_OWNED_ACTIVE(Relation, Relation, ToRelation, ToRelation)
    REMOVE_SINGLE_OWNED_ACTIVE(Relation, Relation, FromRelation, FromRelation)
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)
    REMOVE_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
}


/*@NOTE_5512
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Relation::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Relation* pRelation = (Relation*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
    RESTORE_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5514
Save the state of the current object relations to pDataModelDocObject.
*/
void Relation::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    Relation* pRelation = (Relation*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
    SAVE_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)
}


/*@NOTE_206
Serialize the members only to a CbObject object
*/
void Relation::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _fromName;
        archive << _note;
        archive << _toName;
        archive << _critical;
        archive << _multi;
        archive << _owned;
        archive << _single;
        archive << _static;
        archive << _filter;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _fromName;
            archive >> _note;
            archive >> _toName;
            archive >> _critical;
            archive >> _multi;
            archive >> _owned;
            archive >> _single;
            archive >> _static;
            archive >> _filter;
        }
    }
}


/*@NOTE_205
Method which must be called first in a serialize constructor
*/
void Relation::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(Relation, Relation, FromRelation, FromRelation)
    INIT_SINGLE_ACTIVE(Relation, Relation, ToRelation, ToRelation)
    INIT_SINGLE_ACTIVE(Relation, Relation, RelationMember, RelationMember)
    INIT_MULTI_ACTIVE(Relation, Relation, RelationShape, RelationShape)
    INIT_MULTI_PASSIVE(Class, FromClass, Relation, FromRelation)
    INIT_MULTI_PASSIVE(Class, ToClass, Relation, ToRelation)
}


/*@NOTE_208
Serialize the relations to a CbObject object
*/
void Relation::SerializeRelations(CbArchive& archive,
                                  DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(Relation, Relation, FromRelation, FromRelation)
        WRITE_SINGLE_ACTIVE(Relation, Relation, ToRelation, ToRelation)
        WRITE_SINGLE_ACTIVE(Relation, Relation, RelationMember, RelationMember)
        WRITE_MULTI_ACTIVE(Relation, Relation, RelationShape, RelationShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(Relation, Relation, FromRelation, FromRelation)
            READ_SINGLE_ACTIVE(Relation, Relation, ToRelation, ToRelation)
            READ_SINGLE_ACTIVE(Relation, Relation, RelationMember, RelationMember)
            READ_MULTI_ACTIVE(Relation, Relation, RelationShape, RelationShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Relation)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(Relation, Relation, FromRelation, FromRelation)
METHODS_SINGLE_OWNED_ACTIVE(Relation, Relation, ToRelation, ToRelation)
METHODS_SINGLE_OWNED_ACTIVE(Relation, Relation, RelationMember, RelationMember)
METHODS_MULTI_OWNED_ACTIVE(Relation, Relation, RelationShape, RelationShape)
METHODS_ITERATOR_MULTI_ACTIVE(Relation, Relation, RelationShape, RelationShape)
METHODS_MULTI_OWNED_PASSIVE(Class, FromClass, Relation, FromRelation)
METHODS_MULTI_OWNED_PASSIVE(Class, ToClass, Relation, ToRelation)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
