/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          DataModel.cpp
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DataModel'
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
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
using namespace std;
#ifdef _WIN32
#include <direct.h>   // _chdir (POSIX chdir via CbWinTypes.h on non-Windows)
#endif

#include "ClassBuilderDoc.h"
#include "ParseLogInterface.h"
#include "qt/QtReadSourceDialog.h"
#include "SourceLogInterface.h"
#include "qt/QtDataModelDialog.h"
#include "qt/QtContextDeclarationDialog.h"

enum IncludeType {
    NoneType                            = 0,
    SingleType                          = 1,
    SingleOwnedType                     = SingleType<<1,
    CriticalSingleType                  = SingleType<<2,
    CriticalSingleOwnedType             = SingleType<<3,
    MultiType                           = SingleType<<4,
    MultiOwnedType                      = MultiType<<1,
    CriticalMultiType                   = MultiType<<2,
    CriticalMultiOwnedType              = MultiType<<3,
    UniqueValueTreeType                 = MultiType<<4,
    UniqueValueTreeOwnedType            = UniqueValueTreeType<<1,
    CriticalUniqueValueTreeType         = UniqueValueTreeType<<2,
    CriticalUniqueValueTreeOwnedType    = UniqueValueTreeType<<3,
    ValueTreeType                       = UniqueValueTreeType<<4,
    ValueTreeOwnedType                  = ValueTreeType<<1,
    CriticalValueTreeType               = ValueTreeType<<2,
    CriticalValueTreeOwnedType          = ValueTreeType<<3,
    AvlTreeType                         = ValueTreeType<<4,
    AvlTreeOwnedType                    = AvlTreeType<<1,
    CriticalAvlTreeType                 = AvlTreeType<<2,
    CriticalAvlTreeOwnedType            = AvlTreeType<<3,
    StaticMultiType                     = AvlTreeType<<4,
    StaticMultiOwnedType                = StaticMultiType<<1,
    CriticalStaticMultiType             = StaticMultiType<<2,
    CriticalStaticMultiOwnedType        = StaticMultiType<<3
};
//@END_USER2


// Static members
bool DataModel::_htmlMode = false;


DataModel::DataModel(DataModelDoc* pDataModelDoc) //@INIT_934
    : Gti(pDataModelDoc)
    , _author("")
    , _cppHeader("")
    , _hFile("")
    , _hHeader("")
    , _hUser1("")
    , _hUser2("")
    , _memberPrefix("_")
    , _classPrefix("")
    , _newClassPrefix(_classPrefix)
    , _name("")
    , _note("")
    , _lastSave(CbTime::GetCurrentTime())
    , _stdAfx(0)
    , _serialize(0)
    , _undoRedo(0)
    , __notUsed_rtfTemplate("")
    , __notUsed_rtfOutput("")
    , _publicMethods(true)
    , _protectedMethods(true)
    , _privateMethods(true)
    , _privateMembers(true)
    , _protectedMembers(true)
    , _publicMembers(true)
    , _indentSize(4)
    , __notUsed_schemaFile("")
    , _htmlOutput("")
    , _styleDataModelHeading("Heading 1")
    , _styleGroupHeading("Heading 2")
    , _styleClassHeading("Heading 3")
    , _styleItemsHeading("Heading 4")
    , _styleExplanation("Normal")
    , _styleItem("List")
    , _styleItemExplanation("BodyTextIndent")
    , _styleFigureText("Caption")
    , _relationMethods(true)
    , _modifiers(false)
    , _getSetMethods(true)
    , _classBuilderMethods(true)
    , _phaseSupport(false)
    , _templateClassHeaderOnly(true)
    , _includeContextDeclarations(true)
    , __notUsed_lastSaveRtfDocumentation(CbTime::GetCurrentTime())
    , _classOverview(true)
    , _relationOverview(true)
    , _includeSequenceDiagramMessages(true)
    , _includeSequenceDiagramObjects(true)
    , _showDllExport(1)
    , __notUsed_rtfDiagramFormat(1)
    , _crlf(true)
{//@CODE_934
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
    CbString copyright;
    copyright.Format("* Copyright %d, XXXXX", CbTime::GetCurrentTime().GetYear());
    _hHeader += "/******************************************************************************\\" NL;
    _hHeader += "*" NL;
    _hHeader += "* File:          @INSERT_FILENAME" NL;
    _hHeader += "* Creation date: @INSERT_DATE" NL;
    _hHeader += "* Author:        ClassBuilder" NL;
    _hHeader += "*                XXXX" NL;
    _hHeader += "* Purpose:       Declaration of class '@INSERT_CLASSNAME'" NL;
    _hHeader += "*" NL;
    _hHeader += "* Modifications: @INSERT_MODIFICATIONS(* )" NL;
    _hHeader += "*" NL;
    _hHeader += copyright + NL;
    _hHeader += "*" NL;
    _hHeader += "\\******************************************************************************/";

    _cppHeader += "/******************************************************************************\\" NL;
    _cppHeader += "*" NL;
    _cppHeader += "* File:          @INSERT_FILENAME" NL;
    _cppHeader += "* Creation date: @INSERT_DATE" NL;
    _cppHeader += "* Author:        ClassBuilder" NL;
    _cppHeader += "*                XXXX" NL;
    _cppHeader += "* Purpose:       Method implementations of class '@INSERT_CLASSNAME'" NL;
    _cppHeader += "*" NL;
    _cppHeader += "* Modifications: @INSERT_MODIFICATIONS(* )" NL;
    _cppHeader += "*" NL;
    _cppHeader += copyright + NL;
    _cppHeader += "*" NL;
    _cppHeader += "\\******************************************************************************/";
    
    SetPhase(Analysis_Phase);
}//@CODE_934


/*@NOTE_321
Constructor needed for serialization, not meant to use for other purposes!
*/
DataModel::DataModel() //@INIT_321
    : Gti()
    , _author("")
    , _classPrefix("")
    , _indentSize(4)
    , __notUsed_schemaFile("")
    , _htmlOutput("")
    , _styleFigureText("Caption")
    , _undoRedo(0)
    , _namespace("")
    , _relationMethods(true)
    , _modifiers(false)
    , _getSetMethods(true)
    , _classBuilderMethods(true)
    , _phaseSupport(false)
    , _templateClassHeaderOnly(true)
    , _includeContextDeclarations(true)
    , __notUsed_lastSaveRtfDocumentation(CbTime::GetCurrentTime())
    , _classOverview(true)
    , _relationOverview(true)
    , _includeSequenceDiagramMessages(true)
    , _includeSequenceDiagramObjects(true)
    , _showDllExport(1)
    , __notUsed_rtfDiagramFormat(1)
    , _crlf(true)
{//@CODE_321
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Analysis_Phase);
}//@CODE_321


/*@NOTE_319
Destructor method
*/
DataModel::~DataModel()
{//@CODE_319
    DestructorInclude();

    // Put in your own code
}//@CODE_319


void DataModel::Add()
{//@CODE_947
    if (!GetAdded())
    {
        SetItemText(_name);
        SetIcon(ICON_FILE);

        Gti::Add();

        Gti::ChildIterator iClassDiagram(this, &Gti::IsClassDiagram);
        while (++iClassDiagram)
            iClassDiagram->Add();

        Gti::ChildIterator iSequenceDiagram(this, &Gti::IsSequenceDiagram);
        while (++iSequenceDiagram)
            iSequenceDiagram->Add();

        MetaGroupIterator iMetaGroup(this);
        while (++iMetaGroup)
            iMetaGroup->Add();
        
        ClassGroupIterator iClassGroup(this);
        while (++iClassGroup)
            iClassGroup->Add();

        ClassIterator iClass(this);
        while (++iClass)
        {
            if (!iClass->GetClassGroup())
            {
                iClass->Add();
            }
        }
    }
}//@CODE_947


/*@NOTE_23037
Add the serialize on an existing project.
*/
void DataModel::AddSerialize(CbString className)
{//@CODE_23037
    if (!GetSerialize())
    {
        SetStdAfx(1);

        if (!GetDataModelDoc()->FindBaseClass("CbObject"))
        {
            ExternClass* pExternClass = new ExternClass(GetDataModelDoc());
            pExternClass->SetName("CbObject");
            pExternClass->Add();
        }

        SetSerialize(1);
        InitSerialize(className);
        
        BaseClass::MethodIterator iMethod(GetDocument(), &Method::IsConstructor);
        if (++iMethod)
        {
            iMethod->SetAccess(PUBLIC);
            iMethod->SetNote("Constructor needed for serialization, can also be used for default construction.");
        }

        int i = 0;
        ClassIterator iClass(this);
        while (++iClass)
        {
            if (!iClass->GetClassGroup())
                iClass->SetOrder(i++);
        }

        GetDocument()->Add();
        GetDocumentObject()->Add();

        GetDataModelDoc()->DeleteAllUndoBase();
        GetDataModelDoc()->DeleteAllRedoBase();
    }
}//@CODE_23037


bool DataModel::CheckUpdates()
{//@CODE_936
    _chdir(GetDataModelDoc()->GetPath());

    struct _stat buf;
    bool result = false;
    bool sourceUpdated = false;

    if (_stat(GetHFile(), &buf) == 0)
    {
        if (_lastSave < CbTime(buf.st_mtime))
            sourceUpdated = true;
    }
    DataModel::ClassIterator iClass(this);
    while (sourceUpdated == false && ++iClass)
    {
        if (_stat(iClass->GetHFile(), &buf) == 0)
        {
            if (_lastSave < CbTime(buf.st_mtime))
                sourceUpdated = true;
        }
        if (_stat(iClass->GetCppFile(), &buf) == 0)
        {
            if (_lastSave < CbTime(buf.st_mtime))
                sourceUpdated = true;
        }
    }
    if (sourceUpdated)
    {
        CbString str;
        str.Format("Source files have been modified outside of ClassBuilder for project '%s'. "
			"Do you want to reload them?", GetDataModelDoc()->GetTitle().c_str());
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDYES)
        {
            CbTime lastSave = _lastSave;

            void* ownerHwnd = Cb_OwnerHwnd();
            Qt_ShowReadSourceDialog(this, ownerHwnd);
            GetDataModelDoc()->MarkLastUndo();
            result = (lastSave == _lastSave);
        }
    }
    return result;
}//@CODE_936


CbString DataModel::ConvertToHtmlStringIfNeeded(CbString str)
{//@CODE_16946
    if (DataModel::GetHtmlMode())
    {
        int index = str.Find("<");
        while (index != -1)
        {
            if (index+1 == str.GetLength())
            {
                str = str.Left(index) + "&lt;";
            }
            else
            {
                str = str.Left(index) + "&lt;" + str.Mid(index+1);
            }
            
            index = str.Find("<");
        }
        
        index = str.Find(">");
        while (index != -1)
        {
            if (index+1 == str.GetLength())
            {
                str = str.Left(index) + "&gt;";
            }
            else
            {
                str = str.Left(index) + "&gt;" + str.Mid(index+1);
            }
            
            index = str.Find(">");
        }
    }
    
    return str;
}//@CODE_16946


Class* DataModel::FindClass(const CbString& rName)
{//@CODE_1280
    ClassIterator iClass(this);
    while (++iClass)
    {
        if (rName == iClass->GetName())
        {
            return iClass;
        }
    }

    return 0;
}//@CODE_1280


CbString DataModel::GetHFileWithoutPath()
{//@CODE_11056
    CbString hFile = _hFile;

    int index = hFile.ReverseFind('/');
    if (index != -1)
    {
        hFile = hFile.Mid(index+1);
    }
    
    index = hFile.ReverseFind('\\');
    if (index != -1)
    {
        hFile = hFile.Mid(index+1);
    }
    
    return hFile;
}//@CODE_11056


Gti* DataModel::GetNext(Gti* pGti)
{//@CODE_35371
    Gti* pNextGti = Gti::GetNext(pGti);

    if (!pNextGti)
    {
        pNextGti = GetFirstMetaGroup();
    }
        
    if (!pNextGti)
    {
        pNextGti = GetDataModelDoc()->GetActors();
    }

    return pNextGti;
}//@CODE_35371


/*@NOTE_4882
New document, with new datamodel, init it.
*/
void DataModel::Init(CbString className)
{//@CODE_4882
    ExternClasses* pExternClasses = new ExternClasses(GetDataModelDoc());
    OtherTypes* pOtherTypes       = new OtherTypes(GetDataModelDoc());
    Actors* pActors               = new Actors(GetDataModelDoc());
    
    (new OtherType(GetDataModelDoc()))->SetName("");
    (new OtherType(GetDataModelDoc()))->SetName("...");
    (new OtherType(GetDataModelDoc()))->SetName("void");
    (new OtherType(GetDataModelDoc()))->SetName("bool");
    (new OtherType(GetDataModelDoc()))->SetName("char");
    (new OtherType(GetDataModelDoc()))->SetName("unsigned char");
    (new OtherType(GetDataModelDoc()))->SetName("double");
    (new OtherType(GetDataModelDoc()))->SetName("float");
    (new OtherType(GetDataModelDoc()))->SetName("int");
    (new OtherType(GetDataModelDoc()))->SetName("unsigned int");
    (new OtherType(GetDataModelDoc()))->SetName("long");
    (new OtherType(GetDataModelDoc()))->SetName("unsigned long");
    (new OtherType(GetDataModelDoc()))->SetName("short");
    (new OtherType(GetDataModelDoc()))->SetName("unsigned short");
    
    if (GetSerialize())
    {
        InitSerialize(className);
        
        if (GetUndoRedo())
        {
            InitUndoRedo();
        }
    }
    
    Add();
    pExternClasses->Add();
    pOtherTypes->Add();
    pActors->Add();
}//@CODE_4882


