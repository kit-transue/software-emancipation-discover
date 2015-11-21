/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
%e 2000
%p 5000
%k 5000
%a 5000
%o 6000
%{

#include "smt_spy_parser.h"

#include <string.h>

#include <smtScdTypes.h>

#define out smt_out_stmt_c
#define yylex smt_lex_token_c_internal
#define yyparse smt_shallow_parser_c
#define yyerror smt_yyerror
#define yyexca smt_yyexca
#define yypact smt_yypact
#define yyr1 smt_yyr1
#define yyr2 smt_yyr2
#define yydef smt_yydef
#define yychk smt_yychk
#define yypgo smt_yypgo
#define yyact smt_yyact

char *yylval1;
extern int yylval;
static int mylth;
extern char * smt_lex_input_buffer;
static char * pin;
static int find_end();
static int skip_whites();
extern int is_sp_hdr_esql();
static int seen_eof;
#undef YYLMAX
#define YYLMAX 16384

#ifdef XXX_FIX_LEXER_INPUT
#ifdef yygetc
static int myeof;
#undef yygetc
#define yygetc() (seen_eof+=(*pin==0),*pin & 0x80 ? (pin++, 4) : *pin++)
#define PIN (pin - (yyleng<yy_end+yyeof ? yy_end+yyeof-yyleng : 0))
#define FIND_END(x) myeof=find_end(x) 
#define SKIP_WHITES() myeof=skip_whites()
#define FIX_EOF yyeof=myeof
#undef EOF
#define EOF 0
#define input() yyinput()
#define unput(c) yyunput(c)
#define DIS_NLSTATE do {YY_INIT; --pin;} while(0)
#else
#define YY_INIT NLSTATE
#define DIS_NLSTATE NLSTATE
#undef input
#define PIN pin
#define FIND_END(x) find_end(x)
#define SKIP_WHITES() skip_whites()
#define FIX_EOF
#define input()  ((yylineno += (*pin == '\n' ? 1 : 0)),\
		  *pin & 0x80 ? (pin++, 4) : *pin++) /* "4" is a substitution\
                  for illegal character */
#undef unput
#define unput(c) ((yylineno -= (c == '\n' ? 1 : 0)), pin--)
#endif

#else  // XXX_FIX_LEXER_INPUT
char *PIN;
#define FIX_EOF
#define YY_INIT
#define DIS_NLSTATE
#define SKIP_WHITES()
#define FIND_END(x) find_end(x)
#define yywrap() 0 
int yylval = 0;
#endif  // XXX_FIX_LEXER_INPUT

/* The following macros are used to create SMT tokens and return values to the
 * caller and come in several flavors.  The _N versions of the macros (TOKEN_N,
 * STMT_N, etc.) assume that "find_end" has been called after the token was
 * recognized by lex, and so the length of the token must be calculated to
 * include the extra characters read by find_end; the plain (no _N) versions
 * use the length of the lex-generated token (strlen(yytext)) as the length.
 * To calculate the length, the _N versions use a local variable "cur" which
 * must be set to PIN prior to calling find_end.
 *
 * The "skp" argument to the macros determines whether the token will be placed
 * into the tokenized output to the parser: 1 => no, 0 => yes.
 */

#define TOKEN(skp,type){\
		yylval1 = yytext; if(yytext[0]) {\
                mylth = strlen(yytext);\
                yylval = smt_out_token_c(PIN - smt_lex_input_buffer - mylth,\
                                         mylth, skp, type);} else return(-1);\
                }

#define TOKEN_W(skp,type){\
                char* cur = PIN;\
                SKIP_WHITES();\
                mylth = PIN - cur + strlen(yytext);\
                yylval = smt_out_token_c(PIN - smt_lex_input_buffer - mylth,\
                                         mylth, skp, type);\
                FIX_EOF;\
                }

#define TOKEN_N(skp,type){\
		yylval1 = yytext;\
                mylth = PIN - cur + strlen(yytext);\
                yylval = smt_out_token_c(PIN - smt_lex_input_buffer - mylth,\
                                         mylth, skp, type);\
                }

