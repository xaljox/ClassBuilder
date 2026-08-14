/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Variable.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Variable'
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


Variable::Variable(Type* pType) //@INIT_770
    : Gti(pType->GetDataModelDoc())
    , _name("")
    , _note("")
    , _array(0)
    , _const(0)
    , _pointer(0)
    , _pointerPointer(0)
    , _reference(0)
    , _arraySize(0)
    , _constPointer(0)
    , _arraySizeStr("")
    , _template(pType->GetTemplate())
{//@CODE_770
    ConstructorInclude(pType);

    // Put in your own code
}//@CODE_770


Variable::Variable(Variable& rVariable) //@INIT_772
    : Gti(rVariable.GetDataModelDoc())
    , _name(rVariable._name)
    , _note(rVariable._note)
    , _array(rVariable._array)
    , _const(rVariable._const)
    , _pointer(rVariable._pointer)
    , _pointerPointer(rVariable._pointerPointer)
    , _reference(rVariable._reference)
    , _arraySize(rVariable._arraySize)
    , _constPointer(rVariable._constPointer)
    , _arraySizeStr(rVariable._arraySizeStr)
    , _template(rVariable._template)
{//@CODE_772
    ConstructorInclude(rVariable.GetType());

    SetPhase(rVariable.GetPhase());
}//@CODE_772


/*@NOTE_1568
Constructor needed for putting a new object in the old one's context
*/
Variable::Variable(Variable* pOld) //@INIT_1568
    : Gti(pOld)
{//@CODE_1568
    ReplaceConstructorInclude(pOld);

    _name = pOld->_name;
    _note = pOld->_note;
    _array = pOld->_array;
    _const = pOld->_const;
    _pointer = pOld->_pointer;
    _pointerPointer = pOld->_pointerPointer;
    _reference = pOld->_reference;
    _arraySize = pOld->_arraySize;
    _constPointer = pOld->_constPointer;
    _arraySizeStr = pOld->_arraySizeStr;
    _template = pOld->_template;

    // Put in your own code
}//@CODE_1568


/*@NOTE_7550
Constructor method needed to copy variable from one project to the other.
*/
Variable::Variable(Type* pType, Variable* pVariable) //@INIT_7550
    : Gti(pType->GetDataModelDoc())
    , _name(pVariable->_name)
    , _note(pVariable->_note)
    , _array(pVariable->_array)
    , _const(pVariable->_const)
    , _pointer(pVariable->_pointer)
    , _reference(pVariable->_reference)
    , _arraySize(pVariable->_arraySize)
    , _pointerPointer(pVariable->_pointerPointer)
    , _constPointer(pVariable->_constPointer)
    , _arraySizeStr(pVariable->_arraySizeStr)
    , _template(pVariable->_template)
{//@CODE_7550
    ConstructorInclude(pType);

    // Put in your own code

    SetPhase(pVariable->GetPhase());
}//@CODE_7550


/*@NOTE_139
Constructor needed for serialization, not meant to use for other purposes!
*/
Variable::Variable() //@INIT_139
    : Gti()
    , _pointerPointer(0)
    , _constPointer(0)
    , _arraySizeStr("")
{//@CODE_139
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_139


/*@NOTE_137
Destructor method
*/
Variable::~Variable()
{//@CODE_137
    DestructorInclude();

    // Put in your own code
}//@CODE_137


Variable& Variable::CopyValuesFrom(Variable& rVariable)
{//@CODE_1594
    _name = rVariable._name;
    _note = rVariable._note;
    _array = rVariable._array;
    _const = rVariable._const;
    _pointer = rVariable._pointer;
    _pointerPointer = rVariable._pointerPointer;
    _constPointer = rVariable._constPointer;
    _reference = rVariable._reference;
    _arraySizeStr = rVariable._arraySizeStr;
    _template = rVariable._template;
    
    return *this;
}//@CODE_1594


CbString Variable::GetFirstLowerName()
{//@CODE_1389
    CbString firstLowerName = _name.Left(1);
    firstLowerName.MakeLower();
    firstLowerName += _name.Right(_name.GetLength()-1);

    return firstLowerName;
}//@CODE_1389


CbString Variable::GetFirstUpperName()
{//@CODE_1390
    CbString firstUpperName = _name.Left(1);
    firstUpperName.MakeUpper();
    firstUpperName += _name.Right(_name.GetLength()-1);

    return firstUpperName;
}//@CODE_1390


const CbString Variable::GetTypeName()
{//@CODE_776
    CbString typeName;
    if (GetConst())
        typeName += "const ";

    typeName += GetType()->Type::GetName() + GetTemplate();
    if (_pointer)
        typeName += "*";
    if (_constPointer)
        typeName += " const ";
    if (_pointerPointer)
        typeName += "*";
    if (_reference)
        typeName += "&";

    if (!typeName.IsEmpty() && typeName != "..." && 
		typeName.GetAt(typeName.GetLength()-1) != ' ')
	{
        typeName += " ";
	}

    return typeName;
}//@CODE_776


const CbString Variable::GetVariableName()
{//@CODE_777
    CbString variableName = GetName();
    if (GetArray())
    {
        variableName += "[" + GetArraySizeStr() + "]";
    }

    return variableName;
}//@CODE_777


int Variable::IsSimilar(Variable* pVariable)
{//@CODE_774
    if (this != pVariable && // similar, not exactly the same object
        _const == pVariable->_const &&
        _reference == pVariable->_reference &&
        _pointer == pVariable->_pointer &&
        _pointerPointer == pVariable->_pointerPointer &&
        _constPointer == pVariable->_constPointer &&
        _array == pVariable->_array &&
        _arraySizeStr == pVariable->_arraySizeStr)
    {
        return 1;
    }
    
    return 0;
}//@CODE_774


void Variable::ReplaceInNote(const CbString& oldString,
                             const CbString& newString)
{//@CODE_7419
    ReplaceInStr(_note, oldString, newString);

}//@CODE_7419


void Variable::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23078
    if (ReplaceInStr(_template, oldString, newString))
    {
        Update();
    }
}//@CODE_23078