void DataModel::InitDocumentObjectUndoRedo()
{//@CODE_4899
    Method* pMethod;
    Argument* pArgument;
    CbString code;

    BaseClass::MethodIterator iMethod(GetDocumentObject(), &Method::IsConstructor);
    while (++iMethod)
    {
        if (!iMethod->IsSerializeConstructor())
        {
            code = iMethod->GetCode();
            code += NL"    (void)new UndoNew(this);";
            iMethod->SetCode(code);
        }
    }

    pMethod = new Method(GetDocumentObject(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Delete");
    pMethod->SetAccess(PUBLIC);
    code = "    (void)new UndoDelete(this);";
    pMethod->SetCode(code);
    pMethod->SetNote(
        "Use this method instead of calling delete. This method will make the" NL
        "appropriate actions to put the object on the undo stack, so the delete can be" NL
        "undone. It will also take care of the associations and the aggregations.");

    pMethod = new Method(GetDocumentObject(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("SaveState");
    pMethod->SetAccess(PUBLIC);
    code = 
        "    if (!Get" + GetDocument()->GetBaseName() + "()->GetIsUndoing() && " NL
        "        !Get" + GetDocument()->GetBaseName() + "()->GetIsRedoing()) " NL
        "    {" NL
        "        if (!always)" NL
        "        {" NL
        "            " + GetDocument()->GetName() + "::UndoBaseIterator iUndoBase(Get" + GetDocument()->GetBaseName() + "());" NL
        "            while (--iUndoBase && !iUndoBase->GetLast())" NL
        "            {" NL
        "                // We are already on stack" NL
        "                if (iUndoBase->Get" + GetDocumentObject()->GetBaseName() + "() == this)" NL
        "                    return;" NL
        "            }" NL
        "        }" NL
        "" NL
        "        (void)new UndoChange(this);" NL
        "    }" NL;
    pMethod->SetCode(code);
    pMethod->SetNote(
        "Save the state of the current object, it is checked if it isn't already on" NL
        "stack in the last open undo session. If 'always' is non zero, then it is put" NL
        "on stack unconditionally");
    pArgument = new Argument(pMethod, GetDataModelDoc()->FindType("int"));
    pArgument->SetName("always");
    pArgument->SetDefault("0");

    pMethod = new Method(GetDocumentObject(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("OnUndoRedoAdded");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    pMethod->SetNote(
        "This method is a hook to update the view in case the object appears because of" NL
        "an Undo/Redo. It is called after the object is added again into the data structure" NL
        "This method is empty, so overwrite this virtual method at derived classes if needed.");

    pMethod = new Method(GetDocumentObject(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("OnUndoRedoRemoving");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    pMethod->SetNote(
        "This method is a hook to update the view in case the object disappears because of" NL
        "an Undo/Redo. It is called before the object is removed from the data structure" NL
        "This method is empty, so overwrite this virtual method at derived classes if needed.");

    pMethod = new Method(GetDocumentObject(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("OnUndoRedoChanging");
    pMethod->SetVirtual(1);
    pMethod->SetAccess(PUBLIC);
    code = "    OnUndoRedoRemoving();" NL;
    pMethod->SetCode(code);
    pMethod->SetNote(
        "This method is a hook to update the view in case the object changes state because" NL
        "of an Undo/Redo. It is called before the object changes state. This method calls" NL
        "OnUndoRedoRemoving(), so overwrite this virtual method at derived classes if needed," NL
        "or change the default behaviour.");


    pMethod = new Method(GetDocumentObject(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("OnUndoRedoChanged");
    pMethod->SetVirtual(1);
    pMethod->SetAccess(PUBLIC);
    code = "    OnUndoRedoAdded();" NL;
    pMethod->SetCode(code);
    pMethod->SetNote(
        "This method is a hook to update the view in case the object changes state because" NL
        "of an Undo/Redo. It is called after the object changed state. This method calls" NL
        "OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed," NL
        "or change the default behaviour.");

}//@CODE_4899


void DataModel::InitDocumentUndoRedo(Class* pUndoBase)
{//@CODE_4900
    Method* pMethod;
    Member* pMember;
    Argument* pArgument;
    CbString code;

    pMethod = new Method(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMethod->SetName("Undo");
    pMethod->SetAccess(PUBLIC);
    code = 
        "    int result = 0;" NL
        "    _isUndoing = 1;" NL
        "    if (pUndoBase)" NL
        "    {" NL
        "        UndoBaseIterator iUndoBase(this);" NL
        "        while (--iUndoBase && iUndoBase.Get() != pUndoBase)" NL
        "        {" NL
        "            iUndoBase->Restore();" NL
        "            result++;" NL
        "        }" NL
        "    }" NL
        "    else" NL
        "    {" NL
        "        UndoBaseIterator iUndoBase(this);" NL
        "        if (--iUndoBase)" NL
        "        {" NL
        "            iUndoBase->Restore();" NL
        "            result++;" NL
        "            while (--iUndoBase && !iUndoBase->GetLast())" NL
        "            {" NL
        "                iUndoBase->Restore();" NL
        "                result++;" NL
        "            }" NL
        "        }" NL
        "    }" NL
        "    _isUndoing = 0;"
        "" NL
        "    if (result)" NL
        "    {" NL
        "        _currentUndoCount--;" NL
        "    }" NL
        "" NL
        "    return result;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Undo the last recorded change, returns the number of objects undoed." NL
                     "if 'pUndoBase' is non-zero, then it is undo until 'pUndoBase'");
    pArgument = new Argument(pMethod, pUndoBase);
    pArgument->SetName("p" + pUndoBase->GetBaseName());
    pArgument->SetPointer(1);
    pArgument->SetDefault("0");

    pMethod = new Method(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMethod->SetName("Redo");
    pMethod->SetAccess(PUBLIC);
    code = 
        "    int result = 0;" NL
        "    _isRedoing = 1;" NL
        "    RedoBaseIterator iRedoBase(this);" NL
        "    while (--iRedoBase && !iRedoBase->GetLast())" NL
        "    {" NL
        "        iRedoBase->Restore();" NL
        "        result++;" NL
        "    }" NL
        "" NL
        "    if (iRedoBase)" NL
        "    {" NL
        "        iRedoBase->Restore();" NL
        "        result++;" NL
        "    }" NL
        "" NL
        "    if (result)" NL
        "    {" NL
        "        _currentUndoCount++;" NL
        "    }" NL
        "" NL
        "    _isRedoing = 0;" NL
        "    return result;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Redo the last Undo, returns the number of objects redoed.");

    pMethod = new Method(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMethod->SetName("RollBack");
    pMethod->SetAccess(PUBLIC);
    code = 
        "    // Remember the current situtation of the redo stack" NL
        "    RedoBase* pLastRedoBase = GetLastRedoBase();" NL
        "" NL
        "    // Undo the actions" NL
        "    int result = 0;" NL
        "    _isUndoing = 1;" NL
        "    if (pUndoBase)" NL
        "    {" NL
        "        UndoBaseIterator iUndoBase(this);" NL
        "        while (--iUndoBase && iUndoBase.Get() != pUndoBase)" NL
        "        {" NL
        "            iUndoBase->Restore();" NL
        "            result++;" NL
        "        }" NL
        "    }" NL
        "    else" NL
        "    {" NL
        "        UndoBaseIterator iUndoBase(this);" NL
        "        if (--iUndoBase)" NL
        "        {" NL
        "            // If this session was already counted, decrement counter" NL
        "            if (iUndoBase->GetLast())" NL
        "                _currentUndoCount--;" NL
        "" NL
        "            iUndoBase->Restore();" NL
        "            result++;" NL
        "            while (--iUndoBase && !iUndoBase->GetLast())" NL
        "            {" NL
        "                iUndoBase->Restore();" NL
        "                result++;" NL
        "            }" NL
        "        }" NL
        "    }" NL
        "   _isUndoing = 0;" NL
        "" NL
        "    // Bring redo stack back in previous position" NL
        "    RedoBaseIterator iRedoBase(this);" NL
        "    while (--iRedoBase && iRedoBase.Get() != pLastRedoBase)" NL
        "    {" NL
        "        delete iRedoBase;" NL
        "    }" NL
        "" NL
        "    return result;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Same as Undo, but puts nothing on the Redo stack.");
    pArgument = new Argument(pMethod, pUndoBase);
    pArgument->SetName("p" + pUndoBase->GetBaseName());
    pArgument->SetPointer(1);
    pArgument->SetDefault("0");

    pMethod = new Method(GetDocument(), GetDataModelDoc()->FindType("void"));
    pMethod->SetName("CleanRedo");
    pMethod->SetAccess(PUBLIC);
    code = 
        "    if (!_isRedoing)" NL
        "    {" NL
        "        RedoBaseIterator iRedoBase(this);" NL
        "        while (++iRedoBase)" NL
        "        {" NL
        "            delete iRedoBase;" NL
        "        }" NL
        "    }" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Clean the redo stack, unless we are undoing.");

    pMethod = new Method(GetDocument(), pUndoBase);
    pMethod->SetPointer(1);
    pMethod->SetName("MarkLastUndo");
    pMethod->SetAccess(PUBLIC);
    code = 
        "    if (GetLastUndoBase() && !GetLastUndoBase()->GetLast())" NL
        "    {" NL
        "        GetLastUndoBase()->SetLast(1);" NL
        "        _currentUndoCount++;" NL
        "" NL
        "        // Make sure we don't have to many things on stack." NL
        "        UndoBaseIterator iUndoBase(this);" NL
        "        while (++iUndoBase && _currentUndoCount > _maxUndoCount)" NL
        "        {" NL
        "            if (iUndoBase->GetLast())" NL
        "            {" NL
        "                _currentUndoCount--;" NL
        "            }" NL
        "            delete iUndoBase;" NL
        "        }" NL
        "    }" NL
        "" NL
        "    return GetLastUndoBase();" NL;
    pMethod->SetCode(code);
    pMethod->SetNote(
        "Group the undo's on the undo stack by marking the last on stack as last, any new object" NL
        "placed on stack will be part of a new undoable action. An undo or a redo will always" NL
        "undo or redo the whole group.");

    pMember = new Member(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMember->SetName("currentUndoCount");
    pMember->SetNote("The actual number of undoable actions recorded on the undo stack");
    pMember->SetInitialization("0");
    pMember->SetSerialize(0);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pMember = new Member(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMember->SetName("maxUndoCount");
    pMember->SetNote("The maximum number of undoable recorded actions on stack.");
    pMember->SetInitialization("10");
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);
    pMethod = new SetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pMember = new Member(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMember->SetName("isUndoing");
    pMember->SetNote(
        "Indicates if we are undoing at the moment, if that is the case an added undo" NL
        "may not clear the redo stack.");
    pMember->SetInitialization("0");
    pMember->SetSerialize(0);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pMember = new Member(GetDocument(), GetDataModelDoc()->FindType("int"));
    pMember->SetName("isRedoing");
    pMember->SetNote(
        "Indicates if we are redoing at the moment, if that is the case an added undo" NL
        "may not clear the redo stack.");
    pMember->SetInitialization("0");
    pMember->SetSerialize(0);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    BaseClass::MethodIterator iMethod(GetDocument(), &Method::IsConstructor);
    if (++iMethod)
    {
        Constructor* pConstructor = (Constructor*)iMethod.Get();
        CbString init =
            "    : CbObject()" NL
            "    , " + GetDocument()->GetMemberPrefix() + "currentUndoCount(0)" NL
            "    , " + GetDocument()->GetMemberPrefix() + "maxUndoCount(10)" NL
            "    , " + GetDocument()->GetMemberPrefix() + "isUndoing(0)" NL;
            "    , " + GetDocument()->GetMemberPrefix() + "isRedoing(0)" NL;
        pConstructor->SetInit(init);
    }

	//PR285 Assume that possibly another member prefix then '_' is used.
	CbString memberPrefix = GetDocument()->GetMemberPrefix();
	if (memberPrefix != "_")
	{
		GetDocument()->SetMemberPrefix("_");
		GetDocument()->SetMemberPrefix(memberPrefix);
	}
}//@CODE_4900


void DataModel::InitSerialize(CbString className)
{//@CODE_4884
    BaseClass* pCbObject = GetDataModelDoc()->FindBaseClass("CbObject");
    if (!pCbObject)
    {
        pCbObject = new ExternClass(GetDataModelDoc());
        pCbObject->SetName("CbObject");
    }
        
    Class* pDocumentObject = new Class(this);
    AddDocumentObject(pDocumentObject);
    pDocumentObject->SetName(className + "Object");
    // Repair some argument names, since the type name wasn't known yet.
    BaseClass::MethodIterator iMethod(pDocumentObject);
    while (++iMethod)
    {
        if (iMethod->IsRestoreReferencesMethod() || 
            iMethod->IsSaveReferencesMethod())
        {
            iMethod->GetFirstArgument()->SetName("p" + pDocumentObject->GetName());
        }
    }
    pDocumentObject->SetCppFile(pDocumentObject->GetBaseName() + ".cpp");
    pDocumentObject->SetHFile(pDocumentObject->GetBaseName() + ".h");
    
    // Make document object the first in the list, to be sure to get the same order 
    // if it is done latter, do not known if it is acually needed.
    MoveClassFirst(pDocumentObject);
    
    Class* pDocument = new Class(this);
    AddDocument(pDocument);
    pDocument->SetName(className);
    pDocument->SetCppFile(pDocument->GetBaseName() + ".cpp");
    pDocument->SetHFile(pDocument->GetBaseName() + ".h");
    
    // Make document the first in the list, must create documentObject first
    // for referencing purposes
    MoveClassFirst(pDocument);
    
    (void)new Inherit(pDocument, pCbObject);
    (void)new Inherit(pDocumentObject, pCbObject);
    (void)new Relation(pDocument, pDocumentObject,
        pDocument->GetBaseName(), pDocumentObject->GetBaseName(),
        0, 1, 0, 1, 0);
    pDocumentObject->GetConstructorIncludeMethod()->UpdateArguments();
    (new Constructor(pDocumentObject))->CreateArguments();

    // Static flag observed by the codegen-emitted Serialize body to skip
    // SERIALIZE_ALL_OBJECTS when only the document's own scalar members
    // need to be (de)serialized. Not serialized to disk; transient flag.
    // Required by the generated Serialize body — protected from deletion.
    Member* pMembersOnly = new Member(pDocument, GetDataModelDoc()->FindType("bool"));
    pMembersOnly->SetName("membersOnly");
    pMembersOnly->SetStatic(1);
    pMembersOnly->SetSerialize(0);
    pMembersOnly->SetInitialization("false");
    pMembersOnly->SetAccess(PROTECTED);
    pMembersOnly->SetNote(
        "Static flag observed by the codegen-emitted Serialize body."        NL
        "When true, Serialize skips its trailing SERIALIZE_ALL_OBJECTS"      NL
        "call so only the document's own scalar members are written/"        NL
        "read. Used by SerializeMembersOnly (and any other code that"        NL
        "needs to round-trip just the document's own state, e.g. doc-"       NL
        "level undo/redo via memory-stream snapshots). Not serialized to"    NL
        "disk. Required by the generated Serialize body and protected"       NL
        "from deletion or rename.");

    // SerializeMembersOnly — convenience method that snapshots only the
    // document's own scalar members (without recursing into the owned
    // object graph). Bodied with the flag-dance: set, call, reset.
    Method* pSerializeMembersOnly =
        new Method(pDocument, GetDataModelDoc()->FindType("void"));
    pSerializeMembersOnly->SetName("SerializeMembersOnly");
    pSerializeMembersOnly->SetAccess(PUBLIC);
    pSerializeMembersOnly->SetVirtual(1);
    Argument* pSMArchive =
        new Argument(pSerializeMembersOnly, GetDataModelDoc()->FindType("CbObject"));
    pSMArchive->SetReference(1);
    pSMArchive->SetName("archive");
    pSerializeMembersOnly->SetCode(
        "    _membersOnly = true;"  NL
        "    Serialize(archive);"   NL
        "    _membersOnly = false;" NL);
    pSerializeMembersOnly->SetNote(
        "Memory-stream snapshot of the document's own scalar members,"   NL
        "without serializing the document's owned object graph."         NL
        ""                                                               NL
        "Useful for compact undo/redo records of doc-level property"     NL
        "changes (settings, colors, project-wide options) where the"     NL
        "object graph hasn't changed and you don't want to clone every"  NL
        "object the doc owns."                                           NL
        ""                                                               NL
        "Implementation: sets the static `_membersOnly` flag, calls"     NL
        "Serialize (whose generated body checks the flag and skips"      NL
        "SERIALIZE_ALL_OBJECTS), then clears the flag. The body stays"   NL
        "in lockstep with Serialize automatically — adding a new doc"    NL
        "member just needs the regular Serialize regeneration."          NL
        ""                                                               NL
        "Free to delete if your project has no doc-level snapshot use"   NL
        "case. The static `_membersOnly` flag must stay regardless,"     NL
        "because the generated Serialize body references it.");
}//@CODE_4884


void DataModel::InitUndoRedo()
{//@CODE_4885
    Class* pUndoBase;
    Class* pRedoBase;
    InitUndoRedoBase(pUndoBase, pRedoBase);
    
    InitDocumentUndoRedo(pUndoBase);
    InitDocumentObjectUndoRedo();
    
    Class* pUndoNew;
    Class* pRedoNew;
    InitUndoRedoNew(pUndoBase, pRedoBase, pUndoNew, pRedoNew);
    
    Class* pUndoDelete;
    Class* pRedoDelete;
    InitUndoRedoDelete(pUndoBase, pRedoBase, pUndoDelete, pRedoDelete);
    
    Class* pUndoSubDelete;
    InitUndoSubDelete(pUndoBase, pUndoSubDelete);
    
    Class* pUndoChange;
    Class* pRedoChange;
    InitUndoRedoChange(pUndoBase, pRedoBase, pUndoChange, pRedoChange);
    
    Class* pUndoSubChange;
    InitUndoSubChange(pUndoBase, pUndoSubChange);

    Relation* pUndoStack = new Relation(GetDocument(), pUndoBase, 
        GetDocument()->GetBaseName(), pUndoBase->GetBaseName(), 0, 1, 0, 1, 0);
    pUndoStack->SetNote("The undo stack");
    pUndoBase->GetConstructorIncludeMethod()->UpdateArguments();
    // An argument is added to the constructor automatically, remove it.
    BaseClass::MethodIterator iMethodUndoBase(pUndoBase, &Method::IsConstructor);
    while (++iMethodUndoBase)
    {
        if (!iMethodUndoBase->IsSerializeConstructor() && 
            iMethodUndoBase->GetLastArgument())
        {
            iMethodUndoBase->GetLastArgument()->Delete();
        }
    }
    
    Relation* pRedoStack = new Relation(GetDocument(), pRedoBase, 
        GetDocument()->GetBaseName(), pRedoBase->GetBaseName(), 0, 1, 0, 1, 0);
    pRedoStack->SetNote("The redo stack");
    pRedoBase->GetConstructorIncludeMethod()->UpdateArguments();
    // An argument is added to the constructor automatically, remove it.
    BaseClass::MethodIterator iMethodRedoBase(pRedoBase, &Method::IsConstructor);
    while (++iMethodRedoBase)
    {
        if (!iMethodRedoBase->IsSerializeConstructor() && 
            iMethodRedoBase->GetLastArgument())
        {
            iMethodRedoBase->GetLastArgument()->Delete();
        }
    }
    
    ClassGroup* pClassGroup = new ClassGroup(this);
    pClassGroup->SetName("Undo/Redo classes");
    pClassGroup->SetNote("The classes needed for the undo/redo functionality.");
    
    pUndoBase->SaveState(1);
    pClassGroup->AddClassLast(pUndoBase);
    pUndoNew->SaveState(1);
    pClassGroup->AddClassLast(pUndoNew);
    pUndoDelete->SaveState(1);
    pClassGroup->AddClassLast(pUndoDelete);
    pUndoSubDelete->SaveState(1);
    pClassGroup->AddClassLast(pUndoSubDelete);
    pUndoChange->SaveState(1);
    pClassGroup->AddClassLast(pUndoChange);
    pUndoSubChange->SaveState(1);
    pClassGroup->AddClassLast(pUndoSubChange);

    pRedoBase->SaveState(1);
    pClassGroup->AddClassLast(pRedoBase);
    pRedoNew->SaveState(1);
    pClassGroup->AddClassLast(pRedoNew);
    pRedoDelete->SaveState(1);
    pClassGroup->AddClassLast(pRedoDelete);
    pRedoChange->SaveState(1);
    pClassGroup->AddClassLast(pRedoChange);
}//@CODE_4885


void DataModel::InitUndoRedoBase(Class*& pUndoBase, Class*& pRedoBase)
{//@CODE_4890
    Member* pMember;
    Method* pMethod;
    Constructor* pConstructor;
    Argument* pArgument;
    CbString code;
    CbString init;

    // Undobase Init
    pUndoBase = new Class(this);
    pUndoBase->SetName("UndoBase");
    pUndoBase->SetCppFile(pUndoBase->GetName() + ".cpp");
    pUndoBase->SetHFile(pUndoBase->GetName() + ".h");
    pUndoBase->SetNote("All different kind of undoable mutations to the datastructure are derived from this class.");
    pUndoBase->SetSerialize(0);

    pMember = new Member(pUndoBase, GetDocumentObject());
    pMember->SetName("p" + GetDocumentObject()->GetBaseName());
    pMember->SetNote("The involved document object.");
    pMember->SetPointer(1);
    pMember->SetAccess(PROTECTED);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pMember = new Member(pUndoBase, GetDataModelDoc()->FindType("int"));
    pMember->SetName("last");
    pMember->SetNote(
        "Indicates the last undo of a sequence, so a undo action starts here and should" NL
        "stop before the Undo object with a non zero value of last, or if the undo stack" NL
        "is exhausted.");
    pMember->SetInitialization("0");
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);
    pMethod = new SetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pConstructor = new Constructor(pUndoBase);
    pArgument = new Argument(pConstructor, GetDocumentObject());
    pArgument->SetName("p" + GetDocumentObject()->GetBaseName());
    pArgument->SetPointer(1);
    code =
        "    ConstructorInclude(" + pArgument->GetName() + "->Get" + GetDocument()->GetBaseName() + "());" NL
        NL
        "    // If we add something new to the undo stack, then we have" NL
        "    // to clean the redo stack" NL
        "    Get" + GetDocument()->GetBaseName() +"()->CleanRedo();" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote("Creates an undo object, with as argument the object involved in the undo.");

    pMethod = new Method(pUndoBase, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    pMethod->SetPure(1);
    pMethod->SetNote("Restore the situation, by undoing recorded action.");
    
    // RedoBase init
    pRedoBase = new Class(this);
    pRedoBase->SetName("RedoBase");
    pRedoBase->SetCppFile(pRedoBase->GetName() + ".cpp");
    pRedoBase->SetHFile(pRedoBase->GetName() + ".h");
    pRedoBase->SetNote("All different kind of redoable mutations to the datastructure are derived from this class.");
    pRedoBase->SetSerialize(0);

    pConstructor = new Constructor(pRedoBase);
    pArgument = new Argument(pConstructor, pUndoBase);
    pArgument->SetName("p" + pUndoBase->GetBaseName());
    pArgument->SetPointer(1);
    code =
        "    ConstructorInclude(" + pArgument->GetName() + "->Get" + GetDocument()->GetBaseName() + "());" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote("All different kind of re-doable mutations to the data structure are derived from this class.");

    pMember = new Member(pRedoBase, GetDocumentObject());
    pMember->SetName("p" + GetDocumentObject()->GetBaseName());
    pMember->SetNote("The involved document object.");
    pMember->SetPointer(1);
    pMember->SetAccess(PROTECTED);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);
    pMember->SetInitialization(pArgument->GetName() + "->" + pMethod->GetName() + "()");

    pMember = new Member(pRedoBase, GetDataModelDoc()->FindType("int"));
    pMember->SetName("last");
    pMember->SetNote(
        "Indicates the last undo of a sequence, so a redo action should stop here and" NL
        "process this Redo object as last one.");
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);
    pMember->SetInitialization(pArgument->GetName() + "->" + pMethod->GetName() + "()");

    pMethod = new Method(pRedoBase, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    pMethod->SetPure(1);
    pMethod->SetNote("Restore the situation, by redoing recorded action.");

    // Now the redo type is know, so the following constructor for the UndoBase can
    // be made
    pConstructor = new Constructor(pUndoBase);
    pArgument = new Argument(pConstructor, pRedoBase);
    pArgument->SetName("p" + pRedoBase->GetBaseName());
    pArgument->SetPointer(1);
    code = "    ConstructorInclude(" + pArgument->GetName() + "->Get" + GetDocument()->GetBaseName() + "());" NL;
    pConstructor->SetCode(code);
    init = 
        "    : " + pUndoBase->GetFirstMember()->GetPrefixedName() + "(" + pArgument->GetName() + "->" + pUndoBase->GetFirstMember()->GetGetMemberMethod()->GetName() + "())" NL
        "    , " + pUndoBase->GetLastMember()->GetPrefixedName()  + "(" + pArgument->GetName() + "->" + pUndoBase->GetLastMember()->GetGetMemberMethod()->GetName()  + "())" NL;
    pConstructor->SetInit(init);
    pConstructor->SetNote(
        "Constructor needed if a redo is performed and the corresponding undo has to be" NL
        "popped on stack.");
}//@CODE_4890


void DataModel::InitUndoRedoChange(Class* pUndoBase, Class* pRedoBase,
                                   Class*& pUndoChange, Class*& pRedoChange)
{//@CODE_4909
    Inherit* pInherit;
    Method* pMethod;
    Member* pMember;
    Constructor* pConstructor;
    Argument* pArgument;
    CbString code;
    CbString init;

    pUndoChange = new Class(this);
    pUndoChange->SetName("UndoChange");
    pUndoChange->SetCppFile(pUndoChange->GetName() + ".cpp");
    pUndoChange->SetHFile(pUndoChange->GetName() + ".h");
    pUndoChange->SetNote(
        "An object state has been changed, if Restore is called this change is undone." NL
        "The member variable '_pX' holds a pointer to the changed object." NL
        "Member variable '_pXSave' points to an unreferenced copy that has the" NL
        "previous state of '_pX'.");
    pUndoChange->SetSerialize(0);

    pInherit = new Inherit(pUndoChange, pUndoBase);
    pInherit->SetNote("Basic properties like being on the undo stack are inherited.");
    
    pMember = new Member(pUndoChange, GetDocumentObject());
    pMember->SetName("p" + GetDocumentObject()->GetBaseName() + "Save");
    pMember->SetNote("Points to an unreferenced copy that has the previous state of involved object.");
    pMember->SetPointer(1);
    pMember->SetInitialization("NULL");
    pMember->SetAccess(PRIVATE);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pConstructor = new Constructor(pUndoChange);
    pArgument = new Argument(pConstructor, GetDocumentObject());
    pArgument->SetName("p" + GetDocumentObject()->GetBaseName());
    pArgument->SetPointer(1);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Snapshot the object's state into a sibling instance via CbArchive" NL
        "    // on a memory stream. The polymorphic operator instantiates the" NL
        "    // right subclass via CbClassRegistration." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        store << " + pUndoBase->GetFirstMember()->GetPrefixedName() + ";" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        CbObject* tmp = NULL;" NL
        "        load >> tmp;" NL
        "        " + pUndoChange->GetFirstMember()->GetPrefixedName() + " = static_cast<" + GetDocumentObject()->GetName() + "*>(tmp);" NL
        "    }" NL
        "" NL
        "    // Save the state of the relations" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->SaveReferences(" + pUndoChange->GetFirstMember()->GetPrefixedName() + ");" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor for the making an undo object for the case an object is changed in" NL
        "the document");


    pMethod = new Method(pUndoChange, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetNote("Undo the change.");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Save the current state first" NL
        "    (void)new RedoChange(this);" NL
        "" NL
        "    // Notify object it is going to change" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoChanging();" NL
        "" NL
        "    // Restore member state via CbArchive on a memory stream: serialize" NL
        "    // the saved snapshot out, then deserialize back into the live object." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        " + pUndoChange->GetFirstMember()->GetPrefixedName() + "->Serialize(store);" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->Serialize(load);" NL
        "    }" NL
        "" NL
        "    // Restore the state of the relations" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RestoreReferences(" + pUndoChange->GetFirstMember()->GetPrefixedName() + ");" NL
        "" NL
        "    // Notify object it has changed" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoChanged();" NL
        "" NL
        "    delete this;" NL;
    pMethod->SetCode(code);

    BaseClass::MethodIterator iUndoChangeMethod(pUndoChange, &Method::IsDestructor);
    if (++iUndoChangeMethod)
    {
        code =
            "    DestructorInclude();" NL
            "" NL
            "    // This isn't in use, so get rid of it, but make it destructable first." NL
            "    " + pUndoChange->GetFirstMember()->GetPrefixedName() + "->CleanupReferences();" NL
            "    delete " + pUndoChange->GetFirstMember()->GetPrefixedName() + ";" NL;
        iUndoChangeMethod->SetCode(code);
        iUndoChangeMethod->SetNote("");
    }

    
    pRedoChange = new Class(this);
    pRedoChange->SetName("RedoChange");
    pRedoChange->SetCppFile(pRedoChange->GetName() + ".cpp");
    pRedoChange->SetHFile(pRedoChange->GetName() + ".h");
    pRedoChange->SetNote(
        "An object state has been changed and undone, if Restore is called this change" NL
        "is redone. The member variable '_pX' holds a pointer to the object to" NL
        "change. Member variable '_pXSave' points to an unreferenced copy that" NL
        "has the redo state of '_pX'.");
    pRedoChange->SetSerialize(0);

    pInherit = new Inherit(pRedoChange, pRedoBase);
    pInherit->SetNote("Basic properties like being on the redo stack are inherited");
    
    pMember = new Member(pRedoChange, GetDocumentObject());
    pMember->SetName("p" + GetDocumentObject()->GetBaseName() + "Save");
    pMember->SetNote("Points to an unreferenced copy that has the redo state of the object involved.");
    pMember->SetPointer(1);
    pMember->SetInitialization("NULL");
    pMember->SetAccess(PRIVATE);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pConstructor = new Constructor(pRedoChange);
    pArgument = new Argument(pConstructor, pUndoChange);
    pArgument->SetName("p" + pUndoChange->GetBaseName());
    pArgument->SetPointer(1);
    pMethod->SetVirtual(1);
    init = 
        "    : " + pRedoBase->GetName() + "(" + pArgument->GetName() + ")" NL
        "    , " + pRedoChange->GetFirstMember()->GetPrefixedName() + "(NULL)" NL;
    pConstructor->SetInit(init);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Snapshot the object's state into a sibling instance via CbArchive" NL
        "    // on a memory stream. The polymorphic operator instantiates the" NL
        "    // right subclass via CbClassRegistration." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        store << " + pRedoBase->GetFirstMember()->GetPrefixedName() + ";" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        CbObject* tmp = NULL;" NL
        "        load >> tmp;" NL
        "        " + pRedoChange->GetFirstMember()->GetPrefixedName() + " = static_cast<" + GetDocumentObject()->GetName() + "*>(tmp);" NL
        "    }" NL
        "" NL
        "    // Save the state of the relations" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->SaveReferences(" + pRedoChange->GetFirstMember()->GetPrefixedName() + ");" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor needed if a change undo is performed and the corresponding redo has" NL
        "to be popped on stack.");

    pMethod = new Method(pRedoChange, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Save the current state first" NL
        "    (void)new UndoChange(this);" NL
        "" NL
        "    // Notify object it is going to change" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoChanging();" NL
        "" NL
        "    // Restore member state via CbArchive on a memory stream: serialize" NL
        "    // the saved snapshot out, then deserialize back into the live object." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        " + pRedoChange->GetFirstMember()->GetPrefixedName() + "->Serialize(store);" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->Serialize(load);" NL
        "    }" NL
        "" NL
        "    // Restore the state of the relations" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->RestoreReferences(" + pRedoChange->GetFirstMember()->GetPrefixedName() + ");" NL
        "" NL
        "    // Notify object it has changed" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoChanged();" NL
        "" NL
        "    delete this;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Redo the change.");

    BaseClass::MethodIterator iRedoChangeMethod(pRedoChange, &Method::IsDestructor);
    if (++iRedoChangeMethod)
    {
        code =
            "    DestructorInclude();" NL
            "" NL
            "    // This isn't in use, so get rid of it, but make it destructable first." NL
            "    " + pRedoChange->GetFirstMember()->GetPrefixedName() + "->CleanupReferences();" NL
            "    delete " + pRedoChange->GetFirstMember()->GetPrefixedName() + ";" NL;
        iRedoChangeMethod->SetCode(code);
        iRedoChangeMethod->SetNote("");
    }


    // Now the redo type is know, so the following constructor for the UndoChange can
    // be made
    pConstructor = new Constructor(pUndoChange);
    pArgument = new Argument(pConstructor, pRedoChange);
    pArgument->SetName("p" + pRedoChange->GetBaseName());
    pArgument->SetPointer(1);
    init = 
        "    : " + pUndoBase->GetName() + "(" + pArgument->GetName() + ")" NL
        "    , " + pUndoChange->GetFirstMember()->GetPrefixedName() + "(NULL)" NL;
    pConstructor->SetInit(init);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Snapshot the object's state into a sibling instance via CbArchive" NL
        "    // on a memory stream. The polymorphic operator instantiates the" NL
        "    // right subclass via CbClassRegistration." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        store << " + pUndoBase->GetFirstMember()->GetPrefixedName() + ";" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        CbObject* tmp = NULL;" NL
        "        load >> tmp;" NL
        "        " + pUndoChange->GetFirstMember()->GetPrefixedName() + " = static_cast<" + GetDocumentObject()->GetName() + "*>(tmp);" NL
        "    }" NL
        "" NL
        "    // Save the state of the relations" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->SaveReferences(" + pUndoChange->GetFirstMember()->GetPrefixedName() + ");" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote("Undo constructor used if a redo is performed.");
}//@CODE_4909


void DataModel::InitUndoRedoDelete(Class* pUndoBase, Class* pRedoBase,
                                   Class*& pUndoDelete, Class*& pRedoDelete)
{//@CODE_4888
    Inherit* pInherit;
    Method* pMethod;
    Constructor* pConstructor;
    Argument* pArgument;
    CbString code;
    CbString init;
    
    pUndoDelete = new Class(this);
    pUndoDelete->SetName("UndoDelete");
    pUndoDelete->SetCppFile(pUndoDelete->GetName() + ".cpp");
    pUndoDelete->SetHFile(pUndoDelete->GetName() + ".h");
    pUndoDelete->SetNote(
        "An object has been deleted, if Restore is called this delete is undone. The" NL
        "member variable '_pX' holds a pointer to the deleted object. Note that" NL
        "by calling Delete on X no actual delete is performed, but all references" NL
        "within the document to it are removed.");
    pUndoDelete->SetSerialize(0);

    pInherit = new Inherit(pUndoDelete, pUndoBase);
    pInherit->SetNote("Basic properties like being on the undo stack are inherited");
    
    pConstructor = new Constructor(pUndoDelete);
    pArgument = new Argument(pConstructor, GetDocumentObject());
    pArgument->SetName("p" + GetDocumentObject()->GetBaseName());
    pArgument->SetPointer(1);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Notify object it is going to be removed" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoRemoving();" NL
        "" NL
        "    // Make it a dead object by removing all references to it" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RemoveReferences();" NL
        "" NL
        "    Get" + GetDocument()->GetBaseName() + "()->MoveUndoBaseLast(this);" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor for the making an undo object for the case an object is deleted" NL
        "from the document");

    pMethod = new Method(pUndoDelete, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Restore the relations, with itself as example," NL
        "    // so it is placed back into its original context" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RestoreReferences(" + pUndoBase->GetFirstMember()->GetPrefixedName() + ");" NL
        "" NL
        "    // Notify object it has added" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoAdded();" NL
        "" NL
        "    // Make it redoable" NL
        "    (void)new RedoDelete(this);" NL
        "" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + " = NULL;" NL
        "    delete this;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Undo the delete.");

    BaseClass::MethodIterator iMethod(pUndoDelete, &Method::IsDestructor);
    if (++iMethod)
    {
        code =
            "    DestructorInclude();" NL
            "" NL
            "    if (" + pUndoBase->GetFirstMember()->GetPrefixedName() + ")" NL
            "    {" NL
            "        // This isn't in use, so get rid of it, but make it destructable first." NL
            "        " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->CleanupReferences();" NL
            "        delete " + pUndoBase->GetFirstMember()->GetPrefixedName() + ";" NL
            "    }" NL;
        iMethod->SetCode(code);
        iMethod->SetNote("");
    }

    
    pRedoDelete = new Class(this);
    pRedoDelete->SetName("RedoDelete");
    pRedoDelete->SetCppFile(pRedoDelete->GetName() + ".cpp");
    pRedoDelete->SetHFile(pRedoDelete->GetName() + ".h");
    pRedoDelete->SetNote(
        "An object has been deleted and undone, if Restore is called this delete is" NL
        "redone. The member variable '_pX' holds a pointer to the object to" NL
        "redelete.");
    pRedoDelete->SetSerialize(0);

    pInherit = new Inherit(pRedoDelete, pRedoBase);
    pInherit->SetNote("Basic properties like being on the redo stack are inherited.");

    pConstructor = new Constructor(pRedoDelete);
    pArgument = new Argument(pConstructor, pUndoDelete);
    pArgument->SetName("p" + pUndoDelete->GetBaseName());
    pArgument->SetPointer(1);
    pMethod->SetVirtual(1);
    init = "    : " + pRedoBase->GetName() + "(" + pArgument->GetName() + ")";
    pConstructor->SetInit(init);
    pConstructor->SetNote(
        "Constructor needed if a delete undo is performed and the corresponding redo has" NL
        "to be popped on stack.");

    pMethod = new Method(pRedoDelete, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Make a undoable delete, the constructor take care of" NL
        "    // all book keeping" NL
        "    (void)new UndoDelete(this);" NL
        "" NL
        "    delete this;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Redo the delete.");


    // Now the redo type is know, so the following constructor for the UndoDelete can
    // be made
    pConstructor = new Constructor(pUndoDelete);
    pArgument = new Argument(pConstructor, pRedoDelete);
    pArgument->SetName("p" + pRedoDelete->GetBaseName());
    pArgument->SetPointer(1);
    init = "    : " + pUndoBase->GetName() + "(" + pArgument->GetName() + ")";
    pConstructor->SetInit(init);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Notify object it is going to be removed" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoRemoving();" NL
        "" NL
        "    // Make it a dead object by removing all references to it" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RemoveReferences();" NL
        "" NL
        "    Get" + GetDocument()->GetBaseName() + "()->MoveUndoBaseLast(this);" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor needed if a new redo is performed and the corresponding undo has to" NL
        "be popped on stack.");
}//@CODE_4888


void DataModel::InitUndoRedoNew(Class* pUndoBase, Class* pRedoBase,
                                Class*& pUndoNew, Class*& pRedoNew)
{//@CODE_4914
    Inherit* pInherit;
    Method* pMethod;
    Constructor* pConstructor;
    Argument* pArgument;
    CbString code;
    CbString init;
    
    pUndoNew = new Class(this);
    pUndoNew->SetName("UndoNew");
    pUndoNew->SetCppFile(pUndoNew->GetName() + ".cpp");
    pUndoNew->SetHFile(pUndoNew->GetName() + ".h");
    pUndoNew->SetNote(
        "A new object has been added, if Restore is called this new is undone. The"
        "member variable '_pDocObject' holds a pointer to the new object.");
    pUndoNew->SetSerialize(0);

    pInherit = new Inherit(pUndoNew, pUndoBase);
    pInherit->SetNote("Basic properties like being on the undo stack are inherited.");
    
    pConstructor = new Constructor(pUndoNew);
    pArgument = new Argument(pConstructor, GetDocumentObject());
    pArgument->SetName("p" + GetDocumentObject()->GetBaseName());
    pArgument->SetPointer(1);
    pConstructor->SetNote(
        "Constructor for the making an undo object for the case a new object is added to" NL
        "the document.");

    pMethod = new Method(pUndoNew, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Make a redoable new, the constructor take care of" NL
        "    // all book keeping" NL
        "    (void)new RedoNew(this);" NL
        "" NL
        "    delete this;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Undo the recorded new.");

    
    pRedoNew = new Class(this);
    pRedoNew->SetName("RedoNew");
    pRedoNew->SetCppFile(pRedoNew->GetName() + ".cpp");
    pRedoNew->SetHFile(pRedoNew->GetName() + ".h");
    pRedoNew->SetNote(
        "A new object has been added and undone, if Restore is called this new is" NL
        "redone. The member variable '_pX' holds a pointer to the object" NL
        "removed by the undo operation.");
    pRedoNew->SetSerialize(0);

    pInherit = new Inherit(pRedoNew, pRedoBase);
    pInherit->SetNote("Basic properties like being on the redo stack are inherited.");

    pConstructor = new Constructor(pRedoNew);
    pArgument = new Argument(pConstructor, pUndoNew);
    pArgument->SetName("p" + pUndoNew->GetBaseName());
    pArgument->SetPointer(1);
    pMethod->SetVirtual(1);
    init = "    : " + pRedoBase->GetName() + "(" + pArgument->GetName() + ")";
    pConstructor->SetInit(init);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Notify object it is going to be removed" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoRemoving();" NL
        "" NL
        "    // Make it a dead object by removing all references to it" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->RemoveReferences();" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor needed if a new undo is performed and the corresponding redo has" NL
        "to be popped on stack.");

    pMethod = new Method(pRedoNew, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Restore the relations, with itself as example," NL
        "    // so it is placed back into its original context" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->RestoreReferences(" + pUndoBase->GetFirstMember()->GetPrefixedName() + ");" NL
        "" NL
        "    // Notify object it has added" NL
        "    " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoAdded();" NL
        "" NL
        "    // Make it undoable" NL
        "    (void)new UndoNew(this);" NL
        "" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + " = NULL;" NL
        "    delete this;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Redo the recorded new.");

    BaseClass::MethodIterator iMethod(pRedoNew, &Method::IsDestructor);
    if (++iMethod)
    {
        code =
            "    DestructorInclude();" NL
            "" NL
            "    if (" + pRedoBase->GetFirstMember()->GetPrefixedName() + ")" NL
            "    {" NL
            "        // This isn't in use, so get rid of it, but make it destructable first." NL
            "        " + pRedoBase->GetFirstMember()->GetPrefixedName() + "->CleanupReferences();" NL
            "        delete " + pRedoBase->GetFirstMember()->GetPrefixedName() + ";" NL
            "    }" NL;
        iMethod->SetCode(code);
        iMethod->SetNote("");
    }

    // Now the redo type is know, so the following constructor for the UndoNew can
    // be made
    pConstructor = new Constructor(pUndoNew);
    pArgument = new Argument(pConstructor, pRedoNew);
    pArgument->SetName("p" + pRedoNew->GetBaseName());
    pArgument->SetPointer(1);
    init = "    : " + pUndoBase->GetName() + "(" + pArgument->GetName() + ")";
    pConstructor->SetInit(init);
    pConstructor->SetNote(
        "Constructor needed if a delete redo is performed and the corresponding undo has" NL
        "to be popped on stack.");
}//@CODE_4914


void DataModel::InitUndoSubChange(Class* pUndoBase, Class*& pUndoSubChange)
{//@CODE_5845
    Inherit* pInherit;
    Method* pMethod;
    Member* pMember;
    Constructor* pConstructor;
    Argument* pArgument;
    CbString code;
    CbString init;
    
    pUndoSubChange = new Class(this);
    pUndoSubChange->SetName("UndoSubChange");
    pUndoSubChange->SetCppFile(pUndoSubChange->GetName() + ".cpp");
    pUndoSubChange->SetHFile(pUndoSubChange->GetName() + ".h");
    pUndoSubChange->SetNote(
        "An object state has been changed, if Restore is called this change is undone." NL
        "The member variable '_pX' holds a pointer to the changed object." NL
        "Member variable '_pXSave' points to an unreferenced that which has the" NL
        "previous state of '_pX'. This is an automatically invoked one, so Redo" NL
        "generation is needed");
    pUndoSubChange->SetSerialize(0);

    pInherit = new Inherit(pUndoSubChange, pUndoBase);
    pInherit->SetNote("Basic properties like being on the undo stack are inherited.");
    
    pMember = new Member(pUndoSubChange, GetDocumentObject());
    pMember->SetName("p" + GetDocumentObject()->GetBaseName() + "Save");
    pMember->SetNote("Points to an unreferenced copy that has the previous state of involved object.");
    pMember->SetPointer(1);
    pMember->SetInitialization("NULL");
    pMember->SetAccess(PRIVATE);
    pMethod = new GetMemberMethod(pMember);
    pMethod->SetAccess(PUBLIC);

    pConstructor = new Constructor(pUndoSubChange);
    pArgument = new Argument(pConstructor, GetDocumentObject());
    pArgument->SetName("p" + GetDocumentObject()->GetBaseName());
    pArgument->SetPointer(1);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Snapshot the object's state into a sibling instance via CbArchive" NL
        "    // on a memory stream. The polymorphic operator instantiates the" NL
        "    // right subclass via CbClassRegistration." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        store << " + pUndoBase->GetFirstMember()->GetPrefixedName() + ";" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        CbObject* tmp = NULL;" NL
        "        load >> tmp;" NL
        "        " + pUndoSubChange->GetFirstMember()->GetPrefixedName() + " = static_cast<" + GetDocumentObject()->GetName() + "*>(tmp);" NL
        "    }" NL
        "" NL
        "    // Save the state of the relations" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->SaveReferences(" + pUndoSubChange->GetFirstMember()->GetPrefixedName() + ");" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor for the making an undo object for the case an object is changed in" NL
        "the document");


    pMethod = new Method(pUndoSubChange, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetNote("Undo the change.");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Notify object it is going to change" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoChanging();" NL
        "" NL
        "    // Restore member state via CbArchive on a memory stream: serialize" NL
        "    // the saved snapshot out, then deserialize back into the live object." NL
        "    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);" NL
        "    {" NL
        "        CbArchive store(static_cast<std::ostream&>(ss));" NL
        "        " + pUndoSubChange->GetFirstMember()->GetPrefixedName() + "->Serialize(store);" NL
        "    }" NL
        "    ss.seekg(0);" NL
        "    {" NL
        "        CbArchive load(static_cast<std::istream&>(ss));" NL
        "        " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->Serialize(load);" NL
        "    }" NL
        "" NL
        "    // Restore the state of the relations" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RestoreReferences(" + pUndoSubChange->GetFirstMember()->GetPrefixedName() + ");" NL
        "" NL
        "    // Notify object it has changed" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoChanged();" NL
        "" NL
        "    delete this;" NL;
    pMethod->SetCode(code);

    BaseClass::MethodIterator iUndoChangeMethod(pUndoSubChange, &Method::IsDestructor);
    if (++iUndoChangeMethod)
    {
        code =
            "    DestructorInclude();" NL
            "" NL
            "    // This isn't in use, so get rid of it, but make it destructable first." NL
            "    " + pUndoSubChange->GetFirstMember()->GetPrefixedName() + "->CleanupReferences();" NL
            "    delete " + pUndoSubChange->GetFirstMember()->GetPrefixedName() + ";" NL;
        iUndoChangeMethod->SetCode(code);
        iUndoChangeMethod->SetNote("");
    }

    

}//@CODE_5845


void DataModel::InitUndoSubDelete(Class* pUndoBase, Class*& pUndoSubDelete)
{//@CODE_4889
    Inherit* pInherit;
    Method* pMethod;
    Constructor* pConstructor;
    Argument* pArgument;
    CbString code;
    CbString init;
    
    pUndoSubDelete = new Class(this);
    pUndoSubDelete->SetName("UndoSubDelete");
    pUndoSubDelete->SetCppFile(pUndoSubDelete->GetName() + ".cpp");
    pUndoSubDelete->SetHFile(pUndoSubDelete->GetName() + ".h");
    pUndoSubDelete->SetNote(
        "A resulting delete from another delete, a redo of it is unneccesarry and" NL
        "unwanted.");
    pUndoSubDelete->SetSerialize(0);

    pInherit = new Inherit(pUndoSubDelete, pUndoBase);
    pInherit->SetNote("Basic properties like being on the undo stack are inherited.");
    
    pConstructor = new Constructor(pUndoSubDelete);
    pArgument = new Argument(pConstructor, GetDocumentObject());
    pArgument->SetName("p" + GetDocumentObject()->GetBaseName());
    pArgument->SetPointer(1);
    code =
        "    ConstructorInclude();" NL
        "" NL
        "    // Notify object it is going to be removed" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoRemoving();" NL
        "" NL
        "    // Make it a dead object by removing all references to it" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RemoveReferences();" NL
        "" NL
        "    Get" + GetDocument()->GetBaseName() + "()->MoveUndoBaseLast(this);" NL;
    pConstructor->SetCode(code);
    pConstructor->SetNote(
        "Constructor for the making an undo object for the case an object is indirectly" NL
        "deleted from the document. So the delete is a result from another delete.");

    pMethod = new Method(pUndoSubDelete, GetDataModelDoc()->FindType("void"));
    pMethod->SetName("Restore");
    pMethod->SetAccess(PUBLIC);
    pMethod->SetVirtual(1);
    code =
        "    // Restore the relations, with itself as example," NL
        "    // so it is placed back into its original context" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->RestoreReferences(" + pUndoBase->GetFirstMember()->GetPrefixedName() + ");" NL
        "" NL
        "    // Notify object it has added" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->OnUndoRedoAdded();" NL
        "" NL
        "    " + pUndoBase->GetFirstMember()->GetPrefixedName() + " = NULL;" NL
        "    delete this;" NL;
    pMethod->SetCode(code);
    pMethod->SetNote("Undo the delete.");

    BaseClass::MethodIterator iMethod(pUndoSubDelete, &Method::IsDestructor);
    if (++iMethod)
    {
        code =
            "    DestructorInclude();" NL
            "" NL
            "    if (" + pUndoBase->GetFirstMember()->GetPrefixedName() + ")" NL
            "    {" NL
            "        // This isn't in use, so get rid of it, but make it destructable first." NL
            "        " + pUndoBase->GetFirstMember()->GetPrefixedName() + "->CleanupReferences();" NL
            "        delete " + pUndoBase->GetFirstMember()->GetPrefixedName() + ";" NL
            "    }" NL;
        iMethod->SetCode(code);
        iMethod->SetNote("");
    }
}//@CODE_4889


int DataModel::OnAddClass(bool checkOnly)
{//@CODE_950
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        Class* pClass = new Class(this);
        pClass->SetOrder(GetClassCount()-1); // Make it the last in the tree view

        if (pClass->OnEditAttributes())
        {
            // Post-dialog: coalesce the Add cascade (class + its seeded methods).
            CbViewLock lock(GetDataModelDoc());
            if (pClass->GetSerialize())
            {
                Inherit* pInherit = new Inherit(pClass, GetDocumentObject());
                pInherit->SetVirtual(1);
            }
            pClass->Add();
        }
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_950


int DataModel::OnAddClassDiagram(bool checkOnly)
{//@CODE_3894
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        ClassDiagram* pClassDiagram = new ClassDiagram(this);

        if (pClassDiagram->OnEditAttributes())
            pClassDiagram->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_3894


int DataModel::OnAddGroup(bool checkOnly)
{//@CODE_951
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        ClassGroup* pClassGroup = new ClassGroup(this);
        pClassGroup->SetOrder(GetClassGroupCount()-1); // Make it the last in the tree view

        if (pClassGroup->OnEditAttributes())
            pClassGroup->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_951


int DataModel::OnAddMetaGroup(bool checkOnly)
{//@CODE_29602
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        MetaGroup* pMetaGroup = new MetaGroup(this);
        pMetaGroup->SetOrder(GetMetaGroupCount()); // Make it the last in the tree view

        if (pMetaGroup->OnEditAttributes())
            pMetaGroup->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_29602


int DataModel::OnAddSequenceDiagram(bool checkOnly)
{//@CODE_30481
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        SequenceDiagram* pSequenceDiagram = new SequenceDiagram(this);

        if (pSequenceDiagram->OnEditAttributes())
            pSequenceDiagram->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_30481


int DataModel::OnEditAttributes(bool checkOnly)
{//@CODE_949
	if (checkOnly)
		return 1;

    std::string className;
    if (GetDocument())
        className = (const char*)GetDocument()->GetName();
    void* ownerHwnd = Cb_OwnerHwnd();

    DataModelDialogResult dlgResult =
        Qt_ShowDataModelDialog(this, className, ownerHwnd);

    if (dlgResult.accepted)
    {
        if (dlgResult.modelChanged)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_949


int DataModel::OnEditContext(bool checkOnly)
{//@CODE_25803
    if (checkOnly)
        return true;

    UndoBase* pLastUndoBase = GetDataModelDoc()->MarkLastUndo();
    void* ownerHwnd = Cb_OwnerHwnd();

    if (Qt_ShowContextDeclarationDialog(this, ownerHwnd))
    {
        // Close the dialog's undo transaction. No manual dirty set: any
        // change the dialog applied went through SaveState, which dirtied
        // the doc already (two-place dirty rule).
        GetDataModelDoc()->MarkLastUndo();
        return true;
    }
    else
    {
        GetDataModelDoc()->RollBack(pLastUndoBase);
        return false;
    }
}//@CODE_25803


int DataModel::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_35074
    SequenceDiagram* pSequenceDiagram = dynamic_cast<SequenceDiagram*>(pGti);
    if (pSequenceDiagram && pSequenceDiagram->GetDataModelDoc() == GetDataModelDoc())
    {
        if (!checkOnly)
        {
            SequenceDiagram* pNewSequenceDiagram = 
                new SequenceDiagram(this, pSequenceDiagram);
            pNewSequenceDiagram->Add();
        }
        
        return 1;
    }
        
    ClassDiagram* pClassDiagram = dynamic_cast<ClassDiagram*>(pGti);
    if (pClassDiagram && pClassDiagram->GetDataModelDoc() == GetDataModelDoc())
    {
        if (!checkOnly)
        {
            ClassDiagram* pNewClassDiagram = 
                new ClassDiagram(this, pClassDiagram);
            pNewClassDiagram->Add();
        }
        
        return 1;
    }
        
    return 0;
}//@CODE_35074


/*@NOTE_23450
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void DataModel::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_23450
    Gti::OnUndoRedoChanged(pOldState);

    DataModel* pDataModel = (DataModel*)pOldState;
    
    if (pDataModel &&
        GetPhaseSupport() != pDataModel->GetPhaseSupport())
    {
        DataModelDoc::GtiIterator iGti(GetDataModelDoc());
        while (++iGti)
        {
            if (iGti->GetPhase() != None_Phase)
            {
                iGti->Gti::Update();
            }
        }
    }
    
}//@CODE_23450


void DataModel::ReadAllFiles(ParseLogInterface* pDialog, bool unconditional)
{//@CODE_941
    SaveState();
    int version = GetDataModelDoc()->GetVersion();
    int newVersion = version;
    
    _maxLastSave = _lastSave;
    DataModel::ClassIterator iClass(this);
    while (++iClass)
    {
        iClass->ReadHFile(pDialog, unconditional);
        iClass->ReadCppFile(pDialog, unconditional);
        pDialog->StepProgress();

        if (iClass->GetVersion() > version)
            newVersion = version+1;
    }

    ReadHFile(pDialog, unconditional);
    pDialog->StepProgress();

    if (GetVersion() > version)
        newVersion = version+1;

    if (version != newVersion)
        GetDataModelDoc()->SetVersion(newVersion);

    // SaveState at the top already dirtied the doc (two-place dirty rule).
    pDialog->AddLog("- Ready");
    _lastSave = _maxLastSave;
}//@CODE_941


void DataModel::ReadHFile(ParseLogInterface* pDialog, bool unconditional)
{//@CODE_945
    extern int ReadHSource(ParseLogInterface* pDialog, DataModel* pDataModel);

    struct _stat buf;
    if (_stat(GetHFile(), &buf) == 0)
    {
        CbTime time = CbTime(buf.st_mtime);
        if (unconditional || GetLastSave() < time)
        {
            ReadHSource(pDialog, this);

            SetMaxLastSave(time);
        }
    }
}//@CODE_945


/*@NOTE_23229
Virtual method to replace strings at various places, called if a type name changes.
*/
void DataModel::ReplaceInX(const CbString& oldString, const CbString& newString)
{//@CODE_23229
    ReplaceInStr(_hUser1, oldString, newString);
    ReplaceInStr(_hUser2, oldString, newString);
}//@CODE_23229


void DataModel::SaveAllFiles(SourceLogInterface* pDialog)
{//@CODE_937
    if (CheckUpdates())
    {
        CbString str;
        str.Format("Are you sure you want to continue saving the source files");
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_OKCANCEL) == CBMB_IDCANCEL)
        {
            pDialog->AddLog("- Cancelled 'Save All'");
            return;
        }
    }

    int version = GetDataModelDoc()->GetVersion();
    int newVersion = version;

    // Reset lastsave, needed if the time was by accident in the future, otherwise
    // we can never get the last save time back to a normal time
    SetLastSave(CbTime(1980, 1, 1, 0, 0, 0));
    
    DataModel::ClassIterator iClass(this);
    while (++iClass)
    {
        if (iClass->GetVersion() > version)
        {
            newVersion = version+1;
        }

        iClass->SortMethod(Method::CompareFileSaveState);

        iClass->WriteHFile(pDialog, true);
        iClass->WriteCppFile(pDialog, true);
        pDialog->StepProgress();
    }

    WriteHFile(pDialog, true);
    pDialog->StepProgress();

    if (version != newVersion)
        GetDataModelDoc()->SetVersion(newVersion);

    // No SetModifiedFlag: OnSaveDocument below saves the .cbz and ends with
    // the flag cleared anyway -- setting it first was a no-op.
    GetDataModelDoc()->OnSaveDocument(GetDataModelDoc()->GetPathName());

    pDialog->AddLog("- Ready");
}//@CODE_937


void DataModel::SaveModifiedFiles(SourceLogInterface* pDialog)
{//@CODE_939
    if (CheckUpdates())
    {
        CbString str;
        str.Format("Are you sure you want to continue saving the source files");
        if (CbMessageBox(str, CBMB_ICONQUESTION|CBMB_YESNO) == CBMB_IDNO)
        {
            pDialog->AddLog("- Cancelled 'Save Modifications'");
            return;
        }
    }

    int version = GetDataModelDoc()->GetVersion();
    int newVersion = version;
    
    bool writeMaster = false;
    DataModel::ClassIterator iClass(this);
    while (++iClass)
    {
        if (iClass->GetVersion() > version)
        {
            newVersion = version+1;
        }
        iClass->SortMethod(Method::CompareFileSaveState);

        if (iClass->WriteHFile(pDialog))
            writeMaster = true;
        iClass->WriteCppFile(pDialog);
        pDialog->StepProgress();
    }

    WriteHFile(pDialog, writeMaster);
    pDialog->StepProgress();

    if (version != newVersion)
        GetDataModelDoc()->SetVersion(newVersion);

    // No SetModifiedFlag: OnSaveDocument below saves the .cbz and ends with
    // the flag cleared anyway -- setting it first was a no-op.
    GetDataModelDoc()->OnSaveDocument(GetDataModelDoc()->GetPathName());

    pDialog->AddLog("- Ready");
}//@CODE_939


/*@NOTE_1526
Sort items alphabetically on their name
*/
int DataModel::SortOnName(bool checkOnly)
{//@CODE_1526
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortClass(Class::CompareName);
        SortClassGroup(ClassGroup::CompareName);

        int i = 0;
        ClassIterator iClass(this);
        while (++iClass)
        {
            if (!iClass->GetClassGroup())
            {
                iClass->SaveState();
                iClass->SetOrder(i++);
            }
        }

        i = 0;
        ClassGroupIterator classGroup(this);
        while (++classGroup)
        {
            classGroup->SaveState();
            classGroup->SetOrder(i++);
        }
    }

    return 1;
}//@CODE_1526


int DataModel::SortOnPhase(bool checkOnly)
{//@CODE_23472
    if (!checkOnly)
    {
        // Coalesce the per-row SaveState refreshes into one flush; the lock
        // dtor fires it (CbViewLock also shows the wait cursor). Each child's
        // SaveState already notifies its views, so the old trailing
        // NotifyStructureChanged() was redundant.
        CbViewLock lock(GetDataModelDoc());

        SortClass(Class::ComparePhase);
        SortClassGroup(ClassGroup::ComparePhase);

        int i = 0;
        ClassIterator iClass(this);
        while (++iClass)
        {
            if (!iClass->GetClassGroup())
            {
                iClass->SaveState();
                iClass->SetOrder(i++);
            }
        }

        i = 0;
        ClassGroupIterator classGroup(this);
        while (++classGroup)
        {
            classGroup->SaveState();
            classGroup->SetOrder(i++);
        }
    }

    return GetPhaseSupport();
}//@CODE_23472


void DataModel::Update()
{//@CODE_948
    if (GetAdded())
    {
        SetItemText(_name);

        Gti::Update();
    }
}//@CODE_948


void DataModel::WriteHFile(SourceLogInterface* pDialog, bool unconditional)
{//@CODE_943
    // Set initial buffer size at 16Kbyte
    static CbStringBuilder newContent(1<<14);
    
    // Empty string, while keeping alloc size.
    newContent.Empty();
    int startDateTime, endDateTime;
    WriteHFile(newContent, startDateTime, endDateTime);
    
    bool changed = true;
    
    if (!unconditional)
    {
        ifstream is(GetHFile(), ios::in | ios::binary);
        if (is)
        {
            is.seekg(0, ios::end);
            const long size = (long)is.tellg();
            if (size > 0 && size == (long)newContent.GetLength())
            {
                // Compare around the embedded date/time stamp (it always
                // differs): head [0, startDateTime) and tail [endDateTime, size).
                // Portable replacement for the old Win32 file-mapping compare.
                char* oldContent = new char[size];
                is.seekg(0, ios::beg);
                is.read(oldContent, size);
                if (is.gcount() == size)
                {
                    const char* newContent2 = (const char*)newContent + endDateTime;
                    const char* oldContent2 = oldContent + endDateTime;
                    if (strncmp(newContent, oldContent, (size_t)startDateTime) == 0 &&
                        strncmp(newContent2, oldContent2, (size_t)(size - endDateTime)) == 0)
                    {
                        changed = false;
                    }
                }
                delete[] oldContent;
            }
        }
    }
    
    if (changed)
    {
        ofstream os(GetHFile(), ios::out|ios::binary);
        if (os)
        {
            CbString str;
            str.Format("- Writing file '%s'", GetHFile().c_str());
            pDialog->AddLog(str);
            
            CbString cbOut;
            cbOut += newContent;
            if (!GetCrlf())
                cbOut.Replace("\r\n", "\n");
            os << cbOut;
            os.close();
            
            struct _stat buf;
            if (_stat(GetHFile(), &buf) == 0)
            {
                CbTime time = CbTime(buf.st_mtime);
                if (GetLastSave() < time)
                {
                    SetLastSave(time);
                }
            }
        }
        else
        {
            CbString str;
            str.Format("! Error can not open file '%s'", GetHFile().c_str());
            pDialog->AddLogError(str);
        }
    }
}//@CODE_943


/*@NOTE_7360
Write contents of file to string 'str'.
*/
void DataModel::WriteHFile(CbStringBuilder& str, int& startDateTime,
                           int& endDateTime)
{//@CODE_7360
    CbString def = "_" + GetHFileWithoutPath();
    def.MakeUpper();
    int index;
    while ((index = def.Find('.')) != -1)
        def.SetAt(index, '_');
    
    str += "#ifndef " + def + NL;
    str += "#define " + def + NL NL;
    
    startDateTime = str.GetLength();
    CbString name = GetName();
    name.MakeUpper();
    GetDataModelDoc()->ConvertNonCSymbols(name);
    CbTime currentTime = CbTime::GetCurrentTime();
    CbString version;
    version.Format("%d", GetDataModelDoc()->GetVersion());
    str += "// Date, Time & Version defines" NL;
    str += "#define " + name + "_DATE    " + currentTime.Format("%Y%m%d") + NL;
    str += "#define " + name + "_TIME    " + currentTime.Format("%H%M%S") + NL;
    str += "#define " + name + "_VERSION " + version + NL NL;
    endDateTime = str.GetLength();

    str += "// Context define declarations" NL;
    ContextDeclarationIterator iContextDeclaration(this, &ContextDeclaration::GetEnableDefineDeclaration);
    while (++iContextDeclaration)
    {
        if (!iContextDeclaration->GetDefineDeclaration().IsEmpty())
        {
            str += iContextDeclaration->GetDefineDeclaration() + NL;
        }
    }
    str += NL;
    
    if (GetClassCount() != GetDataModelDoc()->GetBaseClassCount())
    {
        str += "// Forward extern class declarations" NL;
        DataModelDoc::BaseClassIterator iBaseClass(GetDataModelDoc());
        while (++iBaseClass)
        {
            // A forward declaration isn't enough if it is used to inherit from, so
            // better discard it. 
            if (!iBaseClass->IsClass() && iBaseClass->GetInheritCount() == 0)
            {
                bool forward = true;
                if (iBaseClass->GetSuppressForwardDeclaration() ||
                    iBaseClass->GetVariableCount() == 0)
                {
                    forward = false;
                }
                
                Type::VariableIterator iVariable(iBaseClass);
                while (forward && ++iVariable)
                {
                    // If it is not used as a pointer or a reference then a forward isn't enough, so leave it.
                    if (!iVariable->GetPointer() && !iVariable->GetReference())
                    {
                        forward = false;
                    }
                }
                
                if (forward)
                {
                    if (!iBaseClass->GetTemplateDeclaration().IsEmpty())
                    {
                        str += iBaseClass->GetStrippedTemplateDeclaration() + " ";
                    }
                    
                    if (iBaseClass->GetStruct())
                        str += "struct " + iBaseClass->Type::GetName() + ";" NL;
                    else
                        str += "class " + iBaseClass->Type::GetName() + ";" NL;
                }
            }
        }
        str += NL;
    }
    
    str += "//@START_USER1" NL;
    str += GetHUser1();
    str += "//@END_USER1" NL NL;
        
    if (!GetNamespace().IsEmpty())
    {
        str += "namespace " + GetNamespace() + NL;
        str += "{" NL NL;
    }
    
    str += "// Defines needed for relations between templated classes" NL;
    DataModelDoc::BaseClassIterator iBaseClass(GetDataModelDoc());
    while (++iBaseClass)
    {
        str += iBaseClass->DefineTemplate();
    }
    str += NL;
    
    bool first = true;
    DataModelDoc::TypeIterator iType(GetDataModelDoc());
    while (++iType)
    {
        OtherType* pOtherType = dynamic_cast<OtherType*>(iType.Get());
        if (pOtherType && !pOtherType->GetDeclaration().IsEmpty())
        {
            if (first)
            {
                str += "// Type declarations" NL;
                    first = false;
            }
            
            str += "//@START_DECLARATION_" + pOtherType->GetIdAsString() 
                + " " + pOtherType->GetName() + NL;
            str += pOtherType->GetDeclaration();
            str += "//@END_DECLARATION_" + pOtherType->GetIdAsString() + NL NL;
        }
    }
        
    str += "// Forward class declarations" NL;
    int include = 0;
    bool critical = false;
    ClassIterator iClass(this);
    while (++iClass)
    {
        if (!iClass->GetTemplateDeclaration().IsEmpty())
        {
            str += iClass->GetStrippedTemplateDeclaration() + " ";
        }
        
        if (iClass->GetStruct())
            str += "struct " + iClass->Type::GetName() + ";" NL;
        else
            str += "class " + iClass->Type::GetName() + ";" NL;
        iClass->SetFlag(0);
        
        Class::FromRelationIterator relation(iClass);
        while (++relation)
        {
            int includeInstance;
            if (relation->GetSingle())
                includeInstance = SingleType;
            else if (relation->GetStatic())
                includeInstance = StaticMultiType;
            else if (relation->GetRelationMember())
            {
                if (relation->GetRelationMember()->IsUniqueValueTree())
                    includeInstance = UniqueValueTreeType;
                else if (relation->GetRelationMember()->IsValueTree())
                    includeInstance = ValueTreeType;
                else if (relation->GetRelationMember()->IsAvlTree())
                    includeInstance = AvlTreeType;
            }
            else if (relation->GetMulti())
                includeInstance = MultiType;
            
            if (relation->GetOwned())
                includeInstance <<= 1;
            if (relation->GetCritical())
            {
                includeInstance <<= 2;
                critical = true;
            }
            
            include |= includeInstance;
        }
    }
    str += NL;
    
    str += "// Needed ClassBuilder include files" NL;
    if (include & SingleOwnedType)
        str += "#include \"CB_SingleOwned.h\"" NL;
    else if (include & SingleType)
        str += "#include \"CB_Single.h\"" NL;

    if (include & MultiOwnedType)
        str += "#include \"CB_MultiOwned.h\"" NL;
    else if (include & MultiType)
        str += "#include \"CB_Multi.h\"" NL;

    if (include & UniqueValueTreeOwnedType)
        str += "#include \"CB_UniqueValueTreeOwned.h\"" NL;
    else if (include & UniqueValueTreeType)
        str += "#include \"CB_UniqueValueTree.h\"" NL;

    if (include & ValueTreeOwnedType)
        str += "#include \"CB_ValueTreeOwned.h\"" NL;
    else if (include & ValueTreeType)
        str += "#include \"CB_ValueTree.h\"" NL;

    if (include & AvlTreeOwnedType)
        str += "#include \"CB_AvlTreeOwned.h\"" NL;
    else if (include & AvlTreeType)
        str += "#include \"CB_AvlTree.h\"" NL;

    if (include & StaticMultiOwnedType)
        str += "#include \"CB_StaticMultiOwned.h\"" NL;
    else if (include & StaticMultiType)
        str += "#include \"CB_StaticMulti.h\"" NL;

    if (include & CriticalSingleOwnedType)
        str += "#include \"CB_CriticalSingleOwned.h\"" NL;
    else if (include & CriticalSingleType)
        str += "#include \"CB_CriticalSingle.h\"" NL;

    if (include & CriticalMultiOwnedType)
        str += "#include \"CB_CriticalMultiOwned.h\"" NL;
    else if (include & CriticalMultiType)
        str += "#include \"CB_CriticalMulti.h\"" NL;

    if (include & CriticalUniqueValueTreeOwnedType)
        str += "#include \"CB_CriticalUniqueValueTreeOwned.h\"" NL;
    else if (include & CriticalUniqueValueTreeType)
        str += "#include \"CB_CriticalUniqueValueTree.h\"" NL;

    if (include & CriticalValueTreeOwnedType)
        str += "#include \"CB_CriticalValueTreeOwned.h\"" NL;
    else if (include & CriticalValueTreeType)
        str += "#include \"CB_CriticalValueTree.h\"" NL;

    if (include & CriticalAvlTreeOwnedType)
        str += "#include \"CB_CriticalAvlTreeOwned.h\"" NL;
    else if (include & CriticalAvlTreeType)
        str += "#include \"CB_CriticalAvlTree.h\"" NL;

    if (include & CriticalStaticMultiOwnedType)
        str += "#include \"CB_CriticalStaticMultiOwned.h\"" NL;
    else if (include & CriticalStaticMultiType)
        str += "#include \"CB_CriticalStaticMulti.h\"" NL;
    
    str += NL;
    
    str += "// Make sure the inline implementations are skipped" NL;
    str += "#ifdef CB_INLINES" NL;
    str += "#undef CB_INLINES" NL;
    str += "#endif" NL NL;
    
    str += "// Include classes, for declarations" NL;
        
    while (++iClass)
        iClass->WriteRecursiveInclude(str);
    
    // Include again with extra define, to get the inlines.
    str += NL NL "// Include classes again, for inline implementation" NL;
    str += "#define CB_INLINES" NL;
    while (++iClass)
    {
        bool mustInclude = false;
        if ((!iClass->GetTemplate().IsEmpty() && GetTemplateClassHeaderOnly()) || 
            !iClass->GetHUser3().IsEmpty())
        {
            mustInclude = true;
        }
        
        // Find out if there are inlines in the header file
        Class::MethodIterator iMethod(iClass, &Method::IsNonMacroMethod);
        while (!mustInclude && ++iMethod)
        {
            if (iMethod->GetInline() && iMethod->GetImplement() &&
                !iMethod->GetDelete())
            {
                mustInclude = true;
            }
        }
        
        // There is at least one method.
        if (mustInclude)
        {
            str += "#include \"" + iClass->GetHFileWithoutPath() + "\"" NL;
        }
    }
    
    if (!GetNamespace().IsEmpty())
    {
        str += NL "} // end of namespace " + GetNamespace() + NL;
    }
    
    str += NL "//@START_USER2" NL;
    str += GetHUser2();
    str += "//@END_USER2" NL NL;
        
    str += "#endif" NL;
}//@CODE_7360


const CbString& DataModel::GetAuthor()
{//@CODE_1129
    return _author;
}//@CODE_1129


void DataModel::SetAuthor(const CbString& rAuthor)
{//@CODE_1130
    _author = rAuthor;
}//@CODE_1130


/*@NOTE_1534
Returns the value of member '_classPrefix'.
*/
const CbString& DataModel::GetClassPrefix()
{//@CODE_1534
    return _classPrefix;
}//@CODE_1534


/*@NOTE_1535
Set the value of member '_classPrefix' to 'rClassPrefix'.
*/
void DataModel::SetClassPrefix(const CbString& rClassPrefix)
{//@CODE_1535
    if (rClassPrefix != _classPrefix)
    {
        _newClassPrefix = rClassPrefix;

        ClassIterator iClass(this);
        while (++iClass)
        {
            if (iClass->GetBaseName() != iClass->GetName() || _classPrefix.IsEmpty())
            {
                iClass->SetName(rClassPrefix + iClass->GetBaseName());
                iClass->Update();
            }
        }

        _classPrefix = rClassPrefix;
    }
}//@CODE_1535


const CbString& DataModel::GetCppHeader()
{//@CODE_1132
    return _cppHeader;
}//@CODE_1132


void DataModel::SetCppHeader(const CbString& rCppHeader)
{//@CODE_1133
    _cppHeader = rCppHeader;
}//@CODE_1133


const CbString& DataModel::GetHFile()
{//@CODE_1135
    return _hFile;
}//@CODE_1135


void DataModel::SetHFile(const CbString& rHFile)
{//@CODE_1136
    _hFile = rHFile;
}//@CODE_1136


const CbString& DataModel::GetHHeader()
{//@CODE_1138
    return _hHeader;
}//@CODE_1138


void DataModel::SetHHeader(const CbString& rHHeader)
{//@CODE_1139
    _hHeader = rHHeader;
}//@CODE_1139


/*@NOTE_3153
Returns the value of member '_htmlOutput'.
*/
const CbString& DataModel::GetHtmlOutput()
{//@CODE_3153
    return _htmlOutput;
}//@CODE_3153


/*@NOTE_3154
Set the value of member '_htmlOutput' to 'rHtmlOutput'.
*/
void DataModel::SetHtmlOutput(const CbString& rHtmlOutput)
{//@CODE_3154
    _htmlOutput = rHtmlOutput;
}//@CODE_3154


const CbString& DataModel::GetHUser1()
{//@CODE_1141
    return _hUser1;
}//@CODE_1141


void DataModel::SetHUser1(const CbString& rHUser1)
{//@CODE_1142
    _hUser1 = rHUser1;
    if (!rHUser1.IsEmpty())
    {
        if (rHUser1[rHUser1.GetLength()-1] != '\n')
            _hUser1 += NL;
    }
}//@CODE_1142


const CbString& DataModel::GetHUser2()
{//@CODE_1144
    return _hUser2;
}//@CODE_1144


void DataModel::SetHUser2(const CbString& rHUser2)
{//@CODE_1145
    _hUser2 = rHUser2;
    if (!rHUser2.IsEmpty())
    {
        if (rHUser2[rHUser2.GetLength()-1] != '\n')
            _hUser2 += NL;
    }
}//@CODE_1145


/*@NOTE_1549
Returns the value of member '_indentSize'.
*/
int DataModel::GetIndentSize()
{//@CODE_1549
    return _indentSize;
}//@CODE_1549


/*@NOTE_1550
Set the value of member '_indentSize' to 'indentSize'.
*/
void DataModel::SetIndentSize(int indentSize)
{//@CODE_1550
    _indentSize = indentSize;
}//@CODE_1550


const CbTime& DataModel::GetLastSave()
{//@CODE_1166
    return _lastSave;
}//@CODE_1166


void DataModel::SetLastSave(const CbTime& rLastSave)
{//@CODE_1167
    _lastSave = rLastSave;
}//@CODE_1167


/*@NOTE_4382
Returns the value of member '_maxLastSave'.
*/
const CbTime& DataModel::GetMaxLastSave()
{//@CODE_4382
    return _maxLastSave;
}//@CODE_4382


/*@NOTE_4383
Set the value of member '_maxLastSave' to 'rMaxLastSave'.
*/
void DataModel::SetMaxLastSave(const CbTime& maxLastSave)
{//@CODE_4383
    if (_maxLastSave < maxLastSave)
    {
        _maxLastSave = maxLastSave;
    }
}//@CODE_4383


const CbString& DataModel::GetMemberPrefix()
{//@CODE_1147
    return _memberPrefix;
}//@CODE_1147


void DataModel::SetMemberPrefix(const CbString& rMemberPrefix)
{//@CODE_1148
    if (rMemberPrefix != _memberPrefix)
    {
        // Do modification on code of methods first
        CbString oldMemberPrefix = _memberPrefix;
        _memberPrefix = rMemberPrefix;

        ClassIterator iClass(this);
        while (++iClass)
        {
            if (iClass->GetMemberPrefix() == oldMemberPrefix)
            {
                iClass->SaveState();
                iClass->SetMemberPrefix(rMemberPrefix);
                iClass->Update();
            }
        }
    }
}//@CODE_1148


/*@NOTE_4779
Returns the value of member '_name'.
*/
const CbString& DataModel::GetName()
{//@CODE_4779
    return _name;
}//@CODE_4779


/*@NOTE_4780
Set the value of member '_name' to 'rName'.
*/
void DataModel::SetName(const CbString& rName)
{//@CODE_4780
    _name = rName;
}//@CODE_4780


/*@NOTE_3346
Returns the value of member '_newClassPrefix'.
*/
const CbString& DataModel::GetNewClassPrefix()
{//@CODE_3346
    return _newClassPrefix;
}//@CODE_3346


/*@NOTE_3347
Set the value of member '_newClassPrefix' to 'rNewClassPrefix'.
*/
void DataModel::SetNewClassPrefix(const CbString& rNewClassPrefix)
{//@CODE_3347
    _newClassPrefix = rNewClassPrefix;
}//@CODE_3347


const CbString& DataModel::GetNote()
{//@CODE_1169
    return _note;
}//@CODE_1169


void DataModel::SetNote(const CbString& rNote)
{//@CODE_1170
    _note = rNote;
    if (!rNote.IsEmpty())
    {
        if (rNote[rNote.GetLength()-1] != '\n')
            _note += NL;
    }
}//@CODE_1170


/*@NOTE_23443
Set the value of member '_phaseSupport' to 'phaseSupport'.
*/
void DataModel::SetPhaseSupport(bool phaseSupport)
{//@CODE_23443
    _phaseSupport = phaseSupport;
}//@CODE_23443


/*@NOTE_1505
Returns the value of member '_privateMembers'.
*/
bool DataModel::GetPrivateMembers()
{//@CODE_1505
    return _privateMembers;
}//@CODE_1505


/*@NOTE_1506
Set the value of member '_privateMembers' to 'privateMembers'.
*/
void DataModel::SetPrivateMembers(bool privateMembers)
{//@CODE_1506
    _privateMembers = privateMembers;
}//@CODE_1506


/*@NOTE_1514
Returns the value of member '_privateMethods'.
*/
bool DataModel::GetPrivateMethods()
{//@CODE_1514
    return _privateMethods;
}//@CODE_1514


/*@NOTE_1515
Set the value of member '_privateMethods' to 'privateMethods'.
*/
void DataModel::SetPrivateMethods(bool privateMethods)
{//@CODE_1515
    _privateMethods = privateMethods;
}//@CODE_1515


/*@NOTE_1518
Returns the value of member '_protectedMembers'.
*/
bool DataModel::GetProtectedMembers()
{//@CODE_1518
    return _protectedMembers;
}//@CODE_1518


/*@NOTE_1519
Set the value of member '_protectedMembers' to 'protectedMembers'.
*/
void DataModel::SetProtectedMembers(bool protectedMembers)
{//@CODE_1519
    _protectedMembers = protectedMembers;
}//@CODE_1519


/*@NOTE_1511
Returns the value of member '_protectedMethods'.
*/
bool DataModel::GetProtectedMethods()
{//@CODE_1511
    return _protectedMethods;
}//@CODE_1511


/*@NOTE_1512
Set the value of member '_protectedMethods' to 'protectedMethods'.
*/
void DataModel::SetProtectedMethods(bool protectedMethods)
{//@CODE_1512
    _protectedMethods = protectedMethods;
}//@CODE_1512


/*@NOTE_1522
Returns the value of member '_publicMembers'.
*/
bool DataModel::GetPublicMembers()
{//@CODE_1522
    return _publicMembers;
}//@CODE_1522


/*@NOTE_1523
Set the value of member '_publicMembers' to 'publicMembers'.
*/
void DataModel::SetPublicMembers(bool publicMembers)
{//@CODE_1523
    _publicMembers = publicMembers;
}//@CODE_1523


/*@NOTE_1508
Returns the value of member '_publicMethods'.
*/
bool DataModel::GetPublicMethods()
{//@CODE_1508
    return _publicMethods;
}//@CODE_1508


/*@NOTE_1509
Set the value of member '_publicMethods' to 'publicMethods'.
*/
void DataModel::SetPublicMethods(bool publicMethods)
{//@CODE_1509
    _publicMethods = publicMethods;
}//@CODE_1509


bool DataModel::GetSerialize()
{//@CODE_1153
    return _serialize;
}//@CODE_1153


void DataModel::SetSerialize(bool serialize)
{//@CODE_1154
    if (_serialize != serialize)
    {
        if (!serialize)
        {
            DataModel::ClassIterator iClass(this);
            while (++iClass)
                iClass->SetSerialize(serialize);
        }

        _serialize = serialize;
    }
}//@CODE_1154


/*@NOTE_36255
Set the value of member '_showDllExport' to 'showDllExport'.
*/
void DataModel::SetShowDllExport(bool showDllExport)
{//@CODE_36255
    if (_showDllExport != showDllExport)
    {
        _showDllExport = showDllExport;
        
        CbViewLock lock(GetDataModelDoc());
        ClassIterator iClass(this);
        while (++iClass)
        {
            if (iClass->GetDllExport())
            {
                iClass->SaveState(1);
                iClass->Update();
            }
        }
    }
}//@CODE_36255


bool DataModel::GetStdAfx()
{//@CODE_1150
    return _stdAfx;
}//@CODE_1150


void DataModel::SetStdAfx(bool stdAfx)
{//@CODE_1151
    _stdAfx = stdAfx;
}//@CODE_1151


const CbString& DataModel::GetStyleClassHeading()
{//@CODE_1421
    return _styleClassHeading;
}//@CODE_1421


void DataModel::SetStyleClassHeading(const CbString& rStyleClassHeading)
{//@CODE_1422
    _styleClassHeading = rStyleClassHeading;
}//@CODE_1422


const CbString& DataModel::GetStyleDataModelHeading()
{//@CODE_1415
    return _styleDataModelHeading;
}//@CODE_1415


void DataModel::SetStyleDataModelHeading(const CbString& rStyleDataModelHeading)
{//@CODE_1416
    _styleDataModelHeading = rStyleDataModelHeading;
}//@CODE_1416


const CbString& DataModel::GetStyleExplanation()
{//@CODE_1471
    return _styleExplanation;
}//@CODE_1471


void DataModel::SetStyleExplanation(const CbString& rStyleExplanation)
{//@CODE_1472
    _styleExplanation = rStyleExplanation;
}//@CODE_1472


/*@NOTE_4758
Returns the value of member '_styleFigureText'.
*/
const CbString& DataModel::GetStyleFigureText()
{//@CODE_4758
    return _styleFigureText;
}//@CODE_4758


/*@NOTE_4759
Set the value of member '_styleFigureText' to 'rStyleFigureText'.
*/
void DataModel::SetStyleFigureText(const CbString& rStyleFigureText)
{//@CODE_4759
    _styleFigureText = rStyleFigureText;
}//@CODE_4759


const CbString& DataModel::GetStyleGroupHeading()
{//@CODE_1418
    return _styleGroupHeading;
}//@CODE_1418


void DataModel::SetStyleGroupHeading(const CbString& rStyleGroupHeading)
{//@CODE_1419
    _styleGroupHeading = rStyleGroupHeading;
}//@CODE_1419


const CbString& DataModel::GetStyleItem()
{//@CODE_1429
    return _styleItem;
}//@CODE_1429


void DataModel::SetStyleItem(const CbString& rStyleItem)
{//@CODE_1430
    _styleItem = rStyleItem;
}//@CODE_1430


const CbString& DataModel::GetStyleItemExplanation()
{//@CODE_1474
    return _styleItemExplanation;
}//@CODE_1474


void DataModel::SetStyleItemExplanation(const CbString& rStyleItemExplanation)
{//@CODE_1475
    _styleItemExplanation = rStyleItemExplanation;
}//@CODE_1475


const CbString& DataModel::GetStyleItemsHeading()
{//@CODE_1425
    return _styleItemsHeading;
}//@CODE_1425


void DataModel::SetStyleItemsHeading(const CbString& rStyleItemsHeading)
{//@CODE_1426
    _styleItemsHeading = rStyleItemsHeading;
}//@CODE_1426


/*@NOTE_4801
Returns the value of member '_undoRedo'.
*/
bool DataModel::GetUndoRedo()
{//@CODE_4801
    return _undoRedo;
}//@CODE_4801


/*@NOTE_4802
Set the value of member '_undoRedo' to 'undoRedo'.
*/
void DataModel::SetUndoRedo(bool undoRedo)
{//@CODE_4802
    if (_undoRedo != undoRedo)
    {
        _undoRedo = undoRedo;
        
        // We are busy with a new document, so forget all the smart stuff
        if (!GetDocument())
            return;

        if (undoRedo)
        {
            CbViewLock lock(GetDataModelDoc());
            InitUndoRedo();
            ClassGroupIterator iClassGroup(this);
            while (++iClassGroup)
                iClassGroup->Add();
            
            ClassIterator iClass(this);
            while (++iClass)
            {
                if (iClass.Get() == GetDocument())
                {
                    Class::FromRelationIterator fromRel(iClass);
                    while (++fromRel)
                        fromRel->GetFromRelation()->Add();

                    Class::ToRelationIterator toRel(iClass);
                    while (++toRel)
                        toRel->GetToRelation()->Add();

                    BaseClass::MemberIterator member(iClass);
                    while (++member)
                        member->Add();

                    BaseClass::MethodIterator method(iClass, &Method::IsDirectMethod);
                    while (++method)
                        method->Add();
                }
                else
                {
                    if (iClass->GetSerialize())
                    {
                        Method* pMethod1 = new CleanupReferencesMethod(iClass);
                        Method* pMethod2 = new RemoveReferencesMethod(iClass);
                        Method* pMethod3 = new RestoreReferencesMethod(iClass);
                        Method* pMethod4 = new SaveReferencesMethod(iClass);
                        BaseClass::MethodIterator iMethod(iClass, &Method::IsNonMacroMethod);
                        while (++iMethod)
                        {
                            if (iMethod->IsConstructorIncludeMethod() ||
                                iMethod->IsDestructorIncludeMethod())
                            {
                                MemberAndMethodGroup* pMemberAndMethodGroup = 
                                    iMethod->GetMemberAndMethodGroup();
                                if (pMemberAndMethodGroup)
                                {
                                    pMemberAndMethodGroup->AddMethodLast(pMethod1);
                                    pMemberAndMethodGroup->AddMethodLast(pMethod2);
                                    pMemberAndMethodGroup->AddMethodLast(pMethod3);
                                    pMemberAndMethodGroup->AddMethodLast(pMethod4);
                                }
                                break;
                            }
                        }
                        iMethod.Reset();
                        while (++iMethod)
                        {
                            iMethod->Add();
                            if (iMethod->IsSerializeMethod())
                            {
                                iMethod->SetAccess(PUBLIC);
                                iMethod->Update();
                            }
                        }
                    }
                    else
                    {
                        iClass->Add();
                    }
                }
            }

        }
        else
        {
            ClassIterator iClass(this);
            while (++iClass)
            {
                BaseClass::MethodIterator iMethod(iClass);
                while (++iMethod)
                {
                    if (iMethod->IsCleanupReferencesMethod() ||
                        iMethod->IsRemoveReferencesMethod() ||
                        iMethod->IsRestoreReferencesMethod() ||
                        iMethod->IsSaveReferencesMethod())
                    {
                        iMethod->Delete();
                    }
                }
            }
        }
    }
}//@CODE_4802


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5306
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void DataModel::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
}


/*@NOTE_318
Method which must be called first in a constructor
*/
void DataModel::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_ACTIVE(DataModel, DataModel, Class, Class)
    INIT_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    INIT_SINGLE_ACTIVE(DataModel, Document, Class, Document)
    INIT_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
    INIT_MULTI_OWNED_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
    INIT_MULTI_OWNED_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    INIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
}


/*@NOTE_320
Method which must be called first in a destructor
*/
void DataModel::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(DataModel, DataModel, Class, Class)
    EXIT_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    EXIT_SINGLE_ACTIVE(DataModel, Document, Class, Document)
    EXIT_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
    EXIT_MULTI_OWNED_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
    EXIT_MULTI_OWNED_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    EXIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
}


/*@NOTE_5307
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void DataModel::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    REMOVE_MULTI_OWNED_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
    REMOVE_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
    REMOVE_SINGLE_ACTIVE(DataModel, Document, Class, Document)
    REMOVE_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    REMOVE_MULTI_OWNED_ACTIVE(DataModel, DataModel, Class, Class)
    Gti::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
}


/*@NOTE_5308
Bring the current object relations into the same state as pDataModelDocObject.
*/
void DataModel::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModel* pDataModel = (DataModel*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5310
Save the state of the current object relations to pDataModelDocObject.
*/
void DataModel::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    DataModel* pDataModel = (DataModel*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
}


/*@NOTE_323
Serialize the members only to a CbObject object
*/
void DataModel::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _cppHeader;
        archive << _hFile;
        archive << _hHeader;
        archive << _hUser1;
        archive << _hUser2;
        archive << _memberPrefix;
        archive << _name;
        archive << _note;
        archive << _lastSave;
        archive << _stdAfx;
        archive << _serialize;
        archive << _styleDataModelHeading;
        archive << _styleGroupHeading;
        archive << _styleClassHeading;
        archive << _styleItemsHeading;
        archive << _styleItem;
        archive << _styleItemExplanation;
        archive << __notUsed_rtfTemplate;
        archive << __notUsed_rtfOutput;
        archive << _styleExplanation;
        archive << _publicMethods;
        archive << _protectedMethods;
        archive << _privateMethods;
        archive << _privateMembers;
        archive << _protectedMembers;
        archive << _publicMembers;
        archive << _classPrefix;
        archive << _indentSize;
        archive << __notUsed_schemaFile;
        archive << _htmlOutput;
        archive << _styleFigureText;
        archive << _undoRedo;
        archive << _namespace;
        archive << _relationMethods;
        archive << _modifiers;
        archive << _getSetMethods;
        archive << _classBuilderMethods;
        archive << _phaseSupport;
        archive << _templateClassHeaderOnly;
        archive << _includeContextDeclarations;
        archive << __notUsed_lastSaveRtfDocumentation;
        archive << _classOverview;
        archive << _relationOverview;
        archive << _includeSequenceDiagramObjects;
        archive << _includeSequenceDiagramMessages;
        archive << _showDllExport;
        archive << __notUsed_rtfDiagramFormat;
        archive << _crlf;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _cppHeader;
            archive >> _hFile;
            archive >> _hHeader;
            archive >> _hUser1;
            archive >> _hUser2;
            archive >> _memberPrefix;
            archive >> _name;
            archive >> _note;
            archive >> _lastSave;
            archive >> _stdAfx;
            archive >> _serialize;
            archive >> _styleDataModelHeading;
            archive >> _styleGroupHeading;
            archive >> _styleClassHeading;
            archive >> _styleItemsHeading;
            archive >> _styleItem;
            archive >> _styleItemExplanation;
            archive >> __notUsed_rtfTemplate;
            archive >> __notUsed_rtfOutput;
            archive >> _styleExplanation;
            archive >> _publicMethods;
            archive >> _protectedMethods;
            archive >> _privateMethods;
            archive >> _privateMembers;
            archive >> _protectedMembers;
            archive >> _publicMembers;
            archive >> _classPrefix;
            archive >> _indentSize;
            archive >> __notUsed_schemaFile;
            archive >> _htmlOutput;
            archive >> _styleFigureText;
            archive >> _undoRedo;
            archive >> _namespace;
            archive >> _relationMethods;
            archive >> _modifiers;
            archive >> _getSetMethods;
            archive >> _classBuilderMethods;
            archive >> _phaseSupport;
            archive >> _templateClassHeaderOnly;
            archive >> _includeContextDeclarations;
            archive >> __notUsed_lastSaveRtfDocumentation;
            archive >> _classOverview;
            archive >> _relationOverview;
            archive >> _includeSequenceDiagramObjects;
            archive >> _includeSequenceDiagramMessages;
            archive >> _showDllExport;
            archive >> __notUsed_rtfDiagramFormat;
            archive >> _crlf;
        }
    }
}


/*@NOTE_322
Method which must be called first in a serialize constructor
*/
void DataModel::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(DataModel, DataModel, Class, Class)
    INIT_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
    INIT_SINGLE_ACTIVE(DataModel, Document, Class, Document)
    INIT_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
    INIT_MULTI_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
    INIT_MULTI_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    INIT_SINGLE_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)
}


/*@NOTE_325
Serialize the relations to a CbObject object
*/
void DataModel::SerializeRelations(CbArchive& archive,
                                   DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(DataModel, DataModel, Class, Class)
        WRITE_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
        WRITE_SINGLE_ACTIVE(DataModel, Document, Class, Document)
        WRITE_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
        WRITE_MULTI_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
        WRITE_MULTI_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(DataModel, DataModel, Class, Class)
            READ_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
            READ_SINGLE_ACTIVE(DataModel, Document, Class, Document)
            READ_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
            READ_MULTI_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
            READ_MULTI_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(DataModel)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(DataModel, DataModel, Class, Class)
METHODS_ITERATOR_MULTI_ACTIVE(DataModel, DataModel, Class, Class)
METHODS_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
METHODS_ITERATOR_MULTI_ACTIVE(DataModel, DataModel, ClassGroup, ClassGroup)
METHODS_SINGLE_ACTIVE(DataModel, Document, Class, Document)
METHODS_SINGLE_ACTIVE(DataModel, DocumentObject, Class, DocumentObject)
METHODS_MULTI_OWNED_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
METHODS_ITERATOR_MULTI_ACTIVE(DataModel, DataModel, ContextDeclaration, ContextDeclaration)
METHODS_MULTI_OWNED_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
METHODS_ITERATOR_MULTI_ACTIVE(DataModel, DataModel, MetaGroup, MetaGroup)
METHODS_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModel, DataModel)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
