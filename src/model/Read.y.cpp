/* flex/bison skeletons still emit the 'register' storage class, removed in
   C++17 (MSVC C5033, GCC/Clang -Wregister). It is generator boilerplate, not
   our code, so silence it for the whole translation unit. Keep this in sync
   with the matching block in the Read.y prologue (this file is regenerated). */
#if defined(_MSC_VER)
#pragma warning(disable : 5033)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wregister"
#endif

#ifndef lint
char readsccsid[] = "@(#)yaccpar	1.4 (Berkeley) 02/25/90";
#endif
#line 2 "Read.y"
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

extern char* readtext;
extern int readlineno;
extern FILE* readin;
extern int readleng;
extern int readindex;
extern char readline[];
extern char readlineUser3[];
extern char readinterface[];

extern int readparse();
extern int readlex();

static int errorCount = 0;

/* Some own defines */
#define CHECK_ERROR		if (errorCount > 50) {\
    CbMessageBox("To many errors", CBMB_ICONSTOP);\
	READABORT; }

#define RECOVER_ERROR	readerrok; CHECK_ERROR
#line 58 "Read.y"
typedef union {
    int ival;
	double dval;
	char* sval;
} READSTYPE;
#line 66 "ytab.c"
#define LINE 257
#define START_COMMENT 258
#define END_COMMENT 259
#define START_USER1 260
#define END_USER1 261
#define START_USER2 262
#define END_USER2 263
#define START_USER3 264
#define END_USER3 265
#define START_DECLARATION 266
#define END_DECLARATION 267
#define START_NOTE 268
#define END_NOTE 269
#define START_INIT 270
#define START_CODE 271
#define END_CODE 272
#define READERRCODE 256
short readlhs[] = {                                        -1,
    0,    1,    1,    1,    2,    2,    2,    2,    2,    2,
    2,    2,   12,    3,   13,    4,   14,    5,   15,    6,
   16,   16,   17,    7,   18,    8,   19,   20,    9,   21,
   10,   11,   11,   11,   22,
};
short readlen[] = {                                         2,
    1,    1,    2,    2,    1,    1,    1,    1,    1,    1,
    1,    1,    0,    4,    0,    4,    0,    4,    0,    4,
    0,    1,    0,    4,    0,    4,    0,    0,    7,    0,
    4,    0,    2,    2,    1,
};
short readdefred[] = {                                      0,
   13,   15,   17,   19,   23,   25,   27,   30,    0,    0,
    2,    5,    6,    7,    8,    9,   10,   11,   12,   32,
   32,   32,   32,   32,   32,   32,   32,    4,    3,    0,
    0,    0,    0,    0,    0,    0,    0,   34,   35,   14,
   33,   16,   18,   22,   20,   24,   26,   28,   31,   32,
    0,   29,
};
short readdgoto[] = {                                       9,
   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
   30,   20,   21,   22,   23,   45,   24,   25,   26,   50,
   27,   41,
};
short readsindex[] = {                                   -222,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -242,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -186,
 -189, -197, -200, -203, -206, -224, -247,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -245,    0,
};
short readrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   13,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    1,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
short readgindex[] = {                                      0,
    0,  -10,    0,    0,    0,    0,    0,    0,    0,    0,
  -19,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#define READTABLESIZE 272
short readtable[] = {                                      29,
   21,   31,   32,   33,   34,   35,   36,   37,   38,   39,
   38,   39,    1,   28,    0,    1,    0,    2,    0,    3,
    0,    4,    0,    5,   49,    6,   52,    7,    8,    0,
   51,   38,   39,    0,    0,    1,    0,    2,    0,    3,
    0,    4,    0,    5,    0,    6,   48,    7,    8,   38,
   39,    0,   38,   39,    0,   38,   39,    0,   38,   39,
    0,    0,   47,   46,   44,   43,   38,   39,    0,   38,
   39,   42,   40,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   21,    0,
   21,    0,   21,    0,   21,    0,   21,    0,   21,    0,
   21,   21,
};
short readcheck[] = {                                      10,
    0,   21,   22,   23,   24,   25,   26,   27,  256,  257,
  256,  257,    0,  256,   -1,  258,   -1,  260,   -1,  262,
   -1,  264,   -1,  266,  272,  268,  272,  270,  271,   -1,
   50,  256,  257,   -1,   -1,  258,   -1,  260,   -1,  262,
   -1,  264,   -1,  266,   -1,  268,  271,  270,  271,  256,
  257,   -1,  256,  257,   -1,  256,  257,   -1,  256,  257,
   -1,   -1,  269,  267,  265,  263,  256,  257,   -1,  256,
  257,  261,  259,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  258,   -1,
  260,   -1,  262,   -1,  264,   -1,  266,   -1,  268,   -1,
  270,  271,
};
#define READFINAL 9
#ifndef READDEBUG
#define READDEBUG 0
#endif
#define READMAXTOKEN 272
#if READDEBUG
char *readname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"LINE","START_COMMENT",
"END_COMMENT","START_USER1","END_USER1","START_USER2","END_USER2","START_USER3",
"END_USER3","START_DECLARATION","END_DECLARATION","START_NOTE","END_NOTE",
"START_INIT","START_CODE","END_CODE",
};
char *readrule[] = {
"$accept : source_text",
"source_text : item_LIST",
"item_LIST : item",
"item_LIST : item_LIST item",
"item_LIST : item_LIST error",
"item : comment",
"item : user1",
"item : user2",
"item : user3",
"item : declaration",
"item : note",
"item : init",
"item : code",
"$$1 :",
"comment : START_COMMENT $$1 line_LIST END_COMMENT",
"$$2 :",
"user1 : START_USER1 $$2 line_LIST END_USER1",
"$$3 :",
"user2 : START_USER2 $$3 line_LIST END_USER2",
"$$4 :",
"user3 : START_USER3 $$4 line_LIST endUser3_OPT",
"endUser3_OPT :",
"endUser3_OPT : END_USER3",
"$$5 :",
"declaration : START_DECLARATION $$5 line_LIST END_DECLARATION",
"$$6 :",
"note : START_NOTE $$6 line_LIST END_NOTE",
"$$7 :",
"$$8 :",
"init : START_INIT $$7 line_LIST START_CODE $$8 line_LIST END_CODE",
"$$9 :",
"code : START_CODE $$9 line_LIST END_CODE",
"line_LIST :",
"line_LIST : line_LIST line",
"line_LIST : line_LIST error",
"line : LINE",
};
#endif
#define readclearin (readchar=(-1))
#define readerrok (readerrflag=0)
#ifndef READSTACKSIZE
#ifdef READMAXDEPTH
#define READSTACKSIZE READMAXDEPTH
#else
#define READSTACKSIZE 300
#endif
#endif
int readdebug;
int readnerrs;
int readerrflag;
int readchar;
short *readssp;
READSTYPE *readvsp;
READSTYPE readval;
READSTYPE readlval;
#define readstacksize READSTACKSIZE
short readss[READSTACKSIZE];
READSTYPE readvs[READSTACKSIZE];
#line 194 "Read.y"
void readerror (const char* str)
{
    CbString error;
    error.Format("! Error during parsing at line %d error '%s'", readlineno, str);
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
				CbString str(readinterface);

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
                str.Format("    Update note method '%s'", pMethod->GetName());
                g_pDialog->AddLog(str);
            }
        }
    }
}

