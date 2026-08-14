/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          BaseClass.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'BaseClass'
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
#ifndef _BASECLASS_H
#define _BASECLASS_H

//@START_USER1
//@END_USER1



class BaseClass
    : public Type
{
    CB_DECLARE_SERIAL(BaseClass)
    RELATION_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Inherit, Inherit)
    RELATION_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Method, Method)
    RELATION_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, Member, Member)
    RELATION_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, MemberAndMethodGroup, MemberAndMethodGroup)
    RELATION_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    RELATION_MULTI_OWNED_ACTIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)

//@START_USER2
//@END_USER2

// Members
private:
    bool _struct;
    CbString _templateDeclaration;
    CbString _template;
    bool _suppressForwardDeclaration;

protected:
    CbString _memberPrefix;

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void ReplaceConstructorInclude(BaseClass* pOld);
    void SerializeConstructorInclude();

protected:
    BaseClass();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    BaseClass(DataModelDoc* pDataModelDoc);
    BaseClass(BaseClass* pOld);
    BaseClass(OtherType* pOld);
    BaseClass(DataModelDoc* pDataModelDoc, BaseClass* pBaseClass);
    virtual ~BaseClass();
    virtual void Add();
    CbString DefineTemplate();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    ClassShape* FindClassShape(ClassDiagram* pClassDiagram,
                               ClassShape* startAfterClassShape = 0);
    Member* FindMember(const CbString& rName);
    Method* FindMethodWithId(unsigned int id);
    Method* FindMethodWithName(const CbString& name);
    Method* FindSimilarMethod(Method* pMethod);
    virtual CbString GetName();
    CbString GetStrippedTemplateDeclaration();
    virtual CbString GetTemplateDefine();
    int IsInheritBase(ExternClass* pExternClass);
    void MoveMember(Member* pMember);
    virtual void NotifyAddMember(Member* pMember);
    virtual void NotifyAddMethod(Method* pMethod);
    virtual void NotifyRemoveMember(Member* pMember);
    virtual void NotifyRemoveMethod(Method* pMethod);
    virtual int OnAddClassDiagram(bool checkOnly = false);
    virtual int OnAddConstructor(bool checkOnly = false);
    virtual int OnAddGroup(bool checkOnly = false);
    virtual int OnAddMember(bool checkOnly = false);
    virtual int OnAddMethod(bool checkOnly = false);
    virtual int OnAddSequenceDiagram(bool checkOnly = false);
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    void SetName(const CbString& rName);
    virtual bool SetTemplate(const CbString& rTemplateDeclaration,
                             const CbString& rTemplate);
    static void StripTemplateDeclaration(CbString& templateDeclaration);
    virtual void Update();
    const CbString& GetMemberPrefix() const;
    void SetMemberPrefix(const CbString& rMemberPrefix);
    bool GetStruct();
    void SetStruct(bool structX);
    bool GetSuppressForwardDeclaration() const;
    void SetSuppressForwardDeclaration(bool suppressForwardDeclaration);
    CbString GetTemplate();
    void SetTemplate(const CbString& rTemplate);
    CbString GetTemplateDeclaration();
    void SetTemplateDeclaration(const CbString& rTemplateDeclaration);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _BASECLASS_H_INLINES
#define _BASECLASS_H_INLINES

/*@NOTE_4921
Set the value of member '_struct' to 'struct'.
*/
inline void BaseClass::SetStruct(bool structX)
{//@CODE_4921
    _struct = structX;
}//@CODE_4921



/*@NOTE_23030
Returns the value of member '_suppressForwardDeclaration'.
*/
inline bool BaseClass::GetSuppressForwardDeclaration() const
{//@CODE_23030
    return _suppressForwardDeclaration;
}//@CODE_23030



/*@NOTE_23031
Set the value of member '_suppressForwardDeclaration' to 'suppressForwardDeclaration'.
*/
inline void BaseClass::SetSuppressForwardDeclaration(bool suppressForwardDeclaration)
{//@CODE_23031
    _suppressForwardDeclaration = suppressForwardDeclaration;
}//@CODE_23031



//@START_USER3
//@END_USER3

#endif
#endif