#define STMT_N(x,skp,type){\
                 char* cur=PIN;FIND_END(0);\
                 TOKEN_N(skp,type);\
                 smt_out_stmt_c(0,x,yylval,yylval,0);\
                 FIX_EOF;\
                }

#define MACR_N(x) STMT_N(x,0,SMTT_macro)

#define MACR_NN(x) STMT_NN(x,0,SMTT_macro)

#define STMT_NN(x,skp,type){\
		  char* cur=PIN;FIND_END(0);\
                  if(*PIN){\
                     TOKEN_N(skp,type);\
                     smt_out_stmt_c(0,x,yylval,yylval,0);\
                     FIX_EOF;\
                  }else{\
		     RET_NN(-1,SMTT_untok);\
		  }\
                } 
                    
#define STMT(x, skp, type){\
                TOKEN(skp, type);\
                smt_out_stmt_c(0, x, yylval, yylval, 0);\
                }

#define COMM(y)  TOKEN(0, y)
#define MACR(x)    STMT(x, 0, SMTT_macro)
#define RET(x, type)  {\
                TOKEN(0, type);\
                return(x);\
                }

#define RET_NN(x, type)  {\
                TOKEN_N(0, type);\
                return(x);\
		}

#define RET_N(x, type)  {unput('\0');\
                TOKEN_N(0, type);\
                return(x);\
                }


%}

