/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DataModelDoc.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DataModelDoc'
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
#ifndef _DATAMODELDOC_H
#define _DATAMODELDOC_H

//@START_USER1
//@END_USER1



class DataModelDoc
    : public CbObject
{
    CB_DECLARE_SERIAL(DataModelDoc)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Type, Type)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, BaseClass, BaseClass)
    RELATION_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
    RELATION_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
    RELATION_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, OtherTypes, OtherTypes)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Gti, Gti)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)
    RELATION_NOFILTER_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actor, Actor)
    RELATION_SINGLE_OWNED_ACTIVE(DataModelDoc, DataModelDoc, Actors, Actors)
    RELATION_MULTI_OWNED_ACTIVE(DataModelDoc, DataModelDoc, TreeViewModel, TreeViewModel)

//@START_USER2
    friend class CClassBuilderDoc;
    // CbViewLock is the ONLY way to lock/unlock views (RAII, coalesced
    // refresh) -- LockAllViews / UnLockAndUpdateAllViews are private.
    friend class CbViewLock;
//@END_USER2

// Members
private:
    CClassBuilderDoc* _pDoc;
    int _version;
    unsigned int _nextObjectId;
    int _currentUndoCount;
    int _maxUndoCount;
    int _isRedoing;
    int _isUndoing;
    CbString _additionalAllowed;
    CbColorRef _activationNoMethodPenColor;
    CbColorRef _activationPenColor;
    CbColorRef _lifeLinePenColor;
    CbColorRef _lifeLineTextColor;
    CbColorRef _SDNoteShapePenColor;
    CbColorRef _SDNoteShapeTextColor;
    CbColorRef _signalNoMethodPenColor;
    CbColorRef _signalPenColor;
    CbColorRef _signalTextColor;
    CbColorRef _classPenColor;
    CbColorRef _classTextColor;
    CbColorRef _memberTextColor;
    CbColorRef _methodTextColor;
    CbColorRef _relationPenColor;
    CbColorRef _relationTextColor;
    CbColorRef _dependencyPenColor;
    CbColorRef _dependencyTextColor;
    CbColorRef _relationDiagramOnlyPenColor;
    CbColorRef _relationDiagramOnlyTextColor;
    CbColorRef _inheritPenColor;
    CbColorRef _criticalRelationPenColor;
    CbColorRef _noteShapePenColor;
    CbColorRef _noteShapeTextColor;
    CbColorRef _activationUser3PenColor;
    CbString _lastSearch;
    bool _matchCase;
    CbString _methodNameList;
    CbString _similarLinesList;
    CbColorRef _activationInitialPenColor;
    CbString _commentInitialCode;
    static bool _membersOnly;

protected:
    static int _objectVersion;

public:

