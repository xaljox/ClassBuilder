/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Variable.h
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Variable'
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
#ifndef _VARIABLE_H
#define _VARIABLE_H

//@START_USER1
//@END_USER1



class Variable
    : public Gti
{
    CB_DECLARE_SERIAL(Variable)
    RELATION_MULTI_OWNED_PASSIVE(Type, Type, Variable, Variable)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;
    CbString _note;
    bool _array;
    bool _const;
    bool _pointer;
    bool _reference;
    unsigned int _arraySize;
    bool _pointerPointer;
    CbString _template;
    bool _constPointer;
    CbString _arraySizeStr;

protected:

public:

// Methods
private:
    void ConstructorInclude(Type* pType);
    void DestructorInclude();
    void ReplaceConstructorInclude(Variable* pOld);
    void SerializeConstructorInclude();

protected:
    Variable();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Variable(Type* pType);
    Variable(Variable& rVariable);
    Variable(Variable* pOld);
    Variable(Type* pType, Variable* pVariable);
    virtual ~Variable();
    Variable& CopyValuesFrom(Variable& rVariable);
    CbString GetFirstLowerName();
    CbString GetFirstUpperName();
    const CbString GetTypeName();
    const CbString GetVariableName();
    int IsSimilar(Variable* pVariable);
    void ReplaceInNote(const CbString& oldString, const CbString& newString);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    bool GetArray();
    void SetArray(bool array);
    unsigned int GetArraySize();
    void SetArraySize(unsigned int arraySize);
    const CbString& GetArraySizeStr() const;
    void SetArraySizeStr(const CbString& rArraySizeStr);
    bool GetConst();
    void SetConst(bool val);
    bool GetConstPointer() const;
    void SetConstPointer(bool constPointer);
    const CbString& GetName();
    virtual void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    bool GetPointer();
    void SetPointer(bool pointer);
    bool GetPointerPointer();
    void SetPointerPointer(bool pointerPointer);
    bool GetReference();
    void SetReference(bool reference);
    CbString GetTemplate();
    void SetTemplate(const CbString& rTemplate);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _VARIABLE_H_INLINES
#define _VARIABLE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
