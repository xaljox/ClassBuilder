%{
/* flex/bison skeletons still emit the 'register' storage class, removed in
   C++17 (MSVC C5033, GCC/Clang -Wregister). It is generator boilerplate, not
   our code, so silence it for the whole translation unit. Flex copies this
   prologue into Read.l.cpp; the same block is mirrored at the top of the
   generated file so the checked-in build stays clean without regenerating. */
#if defined(_MSC_VER)
#pragma warning(disable : 5033)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wregister"
#endif

/* CB cross-platform read normalization: strip '\r' so the lexer always sees LF.
   Windows fopen("r") already drops it; macOS/Linux "r"=="rb" keeps it, which would
   make every CRLF method body look "changed" on a Mac read-source. Defining
   READ_INPUT here makes flex use ours instead of its default skeleton macro. Keep
   in sync with the mirror prepended in the generated Read.l.cpp. */
#define READ_INPUT(buf,result,max_size) \
    if ( read_current_buffer->read_is_interactive ) \
        { \
        int c; \
        do { c = getc( readin ); } while ( c == '\r' ); \
        result = c == EOF ? 0 : 1; \
        buf[0] = (char) c; \
        } \
    else \
        { \
        if ( ((result = fread( buf, 1, max_size, readin )) == 0) && ferror( readin ) ) \
            READ_FATAL_ERROR( "input in flex scanner failed" ); \
        else \
            { int ri, wi = 0; for ( ri = 0; ri < (int)result; ++ri ) \
                if ( buf[ri] != '\r' ) buf[wi++] = buf[ri]; result = wi; } \
        }

#include <stdlib.h>
#include <string.h>

#include /**/ "Read.y.h"

char yyline[4096];
char yylineUser3[4096];
char yyinterface[4096];
int yylineleng = 0;
int yylineno = 1;
int yyindex = 0;
int yyinterfaceIndex = 0;

enum ModeType {
    NormalMode = 0,
    CommentMode,
    User1Mode,
    User2Mode,
    User3Mode,
    NoteMode,
    CodeMode,
    InitMode,
	DeclarationMode
};

int yymode = 0;

#define DEFAULT {\
    yyline[yyindex++] = yytext[0];\
    yyline[yyindex] = '\0';\
}

#define NEWLINE {\
    yyline[yyindex++] = '\015';\
    yyline[yyindex++] = '\012';\
    yyline[yyindex++] = '\0';\
    yylval.sval = yyline;\
    yylineno++;\
    yyindex = 0;\
    return LINE;\
}
%}

start_user1         "//@START_USER1".*\n
start_user2         "//@START_USER2".*\n
start_user3         "//@START_USER3".*\n
end_user1           "//@END_USER1"
end_user2           "//@END_USER2"
end_user3           "//@END_USER3"
start_note          "/*@NOTE_"[0-9]+.*\n
start_init          "//@INIT_"[0-9]+.*\n
start_code          "{//@CODE_"[0-9]+.*\n
end_code            "}//@CODE_"[0-9]+
start_declaration   "//@START_DECLARATION_"[0-9]*.*\n
end_declaration     "//@END_DECLARATION_"[0-9]*
start_comment       "/*"
end_comment         "*/"

%start normal comment user1 user2 user3 note code init declaration

%%
    {
        switch (ModeType(yymode))
        {
        case NormalMode:
            BEGIN normal;
            break;
        case CommentMode:
            BEGIN comment;
            break;
        case User1Mode:
            BEGIN user1;
            break;
        case User3Mode:
            BEGIN user3;
            break;
        case User2Mode:
            BEGIN user2;
            break;
        case NoteMode:
            BEGIN note;
            break;
        case CodeMode:
            BEGIN code;
            break;
        case InitMode:
            BEGIN init;
            break;
		case DeclarationMode:
		    BEGIN declaration;
			break;
        }
    }
