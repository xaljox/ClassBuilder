/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Method.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Method'
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
#ifndef _METHOD_H
#define _METHOD_H

//@START_USER1
//@END_USER1



class Method
    : public Variable
{
    CB_DECLARE_SERIAL(Method)
    RELATION_MULTI_OWNED_ACTIVE(Method, Method, Argument, Argument)
    RELATION_NOFILTER_MULTI_OWNED_ACTIVE(Method, Method, WrapMemberMethod, WrapMemberMethod)
    RELATION_MULTI_OWNED_ACTIVE(Method, Method, MethodShape, MethodShape)
    RELATION_SINGLE_OWNED_ACTIVE(Method, Method, ExceptionSpecification, ExceptionSpecification)
    RELATION_MULTI_OWNED_ACTIVE(Method, Method, MethodContext, MethodContext)
    RELATION_MULTI_ACTIVE(Method, Method, ChildActivationShape, ChildActivationShape)
    RELATION_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, Method, Method)
    RELATION_MULTI_PASSIVE(MemberAndMethodGroup, MemberAndMethodGroup, Method, Method)

//@START_USER2
//@END_USER2

// Members
private:
    AccessType _access;
    QDialog* _pOpenDialog;
    bool _const;
    bool _inlineX;
    bool _pure;
    bool _static;
    bool _virtual;
    bool _dllExport;
    CbString _callingConvention;
    bool _declare;
    bool _implement;
    bool _delete;

protected:
    CbString _code;
    bool _untouched;

public:

// Methods
private:
    void InitPhase();
    void ConstructorInclude(BaseClass* pBaseClass);
    void DestructorInclude();
    void ReplaceConstructorInclude(Method* pOld);
    void SerializeConstructorInclude();

