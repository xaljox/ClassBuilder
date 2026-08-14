/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ContextDeclaration.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ContextDeclaration'
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
#ifndef _CONTEXTDECLARATION_H
#define _CONTEXTDECLARATION_H

//@START_USER1
//@END_USER1



class ContextDeclaration
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(ContextDeclaration)
    RELATION_MULTI_OWNED_ACTIVE(ContextDeclaration, ContextDeclaration, Context, Context)
    RELATION_MULTI_OWNED_PASSIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _startContextDeclaration;
    CbString _endContextDeclaration;
    CbString _startContextImplementation;
    CbString _endContextImplementation;
    CbString _name;
    CbString _defineDeclaration;
    bool _enableDefineDeclaration;
    CbString _note;

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModel* pDataModel);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ContextDeclaration();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ContextDeclaration(DataModel* pDataModel, CbString name);
    virtual ~ContextDeclaration();
    Context* FindContext(Gti* pGti);
    const CbString& GetDefineDeclaration() const;
    void SetDefineDeclaration(const CbString& rDefineDeclaration);
    bool GetEnableDefineDeclaration() const;
    void SetEnableDefineDeclaration(bool enableDefineDeclaration);
    const CbString& GetEndContextDeclaration() const;
    void SetEndContextDeclaration(const CbString& rEndContextDeclaration);
    const CbString& GetEndContextImplementation() const;
    void SetEndContextImplementation(const CbString& rEndContextImplementation);
    const CbString& GetName() const;
    void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    const CbString& GetStartContextDeclaration() const;
    void SetStartContextDeclaration(const CbString& rStartContextDeclaration);
    const CbString& GetStartContextImplementation() const;
    void SetStartContextImplementation(const CbString& rStartContextImplementation);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CONTEXTDECLARATION_H_INLINES
#define _CONTEXTDECLARATION_H_INLINES

/*@NOTE_27288
Returns the value of member '_defineDeclaration'.
*/
inline const CbString& ContextDeclaration::GetDefineDeclaration() const
{//@CODE_27288
    return _defineDeclaration;
}//@CODE_27288



/*@NOTE_27292
Returns the value of member '_enableDefineDeclaration'.
*/
inline bool ContextDeclaration::GetEnableDefineDeclaration() const
{//@CODE_27292
    return _enableDefineDeclaration;
}//@CODE_27292



/*@NOTE_27293
Set the value of member '_enableDefineDeclaration' to 'enableDefineDeclaration'.
*/
inline void ContextDeclaration::SetEnableDefineDeclaration(bool enableDefineDeclaration)
{//@CODE_27293
    _enableDefineDeclaration = enableDefineDeclaration;
}//@CODE_27293



/*@NOTE_23537
Returns the value of member '_endContextDeclaration'.
*/
inline const CbString& ContextDeclaration::GetEndContextDeclaration() const
{//@CODE_23537
    return _endContextDeclaration;
}//@CODE_23537



/*@NOTE_23545
Returns the value of member '_endContextImplementation'.
*/
inline const CbString& ContextDeclaration::GetEndContextImplementation() const
{//@CODE_23545
    return _endContextImplementation;
}//@CODE_23545



/*@NOTE_26152
Returns the value of member '_name'.
*/
inline const CbString& ContextDeclaration::GetName() const
{//@CODE_26152
    return _name;
}//@CODE_26152



/*@NOTE_27296
Returns the value of member '_note'.
*/
inline const CbString& ContextDeclaration::GetNote()
{//@CODE_27296
    return _note;
}//@CODE_27296



/*@NOTE_23532
Returns the value of member '_endContextDeclaration'.
*/
inline const CbString& ContextDeclaration::GetStartContextDeclaration() const
{//@CODE_23532
    return _startContextDeclaration;
}//@CODE_23532



/*@NOTE_23541
Returns the value of member '_endContextImplementation'.
*/
inline const CbString& ContextDeclaration::GetStartContextImplementation() const
{//@CODE_23541
    return _startContextImplementation;
}//@CODE_23541



//@START_USER3
//@END_USER3

#endif
#endif