// Methods
private:
    void LockAllViews();
    void UnLockAndUpdateAllViews();
    void UpdateTreeViews();
    CClassBuilderDoc* GetDocument();
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DataModelDoc();
    virtual ~DataModelDoc();
    void AddClassDiagramShapes(ClassDiagram* pClassDiagram);
    void AddSequenceDiagramShapes(SequenceDiagram* pSequenceDiagram);
    bool CanRedo();
    bool CanUndo();
    void CleanRedo();
    void ConvertNonCSymbols(CbString& str);
    bool DeletedInOpenUndoStep(DataModelDocObject* pDataModelDocObject);
    BaseClass* FindBaseClass(const CbString& rName);
    DataModelDocObject* FindDataModelDocObject(unsigned int id);
    ExternClass* FindOrDuplicateExternClass(Type* pType);
    Type* FindOrDuplicateType(Type* pType);
    TreeViewModel* FindTreeViewModel(Gti* subTree);
    Type* FindType(const CbString& rName);
    UndoBase* GetLastMarked();
    CbString GetPath();
    CbString GetPathName();
    CbString GetTitle();
    bool HasNonCSymbols(const CbString& str);
    UndoBase* MarkLastUndo(int level = 1);
    void NotifyCdViews();
    void NotifySdViews();
    void NotifyStateChanged();
    void NotifyStructureChanged();
    void NotifyTreeViews();
    int OnSaveDocument(const char* path);
    void RecalculateAllDiagrams(bool cd = true, bool sd = true);
    int Redo();
    void RefreshObjectIds();
    void ReplaceReference(ConnectionSegment* pOld, ConnectionSegment* pNew);
    int RollBack(UndoBase* pUndoBase = 0, bool silent = false);
    void SaveState();
    void SerializeMembersOnly(CbArchive& archive);
    void SetModifiedFlag(int bModified = 1);
    int Undo(UndoBase* pUndoBase = 0);
    void UpdateClassDiagramViews();
    void UpdateSequenceDiagramViews();
    CbColorRef GetActivationInitialPenColor() const;
    void SetActivationInitialPenColor(CbColorRef activationInitialPenColor);
    CbColorRef GetActivationNoMethodPenColor() const;
    void SetActivationNoMethodPenColor(CbColorRef activationNoMethodPenColor);
    CbColorRef GetActivationPenColor() const;
    void SetActivationPenColor(CbColorRef activationPenColor);
    CbColorRef GetActivationUser3PenColor() const;
    void SetActivationUser3PenColor(CbColorRef activationUser3PenColor);
    const CbString& GetAdditionalAllowed() const;
    void SetAdditionalAllowed(const CbString& rAdditionalAllowed);
    CbColorRef GetClassPenColor() const;
    void SetClassPenColor(CbColorRef classPenColor);
    CbColorRef GetClassTextColor() const;
    void SetClassTextColor(CbColorRef classTextColor);
    const CbString& GetCommentInitialCode() const;
    void SetCommentInitialCode(const CbString& commentInitialCode);
    CbColorRef GetCriticalRelationPenColor() const;
    void SetCriticalRelationPenColor(CbColorRef criticalRelationPenColor);
    int GetCurrentUndoCount();
    CbColorRef GetDependencyPenColor() const;
    void SetDependencyPenColor(CbColorRef dependencyPenColor);
    CbColorRef GetDependencyTextColor() const;
    void SetDependencyTextColor(CbColorRef dependencyTextColor);
    CbColorRef GetInheritPenColor() const;
    void SetInheritPenColor(CbColorRef inheritPenColor);
    int GetIsRedoing() const;
    int GetIsUndoing();
    const CbString& GetLastSearch() const;
    void SetLastSearch(const CbString& rLastSearch);
    CbColorRef GetLifeLinePenColor() const;
    void SetLifeLinePenColor(CbColorRef lifeLinePenColor);
    CbColorRef GetLifeLineTextColor() const;
    void SetLifeLineTextColor(CbColorRef lifeLineTextColor);
    bool GetMatchCase() const;
    void SetMatchCase(bool matchCase);
    int GetMaxUndoCount();
    void SetMaxUndoCount(int maxUndoCount);
    static bool GetMembersOnly();
    CbColorRef GetMemberTextColor() const;
    void SetMemberTextColor(CbColorRef memberTextColor);
    const CbString& GetMethodNameList() const;
    void SetMethodNameList(const CbString& rMethodNameList);
    CbColorRef GetMethodTextColor() const;
    void SetMethodTextColor(CbColorRef methodTextColor);
    unsigned int GetNextObjectId();
    CbColorRef GetNoteShapePenColor() const;
    void SetNoteShapePenColor(CbColorRef noteShapePenColor);
    CbColorRef GetNoteShapeTextColor() const;
    void SetNoteShapeTextColor(CbColorRef noteShapeTextColor);
    void SetDocument(CClassBuilderDoc* pPDoc);
    CbColorRef GetRelationDiagramOnlyPenColor() const;
    void SetRelationDiagramOnlyPenColor(CbColorRef relationDiagramOnlyPenColor);
    CbColorRef GetRelationDiagramOnlyTextColor() const;
    void SetRelationDiagramOnlyTextColor(CbColorRef relationDiagramOnlyTextColor);
    CbColorRef GetRelationPenColor() const;
    void SetRelationPenColor(CbColorRef relationPenColor);
    CbColorRef GetRelationTextColor() const;
    void SetRelationTextColor(CbColorRef relationTextColor);
    CbColorRef GetSDNoteShapePenColor() const;
    void SetSDNoteShapePenColor(CbColorRef SDNoteShapePenColor);
    CbColorRef GetSDNoteShapeTextColor() const;
    void SetSDNoteShapeTextColor(CbColorRef SDNoteShapeTextColor);
    CbColorRef GetSignalNoMethodPenColor() const;
    void SetSignalNoMethodPenColor(CbColorRef signalNoMethodPenColor);
    CbColorRef GetSignalPenColor() const;
    void SetSignalPenColor(CbColorRef signalPenColor);
    CbColorRef GetSignalTextColor() const;
    void SetSignalTextColor(CbColorRef signalTextColor);
    const CbString& GetSimilarLinesList() const;
    void SetSimilarLinesList(const CbString& rSimilarLinesList);
    int GetVersion();
    void SetVersion(int version);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _DATAMODELDOC_H_INLINES
