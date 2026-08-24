/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ExceptionSpecificationType.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ExceptionSpecificationType'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _EXCEPTIONSPECIFICATIONTYPE_H
#define _EXCEPTIONSPECIFICATIONTYPE_H

//@START_USER1
//@END_USER1



class ExceptionSpecificationType
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(ExceptionSpecificationType)
    RELATION_MULTI_OWNED_PASSIVE(ExceptionSpecification, ExceptionSpecification, ExceptionSpecificationType, ExceptionSpecificationType)
    RELATION_MULTI_OWNED_PASSIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)

//@START_USER2
//@END_USER2

// Members
private:
    bool _array;
    CbString _arraySizeStr;
    bool _const;
    bool _constPointer;
    bool _pointer;
    bool _pointerPointer;
    bool _reference;
    CbString _template;

protected:

public:

// Methods
private:
    void ConstructorInclude(ExceptionSpecification* pExceptionSpecification,
                            Type* pType);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ExceptionSpecificationType();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ExceptionSpecificationType(ExceptionSpecification* pExceptionSpecification,
                               Type* pType);
    virtual ~ExceptionSpecificationType();
    const CbString GetTypeName();
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoRemoved();
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    bool GetArray() const;
    void SetArray(bool array);
    const CbString& GetArraySizeStr() const;
    void SetArraySizeStr(const CbString& rArraySizeStr);
    bool GetConst() const;
    void SetConst(bool val);
    bool GetConstPointer() const;
    void SetConstPointer(bool constPointer);
    bool GetPointer() const;
    void SetPointer(bool pointer);
    bool GetPointerPointer() const;
    void SetPointerPointer(bool pointerPointer);
    bool GetReference() const;
    void SetReference(bool reference);
    const CbString& GetTemplate() const;
    void SetTemplate(const CbString& rTemplate);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _EXCEPTIONSPECIFICATIONTYPE_H_INLINES
#define _EXCEPTIONSPECIFICATIONTYPE_H_INLINES

/*@NOTE_22859
Returns the value of member '_array'.
*/
inline bool ExceptionSpecificationType::GetArray() const
{//@CODE_22859
    return _array;
}//@CODE_22859



/*@NOTE_22860
Set the value of member '_array' to 'array'.
*/
inline void ExceptionSpecificationType::SetArray(bool array)
{//@CODE_22860
    _array = array;
}//@CODE_22860



/*@NOTE_22862
Returns the value of member '_arraySizeStr'.
*/
inline const CbString& ExceptionSpecificationType::GetArraySizeStr() const
{//@CODE_22862
    return _arraySizeStr;
}//@CODE_22862



/*@NOTE_22863
Set the value of member '_arraySizeStr' to 'rArraySizeStr'.
*/
inline void ExceptionSpecificationType::SetArraySizeStr(const CbString& rArraySizeStr)
{//@CODE_22863
    _arraySizeStr = rArraySizeStr;
}//@CODE_22863



/*@NOTE_22865
Returns the value of member '_const'.
*/
inline bool ExceptionSpecificationType::GetConst() const
{//@CODE_22865
    return _const;
}//@CODE_22865



/*@NOTE_22866
Set the value of member '_const' to 'val'.
*/
inline void ExceptionSpecificationType::SetConst(bool val)
{//@CODE_22866
    _const = val;
}//@CODE_22866



/*@NOTE_22868
Returns the value of member '_constPointer'.
*/
inline bool ExceptionSpecificationType::GetConstPointer() const
{//@CODE_22868
    return _constPointer;
}//@CODE_22868



/*@NOTE_22869
Set the value of member '_constPointer' to 'constPointer'.
*/
inline void ExceptionSpecificationType::SetConstPointer(bool constPointer)
{//@CODE_22869
    _constPointer = constPointer;
}//@CODE_22869



/*@NOTE_22871
Returns the value of member '_pointer'.
*/
inline bool ExceptionSpecificationType::GetPointer() const
{//@CODE_22871
    return _pointer;
}//@CODE_22871



/*@NOTE_22872
Set the value of member '_pointer' to 'pointer'.
*/
inline void ExceptionSpecificationType::SetPointer(bool pointer)
{//@CODE_22872
    _pointer = pointer;
}//@CODE_22872



/*@NOTE_22874
Returns the value of member '_pointerPointer'.
*/
inline bool ExceptionSpecificationType::GetPointerPointer() const
{//@CODE_22874
    return _pointerPointer;
}//@CODE_22874



/*@NOTE_22875
Set the value of member '_pointerPointer' to 'pointerPointer'.
*/
inline void ExceptionSpecificationType::SetPointerPointer(bool pointerPointer)
{//@CODE_22875
    _pointerPointer = pointerPointer;
}//@CODE_22875



/*@NOTE_22877
Returns the value of member '_reference'.
*/
inline bool ExceptionSpecificationType::GetReference() const
{//@CODE_22877
    return _reference;
}//@CODE_22877



/*@NOTE_22878
Set the value of member '_reference' to 'reference'.
*/
inline void ExceptionSpecificationType::SetReference(bool reference)
{//@CODE_22878
    _reference = reference;
}//@CODE_22878



/*@NOTE_22881
Returns the value of member '_template'.
*/
inline const CbString& ExceptionSpecificationType::GetTemplate() const
{//@CODE_22881
    return _template;
}//@CODE_22881



/*@NOTE_22882
Set the value of member '_template' to 'rTemplate'.
*/
inline void ExceptionSpecificationType::SetTemplate(const CbString& rTemplate)
{//@CODE_22882
    _template = rTemplate;
}//@CODE_22882



//@START_USER3
//@END_USER3

#endif
#endif
