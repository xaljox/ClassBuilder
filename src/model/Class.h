/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Class.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Class'
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
#ifndef _CLASS_H
#define _CLASS_H

//@START_USER1
//@END_USER1



class Class
    : public ExternClass
{
    CB_DECLARE_SERIAL(Class)
    RELATION_MULTI_OWNED_ACTIVE(Class, FromClass, Relation, FromRelation)
    RELATION_MULTI_OWNED_ACTIVE(Class, ToClass, Relation, ToRelation)
    RELATION_SINGLE_OWNED_ACTIVE(Class, Class, ConstructorIncludeMethod, ConstructorIncludeMethod)
    RELATION_MULTI_OWNED_ACTIVE(Class, RefClass, IsClassMethod, IsClassMethod)
    RELATION_MULTI_OWNED_ACTIVE(Class, Class, ClassContext, ClassContext)
    RELATION_MULTI_PASSIVE(ClassGroup, ClassGroup, Class, Class)
    RELATION_MULTI_OWNED_PASSIVE(DataModel, DataModel, Class, Class)
    RELATION_SINGLE_PASSIVE(DataModel, Document, Class, Document)
    RELATION_SINGLE_PASSIVE(DataModel, DocumentObject, Class, DocumentObject)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _cppFile;
    CbString _cppHeader;
    CbString _cppUser1;
    CbString _cppUser2;
    CbString _hFile;
    CbString _hHeader;
    CbString _hUser1;
    CbString _hUser2;
    CbString _modified;
    CbString _note;
    int _flag;
    bool _replace;
    bool _dllExport;
    bool _serialize;
    CbString _cppUser3;
    CbString _hUser3;
    bool _relationMacrosLast;

protected:

public:

// Methods
private:
    int HasPureVirtualMethod();
    void SourceCheck(SourceLogInterface* pDialog);
    void ConstructorInclude(DataModel* pDataModel);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    Class();
    void UpdateCppHeader();
    int UpdateHHeader();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Class(DataModel* pDataModel);
    virtual ~Class();
    virtual void Add();
    void AddModified(const CbString& val);
    static int CompareName(Class* pA, Class* pB);
    static int ComparePhase(Class* pA, Class* pB);
    virtual Context* CreateContext(ContextDeclaration* pContextDeclaration);
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    Relation* FindFromRelation(const CbString& rFromName,
                               const CbString& rToName);
    CbString GetBaseName();
    virtual CbString GetContextList();
    CbString GetCppFileWithoutPath();
    CbString GetEndContextDeclaration();
    CbString GetEndContextImplementation();
    virtual Context* GetFirstContext();
    CbString GetHFileWithoutPath();
    CbString GetNewBaseName();
    virtual Context* GetNextContext(Context* pContextPos);
    CbString GetStartContextDeclaration();
    CbString GetStartContextImplementation();
    void InitMemberPrefix();
    void InitPhase();
    void MoveFromRelation(Relation* pRelation);
    void MoveToRelation(Relation* pRelation);
    virtual void NotifyAddMember(Member* pMember);
    virtual void NotifyRemoveMember(Member* pMember);
    virtual int OnAddClass(bool checkOnly = false);
    virtual int OnAddIsClassMethods(bool checkOnly = false);
    virtual int OnAddRelation(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnPaste(Gti* pGti, bool checkOnly = false);
    void ReadCppFile(ParseLogInterface* pDialog, bool unconditional);
    void ReadHFile(ParseLogInterface* pDialog, bool unconditional);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    void SetName(const CbString& rName);
    virtual bool SetTemplate(const CbString& rTemplateDeclaration,
                             const CbString& rTemplate);
    virtual bool ShownByFilter(TreeViewModel* pTreeViewModel);
    virtual int SortOnName(bool checkOnly = false);
    virtual int SortOnPhase(bool checkOnly = false);
    virtual void Update();
    void WriteCppFile(SourceLogInterface* pDialog, bool unconditional = false);
    void WriteCppFileBody(CbStringBuilder& str);
    int WriteHFile(SourceLogInterface* pDialog, bool unconditional = false);
    void WriteHFileBody(CbStringBuilder& str);
    void WriteRecursiveInclude(CbString& str);
    const CbString& GetCppFile();
    void SetCppFile(const CbString& rCppFile);
    const CbString& GetCppHeader();
    void SetCppHeader(const CbString& rCppHeader);
    const CbString& GetCppUser1();
    void SetCppUser1(const CbString& rCppUser1);
    const CbString& GetCppUser2();
    void SetCppUser2(const CbString& rCppUser2);
    const CbString& GetCppUser3();
    void SetCppUser3(const CbString& rCppUser3);
    bool GetDllExport();
    void SetDllExport(bool dllExport);
    int GetFlag();
    void SetFlag(int flag = 0);
    const CbString& GetHFile();
    void SetHFile(const CbString& rHFile);
    const CbString& GetHHeader();
    void SetHHeader(const CbString& rHHeader);
    const CbString& GetHUser1();
    void SetHUser1(const CbString& rHUser1);
    const CbString& GetHUser2();
    void SetHUser2(const CbString& rHUser2);
    const CbString& GetHUser3() const;
    void SetHUser3(const CbString& rHUser3);
    const CbString& GetModified();
    void SetModified(const CbString& rModified);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    bool GetRelationMacrosLast() const;
    void SetRelationMacrosLast(bool relationMacrosLast);
    bool GetReplace() const;
    void SetReplace(bool replace);
    bool GetSerialize() const;
    void SetSerialize(bool serialize);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CLASS_H_INLINES
#define _CLASS_H_INLINES

/*@NOTE_35983
Returns the value of member '_relationMacrosLast'.
*/
inline bool Class::GetRelationMacrosLast() const
{//@CODE_35983
    return _relationMacrosLast;
}//@CODE_35983



/*@NOTE_35984
Set the value of member '_relationMacrosLast' to 'relationMacrosLast'.
*/
inline void Class::SetRelationMacrosLast(bool relationMacrosLast)
{//@CODE_35984
    _relationMacrosLast = relationMacrosLast;
}//@CODE_35984



//@START_USER3
//@END_USER3

#endif
#endif
