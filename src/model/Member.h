/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Member.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Member'
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
#ifndef _MEMBER_H
#define _MEMBER_H

//@START_USER1
//@END_USER1



class Member
    : public Variable
{
    CB_DECLARE_SERIAL(Member)
    RELATION_MULTI_OWNED_ACTIVE(Member, Member, MemberMethod, Method)
    RELATION_MULTI_OWNED_ACTIVE(Member, Member, MemberArgument, MemberArgument)
    RELATION_SINGLE_OWNED_ACTIVE(Member, Member, GetMemberMethod, GetMemberMethod)
    RELATION_SINGLE_OWNED_ACTIVE(Member, Member, SetMemberMethod, SetMemberMethod)
    RELATION_MULTI_OWNED_ACTIVE(Member, Member, RelationMember, RelationMember)
    RELATION_MULTI_OWNED_ACTIVE(Member, Member, MemberShape, MemberShape)
    RELATION_MULTI_OWNED_ACTIVE(Member, Member, MemberContext, MemberContext)
    RELATION_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Member, Member)
    RELATION_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Member, Member)

//@START_USER2
//@END_USER2

// Members
private:
    AccessType _access;
    CbString _initialization;
    bool _delete;
    bool _serialize;
    bool _static;
    bool _bitField;
    unsigned int _bitFieldSize;
    bool _mutable;

protected:

public:

// Methods
private:
    void DropOnClass(bool ctrlKeyDown, Gti* pGtiDrop);
    void DropOnMethod(bool ctrlKeyDown, Gti* pGtiDrop);
    void InitPhase();
    void ConstructorInclude(BaseClass* pBaseClass);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Member();
    void SetItemText();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Member(BaseClass* pBaseClass, Type* pType);
    Member(BaseClass* pBaseClass, Member* pMember);
    Member(BaseClass* pBaseClass, Type* pType, Member* pMember);
    virtual ~Member();
    virtual void Add();
    static int CompareTree(Member* pMember1, Member* pMember2);
    Member& CopyValuesFrom(Member& rMember);
    virtual Context* CreateContext(ContextDeclaration* pContextDeclaration);
    virtual void Delete();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    MemberArgument* FindMemberArgument(Method* pMethod);
    MemberShape* FindMemberShape(ClassShape* pClassShape);
    MemberMethod* FindMethod(const CbString& rName);
    MemberMethod* FindSimilarMethod(Method* pMethod);
    virtual CbString GetContextList();
    CbString GetEndContextDeclaration();
    CbString GetEndContextImplementation();
    virtual Context* GetFirstContext();
    virtual Context* GetNextContext(Context* pContextPos);
    CbString GetPrefixedName();
    CbString GetShapeText(VerbosityType verbosity = VERBOSITY_OFF);
    CbString GetStartContextDeclaration();
    CbString GetStartContextImplementation();
    bool IsNonStatic() const;
    bool IsPrivate() const;
    bool IsProtected() const;
    bool IsPublic() const;
    virtual int OnAddMethod(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void OnUndoRedoRemoved();
    virtual void SetName(const CbString& rName);
    virtual bool ShownByFilter(TreeViewModel* pTreeViewModel);
    virtual void Update();
    AccessType GetAccess();
    void SetAccess(AccessType access);
    bool GetBitField() const;
    void SetBitField(bool bitField);
    unsigned int GetBitFieldSize() const;
    void SetBitFieldSize(unsigned int bitFieldSize);
    bool GetDelete() const;
    void SetDelete(bool val);
    const CbString& GetInitialization();
    void SetInitialization(const CbString& rInitialization);
    bool GetMutable() const;
    void SetMutable(bool val);
    bool GetSerialize() const;
    void SetSerialize(bool serialize);
    bool GetStatic() const;
    void SetStatic(bool val);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MEMBER_H_INLINES
#define _MEMBER_H_INLINES

/*@NOTE_7530
Returns the value of member '_bitField'.
*/
inline bool Member::GetBitField() const
{//@CODE_7530
    return _bitField;
}//@CODE_7530



/*@NOTE_7531
Set the value of member '_bitField' to 'bitField'.
*/
inline void Member::SetBitField(bool bitField)
{//@CODE_7531
    _bitField = bitField;
}//@CODE_7531



/*@NOTE_7534
Returns the value of member '_bitFieldSize'.
*/
inline unsigned int Member::GetBitFieldSize() const
{//@CODE_7534
    return _bitFieldSize;
}//@CODE_7534



/*@NOTE_7535
Set the value of member '_bitFieldSize' to 'bitFieldSize'.
*/
inline void Member::SetBitFieldSize(unsigned int bitFieldSize)
{//@CODE_7535
    _bitFieldSize = bitFieldSize;
}//@CODE_7535



//@START_USER3
//@END_USER3

#endif
#endif
