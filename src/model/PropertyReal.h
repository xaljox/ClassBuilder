/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          PropertyReal.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'PropertyReal'
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
#ifndef _PROPERTYREAL_H
#define _PROPERTYREAL_H

//@START_USER1
//@END_USER1



class PropertyReal
    : public Property
{
    CB_DECLARE_SERIAL(PropertyReal)

//@START_USER2
//@END_USER2

// Members
private:
    double _value;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    PropertyReal();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    PropertyReal(DataModelDocObject* pDataModelDocObject, CbString name,
                 double value);
    virtual ~PropertyReal();
    double GetValue() const;
    void SetValue(double value);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _PROPERTYREAL_H_INLINES
#define _PROPERTYREAL_H_INLINES

/*@NOTE_36232
Returns the value of member '_value'.
*/
inline double PropertyReal::GetValue() const
{//@CODE_36232
    return _value;
}//@CODE_36232



/*@NOTE_36233
Set the value of member '_value' to 'value'.
*/
inline void PropertyReal::SetValue(double value)
{//@CODE_36233
    _value = value;
}//@CODE_36233



//@START_USER3
//@END_USER3

#endif
#endif
