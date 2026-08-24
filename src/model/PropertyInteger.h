/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          PropertyInteger.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'PropertyInteger'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _PROPERTYINTEGER_H
#define _PROPERTYINTEGER_H

//@START_USER1
//@END_USER1



class PropertyInteger
    : public Property
{
    CB_DECLARE_SERIAL(PropertyInteger)

//@START_USER2
//@END_USER2

// Members
private:
    int _value;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    PropertyInteger();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    PropertyInteger(DataModelDocObject* pDataModelDocObject, CbString name,
                    int value);
    virtual ~PropertyInteger();
    int GetValue() const;
    void SetValue(int value);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _PROPERTYINTEGER_H_INLINES
#define _PROPERTYINTEGER_H_INLINES

/*@NOTE_36227
Returns the value of member '_value'.
*/
inline int PropertyInteger::GetValue() const
{//@CODE_36227
    return _value;
}//@CODE_36227



/*@NOTE_36228
Set the value of member '_value' to 'value'.
*/
inline void PropertyInteger::SetValue(int value)
{//@CODE_36228
    _value = value;
}//@CODE_36228



//@START_USER3
//@END_USER3

#endif
#endif