#define _DATAMODELDOC_H_INLINES

/*@NOTE_35975
Returns the value of member '_activationInitialPenColor'.
*/
inline CbColorRef DataModelDoc::GetActivationInitialPenColor() const
{//@CODE_35975
    return _activationInitialPenColor;
}//@CODE_35975



/*@NOTE_35976
Set the value of member '_activationInitialPenColor' to 'activationInitialPenColor'.
*/
inline void DataModelDoc::SetActivationInitialPenColor(CbColorRef activationInitialPenColor)
{//@CODE_35976
    _activationInitialPenColor = activationInitialPenColor;
    if (_activationInitialPenColor != activationInitialPenColor)
    {
        SaveState();
        _activationInitialPenColor = activationInitialPenColor;
    }
    
    DataModelDoc::SequenceDiagramIterator iSequenceDiagram(this);
    while (++iSequenceDiagram)
    {
        SequenceDiagram::SequenceDiagramShapeIterator 
            iSequenceDiagramShape(iSequenceDiagram);
        while (++iSequenceDiagramShape)
        {
            ChildActivationShape* pChildActivationShape = 
                dynamic_cast<ChildActivationShape*>(iSequenceDiagramShape.Get());
            if (pChildActivationShape && activationInitialPenColor !=
                pChildActivationShape->GetActivationInitialPenColor())
            {
                pChildActivationShape->SaveState(1);
                pChildActivationShape->SetActivationInitialPenColor(_activationInitialPenColor);
            }
        }
    }
}//@CODE_35976



/*@NOTE_34875
Returns the value of member '_activationNoMethodPenColor'.
*/
inline CbColorRef DataModelDoc::GetActivationNoMethodPenColor() const
{//@CODE_34875
    return _activationNoMethodPenColor;
}//@CODE_34875



/*@NOTE_34879
Returns the value of member '_activationPenColor'.
*/
inline CbColorRef DataModelDoc::GetActivationPenColor() const
{//@CODE_34879
    return _activationPenColor;
}//@CODE_34879



/*@NOTE_35354
Returns the value of member '_activationUser3PenColor'.
*/
inline CbColorRef DataModelDoc::GetActivationUser3PenColor() const
{//@CODE_35354
    return _activationUser3PenColor;
}//@CODE_35354



/*@NOTE_23056
Returns the value of member '_additionalAllowed'.
*/
inline const CbString& DataModelDoc::GetAdditionalAllowed() const
{//@CODE_23056
    return _additionalAllowed;
}//@CODE_23056



/*@NOTE_23057
Set the value of member '_additionalAllowed' to 'rAdditionalAllowed'.
*/
inline void DataModelDoc::SetAdditionalAllowed(const CbString& rAdditionalAllowed)
{//@CODE_23057
    _additionalAllowed = rAdditionalAllowed;
}//@CODE_23057



/*@NOTE_34931
Returns the value of member '_classPenColor'.
*/
inline CbColorRef DataModelDoc::GetClassPenColor() const
{//@CODE_34931
    return _classPenColor;
}//@CODE_34931



/*@NOTE_34936
Returns the value of member '_classTextColor'.
*/
inline CbColorRef DataModelDoc::GetClassTextColor() const
{//@CODE_34936
    return _classTextColor;
}//@CODE_34936



/*@NOTE_35992
Returns the value of member '_commentInitialCode'.
*/
inline const CbString& DataModelDoc::GetCommentInitialCode() const
{//@CODE_35992
    return _commentInitialCode;
}//@CODE_35992



