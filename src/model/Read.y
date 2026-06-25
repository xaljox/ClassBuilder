%{
/* flex/bison skeletons still emit the 'register' storage class, removed in
   C++17 (MSVC C5033, GCC/Clang -Wregister). It is generator boilerplate, not
   our code, so silence it for the whole translation unit. Bison copies this
   prologue into Read.y.cpp; the same block is mirrored at the top of the
   generated file so the checked-in build stays clean without regenerating. */
#if defined(_MSC_VER)
#pragma warning(disable : 5033)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wregister"
#endif

#include "StdAfx.h"
#include <stdlib.h>
#include <stdio.h>

#include "ParseLogInterface.h"
#include "CbStringBuilder.h"


static CbStringBuilder g_commentTmp;
static CbStringBuilder g_comment;
static CbStringBuilder g_user1;
static CbStringBuilder g_user2;
static CbStringBuilder g_user3(1<<15, 1<<12); 
static CbStringBuilder g_declaration;
static CbStringBuilder g_note;
static CbStringBuilder g_init;
static CbStringBuilder g_code;
static UINT g_id;
static CbStringBuilder* g_pLines;

static ParseLogInterface* g_pDialog;
static DataModel* g_pDataModel;
static Class* g_pClass;

static int g_newVersion;

static void CheckDeclaration();
static void CheckNote();
static void CheckInit();
static void CheckInitStaticMembers(CbString initStaticMembers);
static void CheckCode();
static void CheckId(UINT id);
static void CheckInterface();

extern char* yytext;
extern int yylineno;
extern FILE* yyin;
extern int yyleng;
extern int yyindex;
extern char yyline[];
extern char yylineUser3[];
extern char yyinterface[];

extern int yyparse();
extern int yylex();

static int errorCount = 0;

/* Some own defines */
#define CHECK_ERROR		if (errorCount > 50) {\
    CbMessageBox("To many errors", CBMB_ICONSTOP);\
	YYABORT; }

#define RECOVER_ERROR	yyerrok; CHECK_ERROR
%}

%union {
    int ival;
	double dval;
	char* sval;
}

%token <sval> LINE
%token START_COMMENT
%token <sval> END_COMMENT
%token START_USER1
%token END_USER1
%token START_USER2
%token END_USER2
%token START_USER3
%token END_USER3
%token <ival> START_DECLARATION
%token <ival> END_DECLARATION
%token <ival> START_NOTE
%token <sval> END_NOTE
%token <ival> START_INIT
%token <ival> START_CODE
%token <ival> END_CODE

%%
source_text
      : item_LIST				{
									if (errorCount)
										YYABORT;
								}
;

item_LIST
      : item

	  | item_LIST
	    item					{ RECOVER_ERROR; }

	  /* Error recovery */
	  | item_LIST
	    error					{ CHECK_ERROR; }
;

item
      : comment

	  | user1

	  | user2

	  | user3

	  | declaration

	  | note

	  | init

	  | code
;

comment
      : START_COMMENT           { g_pLines = &g_commentTmp; }
        line_LIST
        END_COMMENT {
            g_commentTmp += $4; 
            if (g_comment.IsEmpty())
                g_comment = g_commentTmp;
        }
;

user1
      : START_USER1             { g_pLines = &g_user1; }
        line_LIST
        END_USER1
;

user2
      : START_USER2             { g_pLines = &g_user2; }
        line_LIST
        END_USER2
;

user3
      : START_USER3             { g_pLines = &g_user3; }
        line_LIST
        endUser3_OPT
;

endUser3_OPT
      : /* EMPTY */
	  
	  | END_USER3 
;

declaration
      : START_DECLARATION       { g_pLines = &g_declaration; g_id = $1; }
        line_LIST
        END_DECLARATION			{ CheckDeclaration(); }
;

note
      : START_NOTE              { g_pLines = &g_note; g_id = $1; }
        line_LIST
        END_NOTE                { CheckNote();}
;

