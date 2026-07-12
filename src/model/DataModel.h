/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DataModel.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DataModel'
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
#ifndef _DATAMODEL_H
#define _DATAMODEL_H

//@START_USER1
//@END_USER1



class DataModel
    : public Gti
{
    CB_DECLARE_SERIAL(DataModel)
    RELATION_MULTI_OWNED_ACTIVE(DataModel, DataModel, Class, Class)
    RELATION_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    RELATION_SINGLE_ACTIVE(DataModel, Document, Class, Document)
    RELATION_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
    RELATION_MULTI_OWNED_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
    RELATION_MULTI_OWNED_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    RELATION_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _author;
    CbString _cppHeader;
    CbString _hFile;
    CbString _hHeader;
    CbString _hUser1;
    CbString _hUser2;
    CbString _memberPrefix;
    CbString _name;
    CbString _note;
    CbTime _lastSave;
    bool _stdAfx;
    bool _serialize;
    CbString _styleDataModelHeading;
    CbString _styleGroupHeading;
    CbString _styleClassHeading;
    CbString _styleItemsHeading;
    CbString _styleItem;
    CbString _styleItemExplanation;
    CbString __notUsed_rtfTemplate;
    CbString __notUsed_rtfOutput;
    CbString _styleExplanation;
    bool _publicMethods;
    bool _protectedMethods;
    bool _privateMethods;
    bool _privateMembers;
    bool _protectedMembers;
    bool _publicMembers;
    CbString _classPrefix;
    int _indentSize;
    CbString __notUsed_schemaFile;
    CbString _htmlOutput;
    CbString _newClassPrefix;
    CbTime _maxLastSave;
    CbString _styleFigureText;
    bool _undoRedo;
    CbString _namespace;
    bool _relationMethods;
    static bool _htmlMode;
    bool _modifiers;
    bool _getSetMethods;
    bool _classBuilderMethods;
    bool _phaseSupport;
    bool _templateClassHeaderOnly;
    bool _includeContextDeclarations;
    CbTime __notUsed_lastSaveRtfDocumentation;
    bool _classOverview;
    bool _relationOverview;
    bool _includeSequenceDiagramObjects;
    bool _includeSequenceDiagramMessages;
    bool _showDllExport;
    int __notUsed_rtfDiagramFormat;
    bool _crlf;

protected:

public:

// Methods
private:
    void InitDocumentObjectUndoRedo();
    void InitDocumentUndoRedo(Class* pUndoBase);
    void InitSerialize(CbString className);
    void InitUndoRedo();
    void InitUndoRedoBase(Class*& pUndoBase, Class*& pRedoBase);
    void InitUndoRedoChange(Class* pUndoBase, Class* pRedoBase,
                            Class*& pUndoChange, Class*& pRedoChange);
    void InitUndoRedoDelete(Class* pUndoBase, Class* pRedoBase,
                            Class*& pUndoDelete, Class*& pRedoDelete);
    void InitUndoRedoNew(Class* pUndoBase, Class* pRedoBase, Class*& pUndoNew,
                         Class*& pRedoNew);
    void InitUndoSubChange(Class* pUndoBase, Class*& pUndoSubChange);
    void InitUndoSubDelete(Class* pUndoBase, Class*& pUndoSubDelete);
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    DataModel();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DataModel(DataModelDoc* pDataModelDoc);
    virtual ~DataModel();
    virtual void Add();
    void AddSerialize(CbString className);
    bool CheckUpdates();
    static CbString ConvertToHtmlStringIfNeeded(CbString str);
    Class* FindClass(const CbString& rName);
    CbString GetHFileWithoutPath();
    virtual Gti* GetNext(Gti* pGti = 0);
    void Init(CbString className);
    virtual int OnAddClass(bool checkOnly = false);
    virtual int OnAddClassDiagram(bool checkOnly = false);
    virtual int OnAddGroup(bool checkOnly = false);
    virtual int OnAddMetaGroup(bool checkOnly = false);
    virtual int OnAddSequenceDiagram(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    void ReadAllFiles(ParseLogInterface* pDialog, bool unconditional = false);
    void ReadHFile(ParseLogInterface* pDialog, bool unconditional);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    void SaveAllFiles(SourceLogInterface* pDialog);
    void SaveModifiedFiles(SourceLogInterface* pDialog);
    virtual int SortOnName(bool checkOnly = false);
    virtual int SortOnPhase(bool checkOnly = false);
    virtual void Update();
    void WriteHFile(SourceLogInterface* pDialog, bool unconditional = false);
    void WriteHFile(CbStringBuilder& str, int& startDateTime, int& endDateTime);
    const CbString& GetAuthor();
    void SetAuthor(const CbString& rAuthor);
    bool GetClassBuilderMethods() const;
    void SetClassBuilderMethods(bool classBuilderMethods);
    bool GetClassOverview() const;
    void SetClassOverview(bool classOverview);
    const CbString& GetClassPrefix();
    void SetClassPrefix(const CbString& rClassPrefix);
    const CbString& GetCppHeader();
    void SetCppHeader(const CbString& rCppHeader);
    bool GetCrlf() const;
    void SetCrlf(bool crlf);
    bool GetGetSetMethods() const;
    void SetGetSetMethods(bool getSetMethods);
    const CbString& GetHFile();
    void SetHFile(const CbString& rHFile);
    const CbString& GetHHeader();
    void SetHHeader(const CbString& rHHeader);
    static bool GetHtmlMode();
    static void SetHtmlMode(bool htmlMode);
    const CbString& GetHtmlOutput();
    void SetHtmlOutput(const CbString& rHtmlOutput);
    const CbString& GetHUser1();
    void SetHUser1(const CbString& rHUser1);
    const CbString& GetHUser2();
    void SetHUser2(const CbString& rHUser2);
    bool GetIncludeContextDeclarations() const;
    void SetIncludeContextDeclarations(bool includeContextDeclarations);
    bool GetIncludeSequenceDiagramMessages() const;
    void SetIncludeSequenceDiagramMessages(bool includeSequenceDiagramMessages);
    bool GetIncludeSequenceDiagramObjects() const;
    void SetIncludeSequenceDiagramObjects(bool includeSequenceDiagramObjects);
    int GetIndentSize();
    void SetIndentSize(int indentSize);
    const CbTime& GetLastSave();
    void SetLastSave(const CbTime& rLastSave);
    const CbTime& GetMaxLastSave();
    void SetMaxLastSave(const CbTime& maxLastSave);
    const CbString& GetMemberPrefix();
    void SetMemberPrefix(const CbString& rMemberPrefix);
    bool GetModifiers() const;
    void SetModifiers(bool modifiers);
    const CbString& GetName();
    void SetName(const CbString& rName);
    const CbString& GetNamespace() const;
    void SetNamespace(const CbString& rNamespace);
    const CbString& GetNewClassPrefix();
    void SetNewClassPrefix(const CbString& rNewClassPrefix);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    bool GetPhaseSupport() const;
    void SetPhaseSupport(bool phaseSupport);
    bool GetPrivateMembers();
    void SetPrivateMembers(bool privateMembers);
    bool GetPrivateMethods();
    void SetPrivateMethods(bool privateMethods);
    bool GetProtectedMembers();
    void SetProtectedMembers(bool protectedMembers);
    bool GetProtectedMethods();
    void SetProtectedMethods(bool protectedMethods);
    bool GetPublicMembers();
    void SetPublicMembers(bool publicMembers);
    bool GetPublicMethods();
    void SetPublicMethods(bool publicMethods);
    bool GetRelationMethods() const;
    void SetRelationMethods(bool relationMethods);
    bool GetRelationOverview() const;
    void SetRelationOverview(bool relationOverview);
    bool GetSerialize();
    void SetSerialize(bool serialize);
    bool GetShowDllExport() const;
    void SetShowDllExport(bool showDllExport);
    bool GetStdAfx();
    void SetStdAfx(bool stdAfx);
    const CbString& GetStyleClassHeading();
    void SetStyleClassHeading(const CbString& rStyleClassHeading);
    const CbString& GetStyleDataModelHeading();
    void SetStyleDataModelHeading(const CbString& rStyleDataModelHeading);
    const CbString& GetStyleExplanation();
    void SetStyleExplanation(const CbString& rStyleExplanation);
    const CbString& GetStyleFigureText();
    void SetStyleFigureText(const CbString& rStyleFigureText);
    const CbString& GetStyleGroupHeading();
    void SetStyleGroupHeading(const CbString& rStyleGroupHeading);
    const CbString& GetStyleItem();
    void SetStyleItem(const CbString& rStyleItem);
    const CbString& GetStyleItemExplanation();
    void SetStyleItemExplanation(const CbString& rStyleItemExplanation);
    const CbString& GetStyleItemsHeading();
    void SetStyleItemsHeading(const CbString& rStyleItemsHeading);
    bool GetTemplateClassHeaderOnly() const;
    void SetTemplateClassHeaderOnly(bool templateClassHeaderOnly);
    bool GetUndoRedo();
    void SetUndoRedo(bool undoRedo);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _DATAMODEL_H_INLINES
#define _DATAMODEL_H_INLINES

/*@NOTE_19541
Returns the value of member '_classBuilderMethods'.
*/
inline bool DataModel::GetClassBuilderMethods() const
{//@CODE_19541
    return _classBuilderMethods;
}//@CODE_19541



/*@NOTE_19542
Set the value of member '_classBuilderMethods' to 'classBuilderMethods'.
*/
inline void DataModel::SetClassBuilderMethods(bool classBuilderMethods)
{//@CODE_19542
    _classBuilderMethods = classBuilderMethods;
}//@CODE_19542



/*@NOTE_27616
Returns the value of member '_relationOverview'.
*/
inline bool DataModel::GetClassOverview() const
{//@CODE_27616
    return _classOverview;
}//@CODE_27616



/*@NOTE_27617
Set the value of member '_relationOverview' to 'classOverview'.
*/
inline void DataModel::SetClassOverview(bool classOverview)
{//@CODE_27617
    _classOverview = classOverview;
}//@CODE_27617



/*@NOTE_41500
Returns the value of member '_crlf'.
*/
inline bool DataModel::GetCrlf() const
{//@CODE_41500
    return _crlf;
}//@CODE_41500



/*@NOTE_41501
Set the value of member '_crlf' to 'crlf'.
*/
inline void DataModel::SetCrlf(bool crlf)
{//@CODE_41501
    _crlf = crlf;
}//@CODE_41501



/*@NOTE_19537
Returns the value of member '_getSetMethods'.
*/
inline bool DataModel::GetGetSetMethods() const
{//@CODE_19537
    return _getSetMethods;
}//@CODE_19537



/*@NOTE_19538
Set the value of member '_getSetMethods' to 'getSetMethods'.
*/
inline void DataModel::SetGetSetMethods(bool getSetMethods)
{//@CODE_19538
    _getSetMethods = getSetMethods;
}//@CODE_19538



/*@NOTE_16943
Returns the value of member '_htmlMode'.
*/
inline bool DataModel::GetHtmlMode()
{//@CODE_16943
    return _htmlMode;
}//@CODE_16943



/*@NOTE_16944
Set the value of member '_htmlMode' to 'htmlMode'.
*/
inline void DataModel::SetHtmlMode(bool htmlMode)
{//@CODE_16944
    _htmlMode = htmlMode;
}//@CODE_16944



/*@NOTE_27300
Returns the value of member '_includeContextDeclarations'.
*/
inline bool DataModel::GetIncludeContextDeclarations() const
{//@CODE_27300
    return _includeContextDeclarations;
}//@CODE_27300



/*@NOTE_27301
Set the value of member '_includeContextDeclarations' to 'includeContextDeclarations'.
*/
inline void DataModel::SetIncludeContextDeclarations(bool includeContextDeclarations)
{//@CODE_27301
    _includeContextDeclarations = includeContextDeclarations;
}//@CODE_27301



/*@NOTE_34319
Returns the value of member '_includeSequenceDiagramMessages'.
*/
inline bool DataModel::GetIncludeSequenceDiagramMessages() const
{//@CODE_34319
    return _includeSequenceDiagramMessages;
}//@CODE_34319



/*@NOTE_34320
Set the value of member '_includeSequenceDiagramMessages' to 'includeSequenceDiagramMessages'.
*/
inline void DataModel::SetIncludeSequenceDiagramMessages(bool includeSequenceDiagramMessages)
{//@CODE_34320
    _includeSequenceDiagramMessages = includeSequenceDiagramMessages;
}//@CODE_34320



/*@NOTE_34315
Returns the value of member '_includeSequenceDiagramMessages'.
*/
inline bool DataModel::GetIncludeSequenceDiagramObjects() const
{//@CODE_34315
    return _includeSequenceDiagramObjects;
}//@CODE_34315



/*@NOTE_34316
Set the value of member '_includeSequenceDiagramMessages' to 'includeSequenceDiagramObjects'.
*/
inline void DataModel::SetIncludeSequenceDiagramObjects(bool includeSequenceDiagramObjects)
{//@CODE_34316
    _includeSequenceDiagramObjects = includeSequenceDiagramObjects;
}//@CODE_34316



/*@NOTE_19533
Returns the value of member '_modifiers'.
*/
inline bool DataModel::GetModifiers() const
{//@CODE_19533
    return _modifiers;
}//@CODE_19533



/*@NOTE_19534
Set the value of member '_modifiers' to 'modifiers'.
*/
inline void DataModel::SetModifiers(bool modifiers)
{//@CODE_19534
    _modifiers = modifiers;
}//@CODE_19534



/*@NOTE_6116
Returns the value of member '_namespace'.
*/
inline const CbString& DataModel::GetNamespace() const
{//@CODE_6116
    return _namespace;
}//@CODE_6116



/*@NOTE_6117
Set the value of member '_namespace' to 'rNamespace'.
*/
inline void DataModel::SetNamespace(const CbString& rNamespace)
{//@CODE_6117
    _namespace = rNamespace;
}//@CODE_6117



/*@NOTE_23442
Returns the value of member '_phaseSupport'.
*/
inline bool DataModel::GetPhaseSupport() const
{//@CODE_23442
    return _phaseSupport;
}//@CODE_23442



/*@NOTE_7500
Returns the value of member '_relationMethods'.
*/
inline bool DataModel::GetRelationMethods() const
{//@CODE_7500
    return _relationMethods;
}//@CODE_7500



/*@NOTE_7501
Set the value of member '_relationMethods' to 'relationMethods'.
*/
inline void DataModel::SetRelationMethods(bool relationMethods)
{//@CODE_7501
    _relationMethods = relationMethods;
}//@CODE_7501



/*@NOTE_27620
Returns the value of member '_relationOverview'.
*/
inline bool DataModel::GetRelationOverview() const
{//@CODE_27620
    return _relationOverview;
}//@CODE_27620



/*@NOTE_27621
Set the value of member '_relationOverview' to 'relationOverview'.
*/
inline void DataModel::SetRelationOverview(bool relationOverview)
{//@CODE_27621
    _relationOverview = relationOverview;
}//@CODE_27621



/*@NOTE_36254
Returns the value of member '_showDllExport'.
*/
inline bool DataModel::GetShowDllExport() const
{//@CODE_36254
    return _showDllExport;
}//@CODE_36254



/*@NOTE_23488
Returns the value of member '_templateClassHeaderOnly'.
*/
inline bool DataModel::GetTemplateClassHeaderOnly() const
{//@CODE_23488
    return _templateClassHeaderOnly;
}//@CODE_23488



/*@NOTE_23489
Set the value of member '_templateClassHeaderOnly' to 'templateClassHeaderOnly'.
*/
inline void DataModel::SetTemplateClassHeaderOnly(bool templateClassHeaderOnly)
{//@CODE_23489
    _templateClassHeaderOnly = templateClassHeaderOnly;
}//@CODE_23489



//@START_USER3
//@END_USER3

#endif
#endif