protected:
    Method();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Method(BaseClass* pBaseClass, Type* pType);
    Method(BaseClass* pBaseClass, Method* pMethod);
    Method(Method* pOld);
    Method(BaseClass* pBaseClass, Type* pType, Method* pMethod);
    virtual ~Method();
    virtual void Add();
    int ChangeName(CbString oldStr, CbString newStr);
    static int CompareFileSaveState(Method* pMethod1, Method* pMethod2);
    static int CompareTree(Method* pMethod1, Method* pMethod2);
    static int CompareTreeSaveState(Method* pMethod1, Method* pMethod2);
    Method& CopyValuesFrom(Method& rMethod);
    virtual Context* CreateContext(ContextDeclaration* pContextDeclaration);
    virtual void Delete();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    Argument* FindArgument(const CbString& rName);
    MethodShape* FindMethodShape(ClassShape* pClassShape);
    int FindStringInStrippedCode(const CbString& string);
    Argument* GetArgumentAtSamePosition(Argument* pArgument);
    virtual CbString GetContextList();
    virtual CbString GetEndContextDeclaration();
    virtual CbString GetEndContextImplementation();
    virtual Context* GetFirstContext();
    const CbString GetInterfaceCpp();
    virtual Context* GetNextContext(Context* pContextPos);
    virtual CbString GetShapeText(VerbosityType verbosity = VERBOSITY_OFF);
    CbString GetSignalShapeText(bool arguments = true,
                                bool argumentNames = true, bool scope = true);
    virtual CbString GetStartContextDeclaration();
    virtual CbString GetStartContextImplementation();
    virtual int HasStringInCode(const CbString& string);
    virtual void InitCode();
    bool IsDirectMethod() const;
    virtual int IsFixed() const;
    virtual bool IsNonMacroMethod() const;
    bool IsNonMacroPrivateMethod() const;
    bool IsNonMacroProtectedMethod() const;
    bool IsNonMacroPublicMethod() const;
    int IsNonPure() const;
    bool IsNonStatic() const;
    bool IsNormalConstructor() const;
    bool IsPrivateMethod() const;
    bool IsProtectedMethod() const;
    bool IsPublicMethod() const;
    int IsSimilar(Method* pMethod);
    virtual void NotifyAddMember(Member* pMember);
    virtual void NotifyRemoveMember(Member* pMember);
    virtual int OnAddArgument(bool checkOnly = false);
    virtual int OnAddMethod(bool checkOnly = false);
    virtual int OnAddVirtuals(bool checkOnly = false);
    virtual int OnCopy(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnEditContext(bool checkOnly = false);
    virtual int OnEditExceptionSpecification(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void OnUndoRedoRemoved();
    virtual void ReplaceInCode(const CbString& oldString,
                               const CbString& newString);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    void SetIcon();
    virtual void SetItemText();
    virtual void SetName(const CbString& rName);
    void SetNameAlways(const CbString& rName);
    virtual void SetPhaseDownAndUpwards(Phase phase);
    virtual bool ShownByFilter(TreeViewModel* pTreeViewModel);
    virtual void Update();
    AccessType GetAccess();
    void SetAccess(AccessType access);
    const CbString& GetCallingConvention() const;
    void SetCallingConvention(const CbString& rCallingConvention);
    const CbString& GetCode();
    void SetCode(const CbString& rCode);
    bool GetConst();
    void SetConst(bool val);
    bool GetDeclare() const;
    void SetDeclare(bool declare);
    bool GetDelete() const;
    void SetDelete(bool del);
    bool GetDllExport() const;
    void SetDllExport(bool dllExport);
    bool GetImplement() const;
    void SetImplement(bool implement);
    bool GetInline();
    void SetInline(bool val);
    QDialog* GetOpenDialog();
    void SetOpenDialog(QDialog* pDialog);
    bool GetPure() const;
    void SetPure(bool pure);
    bool GetStatic() const;
    void SetStatic(bool val);
    bool GetUntouched() const;
    void SetUntouched(bool untouched);
    bool GetVirtual() const;
    void SetVirtual(bool val);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _METHOD_H_INLINES
#define _METHOD_H_INLINES

/*@NOTE_19529
Returns the value of member '_callingConvention'.
*/
inline const CbString& Method::GetCallingConvention() const
{//@CODE_19529
    return _callingConvention;
}//@CODE_19529



/*@NOTE_19530
Set the value of member '_callingConvention' to 'rCallingConvention'.
*/
inline void Method::SetCallingConvention(const CbString& rCallingConvention)
{//@CODE_19530
    _callingConvention = rCallingConvention;
}//@CODE_19530



/*@NOTE_19547
Returns the value of member '_declare'.
*/
inline bool Method::GetDeclare() const
{//@CODE_19547
    return _declare;
}//@CODE_19547



/*@NOTE_19548
Set the value of member '_declare' to 'declare'.
*/
inline void Method::SetDeclare(bool declare)
{//@CODE_19548
    _declare = declare;
}//@CODE_19548



/*@NOTE_40937
Returns the value of member '_delete'.
*/
inline bool Method::GetDelete() const
{//@CODE_40937
    return _delete;
}//@CODE_40937



/*@NOTE_40938
Set the value of member '_delete' to 'del'.
*/
inline void Method::SetDelete(bool del)
{//@CODE_40938
    _delete = del;
}//@CODE_40938



/*@NOTE_19525
Returns the value of member '_dllExport'.
*/
inline bool Method::GetDllExport() const
{//@CODE_19525
    return _dllExport;
}//@CODE_19525



/*@NOTE_19526
Set the value of member '_dllExport' to 'dllExport'.
*/
inline void Method::SetDllExport(bool dllExport)
{//@CODE_19526
    _dllExport = dllExport;
}//@CODE_19526



/*@NOTE_19554
Returns the value of member '_implement'.
*/
inline bool Method::GetImplement() const
{//@CODE_19554
    return _implement;
}//@CODE_19554



/*@NOTE_19552
Set the value of member '_implement' to 'implement'.
*/
inline void Method::SetImplement(bool implement)
{//@CODE_19552
    _implement = implement;
}//@CODE_19552



/*@NOTE_16949
Returns the value of member '_untouched'.
*/
inline bool Method::GetUntouched() const
{//@CODE_16949
    return _untouched;
}//@CODE_16949



/*@NOTE_16950
Set the value of member '_untouched' to 'untouched'.
*/
inline void Method::SetUntouched(bool untouched)
{//@CODE_16950
    if (_untouched != untouched)
    {
        _untouched = untouched;
        Update();
    }
}//@CODE_16950



//@START_USER3
//@END_USER3

#endif
#endif
