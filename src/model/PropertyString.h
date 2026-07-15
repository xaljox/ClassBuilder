/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          PropertyString.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'PropertyString'
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
#ifndef _PROPERTYSTRING_H
#define _PROPERTYSTRING_H

//@START_USER1
//@END_USER1



class PropertyString
    : public Property
{
    CB_DECLARE_SERIAL(PropertyString)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _value;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    PropertyString();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    PropertyString(DataModelDocObject* pDataModelDocObject, CbString name,
                   CbString value);
    virtual ~PropertyString();
    const CbString& GetValue() const;
    void SetValue(const CbString& value);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _PROPERTYSTRING_H_INLINES
#define _PROPERTYSTRING_H_INLINES

/*@NOTE_36235
Returns the value of member '_value'.
*/
inline const CbString& PropertyString::GetValue() const
{//@CODE_36235
    return _value;
}//@CODE_36235



/*@NOTE_36236
Set the value of member '_value' to 'value'.
*/
inline void PropertyString::SetValue(const CbString& value)
{//@CODE_36236
    _value = value;
}//@CODE_36236



//@START_USER3
//@END_USER3

#endif
#endif