ident	([a-zA-Z_$@][a-zA-Z_$@0-9]*)
exp     ([eE][-+]?[0-9]+)
intd    ([0-9]+)
inth    ([0-9a-fA-F]+)
frcn    (([0-9]*"."[0-9]+)|([0-9]+"."))
fltnum  (({frcn}|{intd}){exp}?[FfLl]?)
lngsuf	(([Ll])|([Ll][Ll]))
intsuf  (({lngsuf}?[Uu]?)|([Uu]?{lngsuf}?))
intdec  ([1-9][0-9]*{intsuf})
intoct  ("0"[0-7]*{intsuf})
inthex  ("0"[Xx]{inth}{intsuf})
intnum  ({intdec}|{intoct}|{inthex})
number  ({fltnum}|{intnum})
esc	([\\](a|[^a]))
white   [ \011\013\014\015\n]
whites  ({white}*)
space	[ \11]
spaces	({space}*)
stars	([\*]+)
begccom (\/\*)
cchr    ([^\'\\\n]|{esc})
cchar	([\']{cchr}+[\'])

inc1    (\<([^>\n]*)\>)
inc2    (\"[^\"\n]*\")
/* Empty line				*/
SMTC_el [\001]
/* Marker of separate line comment 	*/
SMTC_nc [\002]
/* Line break (inside statement ...)	*/
SMTC_lb [\003]
%%
{begccom}	            { char ch, *cur = PIN;
                              loop: while ((ch = input()) && ch != '*');
                              if (ch) switch (ch = input()) {
                                       case 0: case '/': break;
                                       case '*': unput('*');
                                       default: goto loop; } 
                              if (ch) {TOKEN_N(0,SMTT_comment);}
                              else {RET_N(-1,SMTT_untok);}}
#{spaces}"include"          MACR_N(SMT_m_include);
#{spaces}"undef"            MACR_N(SMT_m_undef);
#{spaces}"define"           MACR_NN(SMT_m_define);
#{spaces}"if"      	     |
#{spaces}"ifdef"            |
#{spaces}"ifndef"           MACR_N(SMT_m_if);
#{spaces}"else"      	     |
#{spaces}"elif"      	     MACR_N(SMT_m_else);
#{spaces}"endif"      	     MACR_N(SMT_m_endif);
#{spaces}"line"             {char* cur=PIN;FIND_END(0);TOKEN_N(1,0);FIX_EOF;}
#{spaces}"pragma"           |
^#                           MACR_N(SMT_m_pragma);
{white} 		     TOKEN_W(1, 0);

_asm	|
__asm			{char* cur=PIN;FIND_END(1);RET_NN(OTHER, SMTT_kwd);}

\/\/                       {char ch, *cur=PIN; int escaped=0;
                             while(ch=input()) 
                              if(escaped){if(ch=='\r'){if((ch=input())!='\n')unput(ch);}escaped=0;}
                              else if(ch=='\\')escaped=1;
                              else if(ch=='\n')break;
                             if(ch){unput(ch);TOKEN_N(0,SMTT_commentl);}
                             else {RET_N(-1,SMTT_untok);}}
L[\']                   {char* cur=PIN;while(input());RET_N(-1,SMTT_untok);}

"..."                           RET(ELLIPSIS, SMTT_op);
"->"				RET(POINTSAT, SMTT_op);
"<<"				RET(LSHIFT, SMTT_op);
">>"				RET(RSHIFT, SMTT_op);
"--"				RET(MINUSMINUS, SMTT_op);
"++"				RET(PLUSPLUS, SMTT_op);
"&&"				RET(ANDAND, SMTT_op);
"||"				RET(OROR, SMTT_op);
".*"                            RET(DOT_STAR, SMTT_op);
"->*"				RET(ARROW_STAR, SMTT_op);
"<"	|
">"	|
"<="	|
">="				RET(ARITHCOMPARE, SMTT_op);
"=="	|
"!="				RET(EQCOMPARE, SMTT_op);

"*="	|
"/="	|
"%="	|
"+="	|
"-="	|
">>="	|
"<<="	|
"&="	|
"^="	|
"|="	|
"="			RET(ASSGN, SMTT_op);

{number}			RET(CONSTANT, SMTT_const);
{cchar}		   		RET(CONSTANT, SMTT_const);
[\']{cchr}*$                    RET(CONSTANT, SMTT_const);
L[\"]                           |
[\"]     			{char *cur=PIN,ch;int escaped=0;
                                while(ch=input()){
				 if (escaped){escaped=0;continue;}
                                 if(ch=='\\'){escaped=1;continue;}
                                 if(ch=='"'){RET_NN(STRING,SMTT_string);}
                                } RET_N(-1,SMTT_untok);}

L{cchar}		   	RET(CONSTANT, SMTT_const);
L[\']{cchr}*$                   RET(CONSTANT, SMTT_const);

"["				|
"]"				|
";"				|
[.&*+\-~!/%<>^|?:]		RET(*yytext, SMTT_op);
"("				|
")"				|
","				RET(*yytext, SMTT_grouping);
"{"				RET(*yytext, SMTT_begin);
"}"				RET(*yytext, SMTT_end);

"::"				RET(DOUBLE_COLON, SMTT_op);
auto                		RET(SCSPEC, SMTT_kwd)	;
break               		RET(BREAK, SMTT_kwd)	;
case                		RET(CASE, SMTT_kwd)	;
char                		RET(TYPESPEC, SMTT_kwd)	;
const               		RET(TYPE_QUAL, SMTT_kwd)	;
continue            		RET(CONTINUE, SMTT_kwd)	;
default             		RET(DEFAULT, SMTT_kwd)	;
do                  		RET(DO, SMTT_kwd)		;
double              		RET(TYPESPEC, SMTT_kwd)	;
else              		RET(ELSE, SMTT_kwd) 	;
enum                		RET(ENUM, SMTT_kwd)	;
extern              		RET(SCSPEC, SMTT_kwd)	;
float               		RET(TYPESPEC, SMTT_kwd)	;
for                 		RET(FOR, SMTT_kwd)	;
goto                		RET(GOTO, SMTT_kwd)	;
if                		RET(IF, SMTT_kwd)		;
int                 		RET(TYPESPEC, SMTT_kwd)	;
long                		RET(TYPESPEC, SMTT_kwd) 	;
register            		RET(SCSPEC, SMTT_kwd)	;
return              		RET(RETURN, SMTT_kwd)	;
short               		RET(TYPESPEC, SMTT_kwd)	;
signed              		RET(TYPESPEC, SMTT_kwd)	;
sizeof              		RET(SIZEOF, SMTT_kwd)	;
static              		RET(SCSPEC, SMTT_kwd)	;
struct              		RET(AGGR, SMTT_kwd)	;
switch              		RET(SWITCH, SMTT_kwd)	;
typedef             		RET(SCSPEC, SMTT_kwd)	;
union               		RET(AGGR, SMTT_kwd)	;
unsigned            		RET(TYPESPEC, SMTT_kwd)	;
void                		RET(TYPESPEC, SMTT_kwd)	;
volatile            		RET(TYPE_QUAL, SMTT_kwd)	;
while               		RET(WHILE, SMTT_kwd)	;


class               		RET(AGGR, SMTT_kwd)	;
delete              		RET(DELETE, SMTT_kwd)	;
friend              		RET(SCSPEC, SMTT_kwd)	;
inline              		RET(SCSPEC, SMTT_kwd)	;
new                 		RET(NEW, SMTT_kwd)	;
operator            		RET(OPERATOR, SMTT_kwd)	;
overload            		RET(OVERLOAD, SMTT_kwd)	;
protected           		RET(PROTECTED, SMTT_ppp);
private             		RET(PRIVATE, SMTT_ppp)	;
public              		RET(PUBLIC, SMTT_ppp)	;
this                		RET(THIS, SMTT_kwd)	;
virtual             		RET(SCSPEC, SMTT_kwd)	;

END-EXEC			RET(IDENTIFIER, SMTT_ident);

VARCHAR				|
varchar				{if (sp_hdr_is_esql()) {RET(IDENTIFIER, SMTT_macro);} else {RET(IDENTIFIER, SMTT_ident);}}

{ident}				RET(IDENTIFIER, SMTT_ident);
[^a]				RET(OTHER, 0);

%%

int smt_lex_init_c()
  {
  pin = smt_lex_input_buffer;
  YY_INIT;
  yylineno = 1;
  seen_eof = 0;
  }

static int skip_whites()
{
  char ch = 0, prev = 0;
  int done = 0;
  
  do {
    prev = ch;
    ch=input();
  } while(ch==' ' || ch=='\n' || ch=='\011' || ch=='\013' || ch=='\015');

  unput(ch);
  if (prev == '\n')
    {
      DIS_NLSTATE;
    }
  return ch==0;
}

static int find_end(stringless_asm)
/* 970121: This parameter was added to support Microsoft-style asm
   directives.  It turns out that a non-brace-enclosed stringless
   asm does not go to the end of the line, but will be terminated
   by the '}' at the end of the containing block.  The change
   below implements that policy when find_end is invoked in such
   a case.
 */
     int stringless_asm;
{
    char ch;
    int escaped = 0;
    int inside_string = 0;
    while ((ch=input()) && ch != '\n') {
          switch (ch) {
              case '\\':
                if (!escaped) {
                    int num_cr = 0;
                    escaped = 1;
                    while((ch=input())=='\r')
                      num_cr++;
                    if (ch != '\n') {
		      unput(ch);
                      while(num_cr--)
                        unput('\r');
                    } else {
                        escaped = 0;
                    }
                } else {
                    escaped = 0;
                } 
                break;

              case '"':
                if (!escaped)
                   inside_string = !inside_string;
                escaped = 0;
                break;

              case '/':
                if (!inside_string) {
                    ch = input();
                    if (ch == '/')
                        goto ending;
                    if (ch == '*') {
                        loop1: while ((ch=input()) && ch != '*');
                        if (ch) switch (ch = input()) {
                                   case 0: case '/': break;
                                   case '*': unput('*');
                                   default: goto loop1; 
                                } 
                        if (ch == 0)
                            unput (ch);
                    } else {
                      unput (ch);
                    }
                 } 
                 escaped = 0;
                 break;

	      case '}':
		if (stringless_asm) goto done;
		/* FALLTHROUGH! */

               default:
                 escaped = 0;
                 break;
           }
           continue;

      ending:
        while ((ch=input()) && ch != '\n') {
           switch (ch) {
              case '\\':
              {
                int num_cr=0;
                while((ch=input())=='\r')
                  num_cr++;
                if (ch != '\n') {
                   unput(ch);
                   while(num_cr--)
                     unput('\r');
                }
              } 
	      break;
              default: break;
           }
        }
        break;
     }
  done:
     unput (ch);
     return ch==0;
}

int smt_lex_token_c()
{
  int retval = -1;
  if (!seen_eof)
     retval = smt_lex_token_c_internal();
  return retval;
}