void CheckInit()
{
	// Code segment, does not start at beginning of line
	if (readindex)
	{
		g_init += readline;
		readindex = 0;
	}
    Constructor* pConstructor = dynamic_cast<Constructor*>(g_pClass->FindMethodWithId(g_id));
    if (pConstructor && pConstructor->GetInit() != g_init)
    {
        pConstructor->SaveState();
        pConstructor->SetInit(g_init);
        pConstructor->SetVersion(g_newVersion);
        CbString str;
        str.Format("    Update init part of constructor '%s'", pConstructor->GetName());
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
        str.Format("    Update code of method '%s'", pMethod->GetName());
        g_pDialog->AddLog(str);
    }

    int index = g_code.Find("//@CODE_");
    if (index != -1 && (index+8) < g_code.GetLength() && isdigit(g_code.GetAt(index+8)))
    {
        int lineno = readlineno+1;
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
		warning.Format("! Warning at line %d: Found unexpected tag '%s'", lineno, g_code.Mid(start, end-start));
		g_pDialog->AddLogWarning(warning);
    }
}

void CheckInterface()
{
    Method* pMethod = g_pClass->FindMethodWithId(g_id);
    if (pMethod)
    {
		CbString str(readinterface);
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
			text.Format("    Warning interface of method '%s' has changed", pMethod->GetName());
			g_pDialog->AddLogWarning(text);
		}
    }
}