init
      : START_INIT              { g_pLines = &g_init; g_id = $1; }
        line_LIST
        START_CODE              { g_pLines = &g_code; CheckId($4); CheckInit(); g_id = $4; CheckInterface(); }
        line_LIST
        END_CODE                { CheckId($7); CheckCode(); }
;

code
      : START_CODE              { g_pLines = &g_code; g_id = $1; CheckInterface(); }
        line_LIST
        END_CODE                { CheckId($4); CheckCode(); }
;

line_LIST
      : /* EMPTY */             { g_pLines->Empty(); }

	  | line_LIST
	    line					{ RECOVER_ERROR; }

	  /* Error recovery */
	  | line_LIST
	    error					{ CHECK_ERROR; }
;

line
      : LINE                    { (*g_pLines) += $1; }
;

%%
void yyerror (const char* str)
{
    CbString error;
    error.Format("! Error during parsing at line %d error '%s'", yylineno, str);
    g_pDialog->AddLogError(error);
}

void CheckDeclaration()
{
    if (g_pDataModel)
    {
		OtherType* pOtherType = dynamic_cast<OtherType*>
		    (g_pDataModel->GetDataModelDoc()->FindDataModelDocObject(g_id));
		if (pOtherType)
		{
			if (pOtherType->GetDeclaration() != g_declaration)
			{
				pOtherType->SaveState();
                pOtherType->SetDeclaration(g_declaration);
                pOtherType->SetVersion(g_newVersion);
                g_pDialog->AddLog("    Update type declaration");
			}
		}
    }
}

void CheckNote()
{
    if (g_pClass)
    {
        if (g_id == g_pClass->GetId())
        {
            if (g_pClass->GetNote() != g_note)
            {
                g_pClass->SaveState();
                g_pClass->SetNote(g_note);
                g_pClass->SetVersion(g_newVersion);
                g_pDialog->AddLog("    Update class note");
                g_pClass->AddModified("@Update class note");
            }
        }
        else
        {
            Method* pMethod = g_pClass->FindMethodWithId(g_id);

			if (pMethod)
			{
				CbString str(yyinterface);

				int index = str.Find("// Static members");
				if (index != -1)
				{
					str = str.Mid(index+17);
					index = str.Find(NL NL NL);
					if (index != -1)
					{
						CheckInitStaticMembers(str.Left(index));
					}
				}
			}

            if (pMethod && pMethod->GetNote() != g_note)
            {
                pMethod->SaveState();
                pMethod->SetNote(g_note);
                pMethod->SetVersion(g_newVersion);
                CbString str;
                str.Format("    Update note method '%s'", pMethod->GetName().c_str());
                g_pDialog->AddLog(str);
            }
        }
    }
}

void CheckInit()
{
	// Code segment, does not start at beginning of line
	if (yyindex)
	{
		g_init += yyline;
		yyindex = 0;
	}
    Constructor* pConstructor = dynamic_cast<Constructor*>(g_pClass->FindMethodWithId(g_id));
    if (pConstructor && pConstructor->GetInit() != g_init)
    {
        pConstructor->SaveState();
        pConstructor->SetInit(g_init);
        pConstructor->SetVersion(g_newVersion);
        CbString str;
        str.Format("    Update init part of constructor '%s'", pConstructor->GetName().c_str());
        g_pDialog->AddLog(str);
    }
}

void CheckCode()
{
    Method* pMethod = g_pClass->FindMethodWithId(g_id);
    if (pMethod && pMethod->GetCode() != g_code)
    {
        pMethod->SaveState();
        pMethod->SetCode(g_code);
        pMethod->SetVersion(g_newVersion);
        CbString str;
        str.Format("    Update code of method '%s'", pMethod->GetName().c_str());
        g_pDialog->AddLog(str);
    }

    int index = g_code.Find("//@CODE_");
    if (index != -1 && (index+8) < g_code.GetLength() && isdigit(g_code.GetAt(index+8)))
    {
        int lineno = yylineno+1;
        int start = index;
        int end = index+8;

        while (isdigit(g_code.GetAt(end)))
        {
            end++;
        }

        while ((index = g_code.Find(NL, index)) != -1)
        {
            index++;
            lineno--;
        }

		CbString warning;
		warning.Format("! Warning at line %d: Found unexpected tag '%s'", lineno, g_code.Mid(start, end-start).c_str());
		g_pDialog->AddLogWarning(warning);
    }
}