"\015" ;
<normal>{start_comment} {
    yyindex = 0;
    yyline[yyindex++] = '/';
    yyline[yyindex++] = '*';
    BEGIN comment;
    yymode = CommentMode;
    return START_COMMENT;
}
<comment>. DEFAULT
<comment>"\n" NEWLINE
<comment>"*/" {
    yyline[yyindex++] = '*';
    yyline[yyindex++] = '/';
    yyline[yyindex++] = '\0';
    yylval.sval = yyline;
    yyindex = 0;
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_COMMENT;
}

<normal>{start_user1} {
    yylineno++;
    yyindex = 0;
    BEGIN user1;
    yymode = User1Mode;
    return START_USER1;
}
<user1>. DEFAULT
<user1>"\n" NEWLINE
<user1>{end_user1} {
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_USER1;
}

<normal>{start_user2} {
    yylineno++;
    yyindex = 0;
    BEGIN user2;
    yymode = User2Mode;
    return START_USER2;
}
<user2>. DEFAULT
<user2>"\n" NEWLINE
<user2>{end_user2} {
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_USER2;
}

<normal>{start_user3} {
    yylineno++;
    yyindex = 0;
    BEGIN user3;
    yymode = User3Mode;
    return START_USER3;
}
<user3>. DEFAULT
<user3>"\n" NEWLINE
<user3>{end_user3} {
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_USER3;
}

<normal>{start_declaration} {
    yylineno++;
    yyindex = 0;
    yylval.ival = atoi(yytext+21);
    BEGIN declaration;
    yymode = DeclarationMode;
    return START_DECLARATION;
}
<declaration>. DEFAULT
<declaration>"\n" NEWLINE
<declaration>{end_declaration} {
    yylval.ival = atoi(yytext+19);
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_DECLARATION;
}

<normal>{start_note} {
    yylineno++;
    yyindex = 0;
    yylval.ival = atoi(yytext+8);
    BEGIN note;
    yymode = NoteMode;
    return START_NOTE;
}
<note>. DEFAULT
<note>"\n" NEWLINE
<note>"*/" {
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_NOTE;
}

<normal>{start_init} {
    yylineno++;
    yyindex = 0;
    yylval.ival = atoi(yytext+8);
    BEGIN init;
    yymode = InitMode;
    return START_INIT;
}
<init>. DEFAULT
<init>"\n" NEWLINE
<init>{start_code} {
    yylval.ival = atoi(yytext+9);
    BEGIN code;
    yymode = CodeMode;
    return START_CODE;
}

<normal>{start_code} {
    yylineno++;
    yyindex = 0;
    yylval.ival = atoi(yytext+9);
    BEGIN code;
    yymode = CodeMode;
    return START_CODE;
}
<code>. DEFAULT
<code>"\n" NEWLINE
<code>{end_code} {
    yylval.ival = atoi(yytext+9);
    BEGIN normal;
    yymode = NormalMode;
	yyinterfaceIndex = 0;
    return END_CODE;
}

<normal>\n {
    yylineno++;
    yyinterface[yyinterfaceIndex] = '\015';
	yyinterfaceIndex = (yyinterfaceIndex+1)%4096;
    yyinterface[yyinterfaceIndex] = '\012';
	yyinterfaceIndex = (yyinterfaceIndex+1)%4096;
    yyinterface[yyinterfaceIndex] = '\0';
}
<normal>. {
    yyinterface[yyinterfaceIndex] = yytext[0];
	yyinterfaceIndex = (yyinterfaceIndex+1)%4096;
    yyinterface[yyinterfaceIndex] = '\0';
}
%%

int yywrap( )
{
	if (yymode == User3Mode && yyindex)
	{
		yyline[yyindex] = '\0';
		strcpy(yylineUser3, yyline);
	}
	else
		yylineUser3[0] = '\0';

    strcpy( yyline, "" );
    yylineno = 1;
    yyindex = 0;
    yymode = 0;
    
    return 1;
}