static void CheckId(UINT id)
{
	if (id != g_id)
	{
		CbString error;
		error.Format("! Error at line %d: Expected '//@CODE_%d' instead of '//@CODE_%d'", readlineno, g_id, id);
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
						text.Format("    Update initialization of static member '%s'", iMember->GetPrefixedName());
						g_pDialog->AddLog(text);
					}
					initStaticMembers = initStaticMembers.Mid(index);
				}
				else
				{
					CbString text;
					text.Format("    Warning static member '%s' is not properly initialized", iMember->GetPrefixedName());
					g_pDialog->AddLogWarning(text);
				}
			}
			else
			{
				CbString text;
				text.Format("    Warning static member '%s' not found", iMember->GetPrefixedName());
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
        str.Format("! Error: can not open file '%s' for reading", pDataModel->GetHFile());
        pDialog->AddLogError(str);
		return 0;
	}

    str.Format("- Reading file '%s'", pDataModel->GetHFile());
    pDialog->AddLog(str);

    g_commentTmp.Empty();
    g_comment.Empty();
    g_user1.Empty();
    g_user2.Empty();
    g_user3.Empty();

    errorCount = 0;
    readin = fp;
    int status = readparse();
    
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
        str.Format("! Error: can not open file '%s' for reading", pClass->GetHFile());
        pDialog->AddLogError(str);
		return 0;
	}

    str.Format("- Reading file '%s'", pClass->GetHFile());
    pDialog->AddLog(str);

    g_commentTmp.Empty();
    g_comment.Empty();
    g_user1.Empty();
    g_user2.Empty();
    g_user3.Empty();

    errorCount = 0;
    readin = fp;
    int status = readparse();
    
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
        str.Format("! Error: can not open file '%s' for reading", pClass->GetCppFile());
        pDialog->AddLogError(str);
		return 0;
	}

    str.Format("- Reading file '%s'", pClass->GetCppFile());
    pDialog->AddLog(str);

    g_commentTmp.Empty();
    g_comment.Empty();
    g_user1.Empty();
    g_user2.Empty();
    g_user3.Empty();

    errorCount = 0;
    readin = fp;
    int status = readparse();
    
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
	if (readlineUser3[0] != '\0')
	{
		g_user3 += readlineUser3;
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
#line 707 "ytab.c"
#define READABORT goto readabort
#define READACCEPT goto readaccept
#define READERROR goto readerrlab
int
readparse()
{
    register int readm, readn, readstate;
#if READDEBUG
    register char *reads;
    extern char *getenv();

    if (reads = getenv("READDEBUG"))
    {
        readn = *reads;
        if (readn >= '0' && readn <= '9')
            readdebug = readn - '0';
    }
#endif

    readnerrs = 0;
    readerrflag = 0;
    readchar = (-1);

    readssp = readss;
    readvsp = readvs;
    *readssp = readstate = 0;

readloop:
    if (readn = readdefred[readstate]) goto readreduce;
    if (readchar < 0)
    {
        if ((readchar = readlex()) < 0) readchar = 0;
#if READDEBUG
        if (readdebug)
        {
            reads = 0;
            if (readchar <= READMAXTOKEN) reads = readname[readchar];
            if (!reads) reads = "illegal-symbol";
            printf("readdebug: state %d, reading %d (%s)\n", readstate,
                    readchar, reads);
        }
#endif
    }
    if ((readn = readsindex[readstate]) && (readn += readchar) >= 0 &&
            readn <= READTABLESIZE && readcheck[readn] == readchar)
    {
#if READDEBUG
        if (readdebug)
            printf("readdebug: state %d, shifting to state %d\n",
                    readstate, readtable[readn]);
#endif
        if (readssp >= readss + readstacksize - 1)
        {
            goto readoverflow;
        }
        *++readssp = readstate = readtable[readn];
        *++readvsp = readlval;
        readchar = (-1);
        if (readerrflag > 0)  --readerrflag;
        goto readloop;
    }
    if ((readn = readrindex[readstate]) && (readn += readchar) >= 0 &&
            readn <= READTABLESIZE && readcheck[readn] == readchar)
    {
        readn = readtable[readn];
        goto readreduce;
    }
    if (readerrflag) goto readinrecovery;
#ifdef lint
    goto readnewerror;
#endif
readnewerror:
    readerror("syntax error");
#ifdef lint
    goto readerrlab;
#endif
readerrlab:
    ++readnerrs;
readinrecovery:
    if (readerrflag < 3)
    {
        readerrflag = 3;
        for (;;)
        {
            if ((readn = readsindex[*readssp]) && (readn += READERRCODE) >= 0 &&
                    readn <= READTABLESIZE && readcheck[readn] == READERRCODE)
            {
#if READDEBUG
                if (readdebug)
                    printf("readdebug: state %d, error recovery shifting\
 to state %d\n", *readssp, readtable[readn]);
#endif
                if (readssp >= readss + readstacksize - 1)
                {
                    goto readoverflow;
                }
                *++readssp = readstate = readtable[readn];
                *++readvsp = readlval;
                goto readloop;
            }
            else
            {
#if READDEBUG
                if (readdebug)
                    printf("readdebug: error recovery discarding state %d\n",
                            *readssp);
#endif
                if (readssp <= readss) goto readabort;
                --readssp;
                --readvsp;
            }
        }
    }
    else
    {
        if (readchar == 0) goto readabort;
#if READDEBUG
        if (readdebug)
        {
            reads = 0;
            if (readchar <= READMAXTOKEN) reads = readname[readchar];
            if (!reads) reads = "illegal-symbol";
            printf("readdebug: state %d, error recovery discards token %d (%s)\n",
                    readstate, readchar, reads);
        }
#endif
        readchar = (-1);
        goto readloop;
    }
readreduce:
#if READDEBUG
    if (readdebug)
        printf("readdebug: state %d, reducing by rule %d (%s)\n",
                readstate, readn, readrule[readn]);
#endif
    readm = readlen[readn];
    readval = readvsp[1-readm];
    switch (readn)
    {
case 1:
#line 83 "Read.y"
{
									if (errorCount)
										READABORT;
								}
break;
case 3:
#line 93 "Read.y"
{ RECOVER_ERROR; }
break;
case 4:
#line 97 "Read.y"
{ CHECK_ERROR; }
break;
case 13:
#line 119 "Read.y"
{ g_pLines = &g_commentTmp; }
break;
case 14:
#line 121 "Read.y"
{
            g_commentTmp += readvsp[0].sval ; 
            if (g_comment.IsEmpty())
                g_comment = g_commentTmp;
        }
break;
case 15:
#line 129 "Read.y"
{ g_pLines = &g_user1; }
break;
case 17:
#line 135 "Read.y"
{ g_pLines = &g_user2; }
break;
case 19:
#line 141 "Read.y"
{ g_pLines = &g_user3; }
break;
case 23:
#line 153 "Read.y"
{ g_pLines = &g_declaration; g_id = readvsp[0].ival ; }
break;
case 24:
#line 155 "Read.y"
{ CheckDeclaration(); }
break;
case 25:
#line 159 "Read.y"
{ g_pLines = &g_note; g_id = readvsp[0].ival ; }
break;
case 26:
#line 161 "Read.y"
{ CheckNote();}
break;
case 27:
#line 165 "Read.y"
{ g_pLines = &g_init; g_id = readvsp[0].ival ; }
break;
case 28:
#line 167 "Read.y"
{ g_pLines = &g_code; CheckId(readvsp[0].ival ); CheckInit(); g_id = readvsp[0].ival ; CheckInterface(); }
break;
case 29:
#line 169 "Read.y"
{ CheckId(readvsp[0].ival ); CheckCode(); }
break;
case 30:
#line 173 "Read.y"
{ g_pLines = &g_code; g_id = readvsp[0].ival ; CheckInterface(); }
break;
case 31:
#line 175 "Read.y"
{ CheckId(readvsp[0].ival ); CheckCode(); }
break;
case 32:
#line 179 "Read.y"
{ g_pLines->Empty(); }
break;
case 33:
#line 182 "Read.y"
{ RECOVER_ERROR; }
break;
case 34:
#line 186 "Read.y"
{ CHECK_ERROR; }
break;
case 35:
#line 190 "Read.y"
{ (*g_pLines) += readvsp[0].sval ; }
break;
#line 938 "ytab.c"
    }
    readssp -= readm;
    readstate = *readssp;
    readvsp -= readm;
    readm = readlhs[readn];
    if (readstate == 0 && readm == 0)
    {
#ifdef READDEBUG
        if (readdebug)
            printf("readdebug: after reduction, shifting from state 0 to\
 state %d\n", READFINAL);
#endif
        readstate = READFINAL;
        *++readssp = READFINAL;
        *++readvsp = readval;
        if (readchar < 0)
        {
            if ((readchar = readlex()) < 0) readchar = 0;
#if READDEBUG
            if (readdebug)
            {
                reads = 0;
                if (readchar <= READMAXTOKEN) reads = readname[readchar];
                if (!reads) reads = "illegal-symbol";
                printf("readdebug: state %d, reading %d (%s)\n",
                        READFINAL, readchar, reads);
            }
#endif
        }
        if (readchar == 0) goto readaccept;
        goto readloop;
    }
    if ((readn = readgindex[readm]) && (readn += readstate) >= 0 &&
            readn <= READTABLESIZE && readcheck[readn] == readstate)
        readstate = readtable[readn];
    else
        readstate = readdgoto[readm];
#ifdef READDEBUG
    if (readdebug)
        printf("readdebug: after reduction, shifting from state %d \
to state %d\n", *readssp, readstate);
#endif
    if (readssp >= readss + readstacksize - 1)
    {
        goto readoverflow;
    }
    *++readssp = readstate;
    *++readvsp = readval;
    goto readloop;
readoverflow:
    readerror("yacc stack overflow");
readabort:
    return (1);
readaccept:
    return (0);
}
