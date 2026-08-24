/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          Gti.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Gti'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _GTI_H
#define _GTI_H

//@START_USER1
//@END_USER1



class Gti
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(Gti)
    RELATION_MULTI_ACTIVE(Gti, Parent, Gti, Child)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    RELATION_MULTI_PASSIVE(Gti, Parent, Gti, Child)

//@START_USER2
//@END_USER2

// Members
private:
    bool _added;
    CbString _itemText;
    int _icon;
    int _initialVersion;
    int _version;
    int _order;
    CbString _addInString;
    Phase _phase;
    unsigned int _state;
    static Gti* _pGtiCopy;

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void ReplaceConstructorInclude(Gti* pOld);
    void SerializeConstructorInclude();

protected:
    Gti();
    void AddRecursive();
    void RemoveRecursive();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Gti(DataModelDoc* pDataModelDoc);
    Gti(Gti* pOld);
    virtual ~Gti();
    virtual void Add();
    static int CompareTreeOrder(Gti* pGti1, Gti* pGti2);
    virtual Context* CreateContext(ContextDeclaration* pContextDeclaration);
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    Gti* FindStringFiltered(CbString str, bool matchCase = true,
                            bool wholeName = false, bool searchTypes = true,
                            bool searchMethods = true,
                            bool searchArguments = true,
                            bool searchMembers = true);
    virtual Context* GetFirstContext();
    virtual Gti* GetNext(Gti* pGti = 0);
    virtual Context* GetNextContext(Context* pContextPos);
    int GetStateIcon();
    int IsAllowedToEditPhase(Phase phase) const;
    virtual int OnAddActor(bool checkOnly = false);
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnAddClass(bool checkOnly = false);
    virtual int OnAddClassDiagram(bool checkOnly = false);
    virtual int OnAddConstructor(bool checkOnly = false);
    virtual int OnAddGroup(bool checkOnly = false);
    virtual int OnAddInherit(bool checkOnly = false);
    virtual int OnAddIsClassMethods(bool checkOnly = false);
    virtual int OnAddMember(bool checkOnly = false);
    virtual int OnAddMetaGroup(bool checkOnly = false);
    virtual int OnAddMethod(bool checkOnly = false);
    virtual int OnAddRelation(bool checkOnly = false);
    virtual int OnAddSequenceDiagram(bool checkOnly = false);
    virtual int OnAddType(bool checkOnly = false);
    virtual int OnAddVirtuals(bool checkOnly = false);
    virtual int OnCopy(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnEditExceptionSpecification(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void OnUndoRedoChanging(DataModelDocObject* pNewState);
    virtual void OnUndoRedoRemoving();
    void Remove();
    virtual void SetPhaseDownAndUpwards(Phase phase);
    void SetPhaseUpwards(Phase phase);
    virtual bool ShownByFilter(TreeViewModel* pTreeViewModel);
    virtual int SortOnName(bool checkOnly = false);
    virtual int SortOnPhase(bool checkOnly = false);
    virtual void Update();
    bool GetAdded();
    void SetAdded(bool added);
    const CbString& GetAddInString() const;
    void SetAddInString(const CbString& addInString);
    int GetIcon();
    void SetIcon(int icon);
    int GetInitialVersion();
    void SetInitialVersion(int initialVersion);
    CbString GetItemText();
    void SetItemText(const CbString& rItemText);
    int GetOrder();
    void SetOrder(int order);
    static Gti* GetGtiCopy();
    static void SetGtiCopy(Gti* pGtiCopy);
    Phase GetPhase() const;
    void SetPhase(Phase phase);
    unsigned int GetState() const;
    void SetState(unsigned int state);
    int GetVersion();
    void SetVersion(int version);
    virtual void CleanupReferences();
    bool IsActor() const;
    bool IsActors() const;
    bool IsArgument() const;
    bool IsBaseClass() const;
    bool IsClass() const;
    bool IsClassDiagram() const;
    bool IsClassGroup() const;
    bool IsCleanupReferencesMethod() const;
    bool IsConstructor() const;
    bool IsConstructorIncludeMethod() const;
    bool IsDataModel() const;
    bool IsDestructor() const;
    bool IsDestructorIncludeMethod() const;
    bool IsExternClass() const;
    bool IsExternClasses() const;
    bool IsFindAvlTreeMethod() const;
    bool IsFindEqualOrBiggerAvlTreeMethod() const;
    bool IsFindEqualOrSmallerAvlTreeMethod() const;
    bool IsFindMethod() const;
    bool IsFindReverseAvlTreeMethod() const;
    bool IsFindReverseValueTreeMethod() const;
    bool IsFindUniqueValueTreeMethod() const;
    bool IsFindValueTreeMethod() const;
    bool IsFixedMethod() const;
    bool IsFromRelation() const;
    bool IsFromRelationMacroMethods() const;
    bool IsFromRelationMethod() const;
    bool IsGetMemberMethod() const;
    bool IsGroup() const;
    bool IsInherit() const;
    bool IsIsClassMethod() const;
    bool IsMacroMethod() const;
    bool IsMacroMethods() const;
    bool IsMember() const;
    bool IsMemberAndMethodGroup() const;
    bool IsMemberArgument() const;
    bool IsMemberMethod() const;
    bool IsMetaGroup() const;
    bool IsMethod() const;
    bool IsMultiMacroMethods() const;
    bool IsMultiOwnedMacroMethods() const;
    bool IsOtherType() const;
    bool IsOtherTypes() const;
    bool IsRemoveReferencesMethod() const;
    bool IsReplaceConstructor() const;
    bool IsReplaceConstructorIncludeMethod() const;
    bool IsRestoreReferencesMethod() const;
    bool IsSaveReferencesMethod() const;
    bool IsSequenceDiagram() const;
    bool IsSerializeConstructor() const;
    bool IsSerializeConstructorIncludeMethod() const;
    bool IsSerializeMethod() const;
    bool IsSerializeRelationsMethod() const;
    bool IsSetMemberMethod() const;
    bool IsSingleMacroMethods() const;
    bool IsSingleOwnedMacroMethods() const;
    bool IsStaticMultiMacroMethods() const;
    bool IsStaticMultiOwnedMacroMethods() const;
    bool IsToRelation() const;
    bool IsToRelationMacroMethods() const;
    bool IsType() const;
    bool IsVariable() const;
    bool IsWrapMemberMethod() const;
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _GTI_H_INLINES
#define _GTI_H_INLINES

/*@NOTE_5815
Returns the value of member '_added'.
*/
inline bool Gti::GetAdded()
{//@CODE_5815
    return _added;
}//@CODE_5815



/*@NOTE_5816
Set the value of member '_added' to 'added'.
*/
inline void Gti::SetAdded(bool added)
{//@CODE_5816
    _added = added;
}//@CODE_5816



/*@NOTE_36025
Returns the value of member '_addInString'.
*/
inline const CbString& Gti::GetAddInString() const
{//@CODE_36025
    return _addInString;
}//@CODE_36025



/*@NOTE_36026
Set the value of member '_addInString' to 'addInString'.
*/
inline void Gti::SetAddInString(const CbString& addInString)
{//@CODE_36026
    _addInString = addInString;
}//@CODE_36026



/*@NOTE_40786
Returns the value of member '_pGtiCopy'.
*/
inline Gti* Gti::GetGtiCopy()
{//@CODE_40786
    return _pGtiCopy;
}//@CODE_40786



/*@NOTE_40787
Set the value of member '_pGtiCopy' to 'pGtiCopy'.
*/
inline void Gti::SetGtiCopy(Gti* pGtiCopy)
{//@CODE_40787
    _pGtiCopy = pGtiCopy;
}//@CODE_40787



/*@NOTE_23446
Returns the value of member '_phase'.
*/
inline Phase Gti::GetPhase() const
{//@CODE_23446
    return _phase;
}//@CODE_23446



/*@NOTE_35842
Returns the value of member '_state'.
*/
inline unsigned int Gti::GetState() const
{//@CODE_35842
    return _state;
}//@CODE_35842



/*@NOTE_35843
Set the value of member '_state' to 'state'.
*/
inline void Gti::SetState(unsigned int state)
{//@CODE_35843
    _state = state;
}//@CODE_35843



//@START_USER3
//@END_USER3

#endif
#endif