bool Variable::GetArray()
{//@CODE_1263
    return _array;
}//@CODE_1263


void Variable::SetArray(bool array)
{//@CODE_1264
    _array = array;
}//@CODE_1264


unsigned int Variable::GetArraySize()
{//@CODE_1275
    return _arraySize;
}//@CODE_1275


void Variable::SetArraySize(unsigned int arraySize)
{//@CODE_1276
    _arraySize = arraySize;
}//@CODE_1276


/*@NOTE_19513
Returns the value of member '_arraySizeStr'.
*/
const CbString& Variable::GetArraySizeStr() const
{//@CODE_19513
    return _arraySizeStr;
}//@CODE_19513


/*@NOTE_19514
Set the value of member '_arraySizeStr' to 'rArraySizeStr'.
*/
void Variable::SetArraySizeStr(const CbString& rArraySizeStr)
{//@CODE_19514
    _arraySizeStr = rArraySizeStr;
}//@CODE_19514


bool Variable::GetConst()
{//@CODE_1266
    return _const;
}//@CODE_1266


void Variable::SetConst(bool val)
{//@CODE_1267
    _const = val;
}//@CODE_1267


/*@NOTE_19509
Returns the value of member '_constPointer'.
*/
bool Variable::GetConstPointer() const
{//@CODE_19509
    return _constPointer;
}//@CODE_19509


/*@NOTE_19510
Set the value of member '_constPointer' to 'constPointer'.
*/
void Variable::SetConstPointer(bool constPointer)
{//@CODE_19510
    _constPointer = constPointer;
}//@CODE_19510


/*@NOTE_4788
Returns the value of member '_name'.
*/
const CbString& Variable::GetName()
{//@CODE_4788
    return _name;
}//@CODE_4788


/*@NOTE_4789
Set the value of member '_name' to 'rName'.
*/
void Variable::SetName(const CbString& rName)
{//@CODE_4789
    if (_name != rName)
    {
        SaveState();

        _name = rName;
    }
}//@CODE_4789


const CbString& Variable::GetNote()
{//@CODE_1260
    return _note;
}//@CODE_1260


void Variable::SetNote(const CbString& rNote)
{//@CODE_1261
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_1261


bool Variable::GetPointer()
{//@CODE_1269
    return _pointer;
}//@CODE_1269


void Variable::SetPointer(bool pointer)
{//@CODE_1270
    _pointer = pointer;
}//@CODE_1270


/*@NOTE_4378
Returns the value of member '_pointerPointer'.
*/
bool Variable::GetPointerPointer()
{//@CODE_4378
    return _pointerPointer;
}//@CODE_4378


/*@NOTE_4379
Set the value of member '_pointerPointer' to 'pointerPointer'.
*/
void Variable::SetPointerPointer(bool pointerPointer)
{//@CODE_4379
    _pointerPointer = pointerPointer;
}//@CODE_4379


bool Variable::GetReference()
{//@CODE_1272
    return _reference;
}//@CODE_1272


void Variable::SetReference(bool reference)
{//@CODE_1273
    _reference = reference;
}//@CODE_1273


/*@NOTE_7440
Returns the value of member '_template'.
*/
CbString Variable::GetTemplate()
{//@CODE_7440
    return DataModel::ConvertToHtmlStringIfNeeded(_template);
}//@CODE_7440


/*@NOTE_7441
Set the value of member '_template' to 'rTemplate'.
*/
void Variable::SetTemplate(const CbString& rTemplate)
{//@CODE_7441
    _template = rTemplate;
}//@CODE_7441


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5570
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Variable::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_136
Method which must be called first in a constructor
*/
void Variable::ConstructorInclude(Type* pType)
{
    INIT_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_138
Method which must be called first in a destructor
*/
void Variable::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_5571
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Variable::RemoveReferences()
{
    Gti::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_1570
Method which must be called first in a replace constructor
*/
void Variable::ReplaceConstructorInclude(Variable* pOld)
{
    REPLACE_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_5572
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Variable::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Variable* pVariable = (Variable*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5574
Save the state of the current object relations to pDataModelDocObject.
*/
void Variable::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    Variable* pVariable = (Variable*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_141
Serialize the members only to a CbObject object
*/
void Variable::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
        archive << _note;
        archive << _array;
        archive << _const;
        archive << _pointer;
        archive << _reference;
        archive << _arraySize;
        archive << _pointerPointer;
        archive << _template;
        archive << _constPointer;
        archive << _arraySizeStr;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
            archive >> _note;
            archive >> _array;
            archive >> _const;
            archive >> _pointer;
            archive >> _reference;
            archive >> _arraySize;
            archive >> _pointerPointer;
            archive >> _template;
            archive >> _constPointer;
            archive >> _arraySizeStr;
        }
    }
}


/*@NOTE_140
Method which must be called first in a serialize constructor
*/
void Variable::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Type, Type, Variable, Variable)
}


/*@NOTE_143
Serialize the relations to a CbObject object
*/
void Variable::SerializeRelations(CbArchive& archive,
                                  DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Variable)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