/*@NOTE_35993
Set the value of member '_commentInitialCode' to 'commentInitialCode'.
*/
inline void DataModelDoc::SetCommentInitialCode(const CbString& commentInitialCode)
{//@CODE_35993
    _commentInitialCode = commentInitialCode;
}//@CODE_35993



/*@NOTE_35041
Returns the value of member '_criticalRelationPenColor'.
*/
inline CbColorRef DataModelDoc::GetCriticalRelationPenColor() const
{//@CODE_35041
    return _criticalRelationPenColor;
}//@CODE_35041



/*@NOTE_5098
Returns the value of member '_currentUndoCount'.
*/
inline int DataModelDoc::GetCurrentUndoCount()
{//@CODE_5098
    return _currentUndoCount;
}//@CODE_5098



/*@NOTE_34957
Returns the value of member '_dependencyPenColor'.
*/
inline CbColorRef DataModelDoc::GetDependencyPenColor() const
{//@CODE_34957
    return _dependencyPenColor;
}//@CODE_34957



/*@NOTE_34960
Returns the value of member '_dependencyTextColor'.
*/
inline CbColorRef DataModelDoc::GetDependencyTextColor() const
{//@CODE_34960
    return _dependencyTextColor;
}//@CODE_34960



/*@NOTE_34997
Returns the value of member '_inheritPenColor'.
*/
inline CbColorRef DataModelDoc::GetInheritPenColor() const
{//@CODE_34997
    return _inheritPenColor;
}//@CODE_34997



/*@NOTE_6104
Returns the value of member '_isRedoing'.
*/
inline int DataModelDoc::GetIsRedoing() const
{//@CODE_6104
    return _isRedoing;
}//@CODE_6104



/*@NOTE_5867
Returns the value of member '_isUndoing'.
*/
inline int DataModelDoc::GetIsUndoing()
{//@CODE_5867
    return _isUndoing;
}//@CODE_5867



/*@NOTE_35368
Returns the value of member '_lastSearch'.
*/
inline const CbString& DataModelDoc::GetLastSearch() const
{//@CODE_35368
    return _lastSearch;
}//@CODE_35368



/*@NOTE_35369
Set the value of member '_lastSearch' to 'rLastSearch'.
*/
inline void DataModelDoc::SetLastSearch(const CbString& rLastSearch)
{//@CODE_35369
    _lastSearch = rLastSearch;
}//@CODE_35369



/*@NOTE_34883
Returns the value of member '_lifeLinePenColor'.
*/
inline CbColorRef DataModelDoc::GetLifeLinePenColor() const
{//@CODE_34883
    return _lifeLinePenColor;
}//@CODE_34883



/*@NOTE_34887
Returns the value of member '_lifeLineTextColor'.
*/
inline CbColorRef DataModelDoc::GetLifeLineTextColor() const
{//@CODE_34887
    return _lifeLineTextColor;
}//@CODE_34887



/*@NOTE_35382
Returns the value of member '_matchCase'.
*/
inline bool DataModelDoc::GetMatchCase() const
{//@CODE_35382
    return _matchCase;
}//@CODE_35382



/*@NOTE_35383
Set the value of member '_matchCase' to 'matchCase'.
*/
inline void DataModelDoc::SetMatchCase(bool matchCase)
{//@CODE_35383
    _matchCase = matchCase;
}//@CODE_35383



/*@NOTE_5100
Returns the value of member '_maxUndoCount'.
*/
inline int DataModelDoc::GetMaxUndoCount()
{//@CODE_5100
    return _maxUndoCount;
}//@CODE_5100



/*@NOTE_5101
Set the value of member '_maxUndoCount' to 'maxUndoCount'.
*/
inline void DataModelDoc::SetMaxUndoCount(int maxUndoCount)
{//@CODE_5101
    _maxUndoCount = maxUndoCount;
}//@CODE_5101



/*@NOTE_38300
Returns the value of member '_membersOnly'.
*/
inline bool DataModelDoc::GetMembersOnly()
{//@CODE_38300
    return _membersOnly;
}//@CODE_38300



/*@NOTE_34940
Returns the value of member '_memberTextColor'.
*/
inline CbColorRef DataModelDoc::GetMemberTextColor() const
{//@CODE_34940
    return _memberTextColor;
}//@CODE_34940