void CheckInterface()
{
    Method* pMethod = g_pClass->FindMethodWithId(g_id);
    if (pMethod)
    {
		CbString str(yyinterface);
		str.TrimLeft();
		str.TrimRight();
		
		CbString ref(WrapArguments(pMethod->GetInterfaceCpp()));

		int index = ref.Find(" //@INIT_");
		if (index != -1)
		{
			ref = ref.Left(index);
		}
		ref.TrimLeft();
		ref.TrimRight();

		if (str.Find(ref) == -1)
		{
			CbString text;
			text.Format("    Warning interface of method '%s' has changed", pMethod->GetName().c_str());
			g_pDialog->AddLogWarning(text);
		}
    }
}

static void CheckId(UINT id)
{
	if (id != g_id)
	{
		CbString error;
		error.Format("! Error at line %d: Expected '//@CODE_%d' instead of '//@CODE_%d'", yylineno, g_id, id);
		g_pDialog->AddLogError(error);
	}
}

static void CheckInitStaticMembers(CbString initStaticMembers)
{
	if (!initStaticMembers.IsEmpty())
	{
		BaseClass::MemberIterator iMember(g_pClass, &Member::GetStatic);
		while (++iMember)
		{
			CbString str;
			str += iMember->GetTypeName() + g_pClass->GetName() + "::" 
				+ g_pClass->GetMemberPrefix() + iMember->GetVariableName();

			int index = initStaticMembers.Find(str);
			if (index != -1)
			{
				initStaticMembers = 
				    initStaticMembers.Mid(index+str.GetLength());
				initStaticMembers.TrimLeft();
				if (initStaticMembers.GetAt(0) == '=')
				{
					if (initStaticMembers.GetAt(1) == ' ')
						initStaticMembers = initStaticMembers.Mid(2);
					else
						initStaticMembers = initStaticMembers.Mid(1);
					index = initStaticMembers.Find(";");
				}
				else if (initStaticMembers.GetAt(0) == '(')
				{
					initStaticMembers = initStaticMembers.Mid(1);
					index = initStaticMembers.Find(");");
				}
				else if (initStaticMembers.GetAt(0) == ';')
				{
					index = 0;
				}
				if (index != -1)
				{
					CbString init = initStaticMembers.Left(index);

					if (init != iMember->GetInitialization())
					{
						iMember->SaveState();
						iMember->SetInitialization(init);

						CbString text;
						text.Format("    Update initialization of static member '%s'", iMember->GetPrefixedName().c_str());
						g_pDialog->AddLog(text);
					}
					initStaticMembers = initStaticMembers.Mid(index);
				}
				else
				{
					CbString text;
					text.Format("    Warning static member '%s' is not properly initialized", iMember->GetPrefixedName().c_str());
					g_pDialog->AddLogWarning(text);
				}
			}
			else
			{
				CbString text;
				text.Format("    Warning static member '%s' not found", iMember->GetPrefixedName().c_str());
				g_pDialog->AddLogWarning(text);
			}
		}
	}
}

int ReadHSource(ParseLogInterface* pDialog, DataModel* pDataModel)
{
    g_pDialog = pDialog;
    g_pDataModel = pDataModel;
    g_pClass = 0;
    g_newVersion = g_pDataModel->GetDataModelDoc()->GetVersion() + 1;

    CbString str;
    FILE* fp;
	if ((fp = fopen(pDataModel->GetHFile(), "r")) == NULL)
	{
        str.Format("! Error: can not open file '%s' for reading", pDataModel->GetHFile().c_str());
        pDialog->AddLogError(str);
		return 0;
	}

    str.Format("- Reading file '%s'", pDataModel->GetHFile().c_str());
    pDialog->AddLog(str);

    g_commentTmp.Empty();
    g_comment.Empty();
    g_user1.Empty();
    g_user2.Empty();
    g_user3.Empty();

    errorCount = 0;
    yyin = fp;
    int status = yyparse();
    
    fclose(fp);
    
    if (status)
    {
        return 0;
    }
    
    if (pDataModel->GetHUser1() != g_user1)
    {
		pDataModel->SaveState();
        pDataModel->SetHUser1(g_user1);
        pDataModel->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user1");
    }
    
    if (pDataModel->GetHUser2() != g_user2)
    {
		pDataModel->SaveState();
        pDataModel->SetHUser2(g_user2);
        pDataModel->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user2");
    }

	return 1;
}

int ReadHSource(ParseLogInterface* pDialog, Class* pClass)
{
    g_pDialog = pDialog;
    g_pDataModel = pClass->GetDataModel();
    g_pClass = pClass;
    g_newVersion = g_pDataModel->GetDataModelDoc()->GetVersion() + 1;

    CbString str;
    FILE* fp;
	if ((fp = fopen(pClass->GetHFile(), "r")) == NULL)
	{
        str.Format("! Error: can not open file '%s' for reading", pClass->GetHFile().c_str());
        pDialog->AddLogError(str);
		return 0;
	}

    str.Format("- Reading file '%s'", pClass->GetHFile().c_str());
    pDialog->AddLog(str);

    g_commentTmp.Empty();
    g_comment.Empty();
    g_user1.Empty();
    g_user2.Empty();
    g_user3.Empty();

    errorCount = 0;
    yyin = fp;
    int status = yyparse();
    
    fclose(fp);
    
    if (status)
    {
        return 0;
    }
    
    if (pClass->GetHHeader() != g_comment)
    {
		pClass->SaveState();
        pClass->SetHHeader(g_comment);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update comment header");
        pClass->AddModified("@Update comment header");
    }
    
    if (pClass->GetHUser1() != g_user1)
    {
		pClass->SaveState();
        pClass->SetHUser1(g_user1);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user1");
    }
    
    if (pClass->GetHUser2() != g_user2)
    {
		pClass->SaveState();
        pClass->SetHUser2(g_user2);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user2");
    }
    
    if (pClass->GetHUser3() != g_user3)
    {
		pClass->SaveState();
        pClass->SetHUser3(g_user3);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user3");
    }

	return 1;
}

int ReadCppSource(ParseLogInterface* pDialog, Class* pClass)
{
    g_pDialog = pDialog;
    g_pDataModel = pClass->GetDataModel();
    g_pClass = pClass;
    g_newVersion = g_pDataModel->GetDataModelDoc()->GetVersion() + 1;

    CbString str;
    FILE* fp;
	if ((fp = fopen(pClass->GetCppFile(), "r")) == NULL)
	{
        str.Format("! Error: can not open file '%s' for reading", pClass->GetCppFile().c_str());
        pDialog->AddLogError(str);
		return 0;
	}

    str.Format("- Reading file '%s'", pClass->GetCppFile().c_str());
    pDialog->AddLog(str);

    g_commentTmp.Empty();
    g_comment.Empty();
    g_user1.Empty();
    g_user2.Empty();
    g_user3.Empty();

    errorCount = 0;
    yyin = fp;
    int status = yyparse();
    
    fclose(fp);
    
    if (status)
    {
        return 0;
    }
    
    if (pClass->GetCppHeader() != g_comment)
    {
		pClass->SaveState();
        pClass->SetCppHeader(g_comment);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update comment header");
    }
    
    if (pClass->GetCppUser1() != g_user1)
    {
		pClass->SaveState();
        pClass->SetCppUser1(g_user1);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user1");
    }
    
    if (pClass->GetCppUser2() != g_user2)
    {
		pClass->SaveState();
        pClass->SetCppUser2(g_user2);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user2");
    }

    // Some unfinished line on stack for User3 code block
	if (yylineUser3[0] != '\0')
	{
		g_user3 += yylineUser3;
	}

    if (pClass->GetCppUser3() != g_user3)
    {
		pClass->SaveState();
        pClass->SetCppUser3(g_user3);
        pClass->SetVersion(g_newVersion);
        pDialog->AddLog("    Update code block user3");
    }

	return 1;
}