/*@NOTE_35846
Returns the value of member '_methodNameList'.
*/
inline const CbString& DataModelDoc::GetMethodNameList() const
{//@CODE_35846
    return _methodNameList;
}//@CODE_35846



/*@NOTE_35847
Set the value of member '_methodNameList' to 'rMethodNameList'.
*/
inline void DataModelDoc::SetMethodNameList(const CbString& rMethodNameList)
{//@CODE_35847
    _methodNameList = rMethodNameList;
}//@CODE_35847



/*@NOTE_34944
Returns the value of member '_methodTextColor'.
*/
inline CbColorRef DataModelDoc::GetMethodTextColor() const
{//@CODE_34944
    return _methodTextColor;
}//@CODE_34944



/*@NOTE_35051
Returns the value of member '_noteShapePenColor'.
*/
inline CbColorRef DataModelDoc::GetNoteShapePenColor() const
{//@CODE_35051
    return _noteShapePenColor;
}//@CODE_35051



/*@NOTE_35055
Returns the value of member '_noteShapeTextColor'.
*/
inline CbColorRef DataModelDoc::GetNoteShapeTextColor() const
{//@CODE_35055
    return _noteShapeTextColor;
}//@CODE_35055



/*@NOTE_34993
Returns the value of member '_relationDiagramOnlyPenColor'.
*/
inline CbColorRef DataModelDoc::GetRelationDiagramOnlyPenColor() const
{//@CODE_34993
    return _relationDiagramOnlyPenColor;
}//@CODE_34993



/*@NOTE_34965
Returns the value of member '_relationDiagramOnlyTextColor'.
*/
inline CbColorRef DataModelDoc::GetRelationDiagramOnlyTextColor() const
{//@CODE_34965
    return _relationDiagramOnlyTextColor;
}//@CODE_34965



/*@NOTE_34948
Returns the value of member '_dependencyTextColor'.
*/
inline CbColorRef DataModelDoc::GetRelationPenColor() const
{//@CODE_34948
    return _relationPenColor;
}//@CODE_34948



/*@NOTE_34952
Returns the value of member '_dependencyTextColor'.
*/
inline CbColorRef DataModelDoc::GetRelationTextColor() const
{//@CODE_34952
    return _relationTextColor;
}//@CODE_34952



/*@NOTE_34891
Returns the value of member '_SDNoteShapePenColor'.
*/
inline CbColorRef DataModelDoc::GetSDNoteShapePenColor() const
{//@CODE_34891
    return _SDNoteShapePenColor;
}//@CODE_34891



/*@NOTE_34895
Returns the value of member '_SDNoteShapeTextColor'.
*/
inline CbColorRef DataModelDoc::GetSDNoteShapeTextColor() const
{//@CODE_34895
    return _SDNoteShapeTextColor;
}//@CODE_34895



/*@NOTE_34899
Returns the value of member '_signalNoMethodPenColor'.
*/
inline CbColorRef DataModelDoc::GetSignalNoMethodPenColor() const
{//@CODE_34899
    return _signalNoMethodPenColor;
}//@CODE_34899



/*@NOTE_34903
Returns the value of member '_signalPenColor'.
*/
inline CbColorRef DataModelDoc::GetSignalPenColor() const
{//@CODE_34903
    return _signalPenColor;
}//@CODE_34903



/*@NOTE_34910
Returns the value of member '_signalTextColor'.
*/
inline CbColorRef DataModelDoc::GetSignalTextColor() const
{//@CODE_34910
    return _signalTextColor;
}//@CODE_34910



/*@NOTE_35850
Returns the value of member '_similarLinesList'.
*/
inline const CbString& DataModelDoc::GetSimilarLinesList() const
{//@CODE_35850
    return _similarLinesList;
}//@CODE_35850



/*@NOTE_35851
Set the value of member '_similarLinesList' to 'rSimilarLinesList'.
*/
inline void DataModelDoc::SetSimilarLinesList(const CbString& rSimilarLinesList)
{//@CODE_35851
    _similarLinesList = rSimilarLinesList;
}//@CODE_35851



//@START_USER3
//@END_USER3

#endif
#endif
