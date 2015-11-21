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
%{
    
#include <string.h>
#include <stdio.h>

extern FILE *yyin, *fout;
extern char yytext[];

typedef union {
    int i;
    char *s;
} YYSTYPE;

#include "galgen.h"

extern int ret_status;
extern char *temp1, *temp2;

void y_gen_Tcl( body )
char* body;
{
  gen_Tcl( body);
}

void y_gen_caller( body )
char* body;
{
  gen_caller( body);
}

void y_gen_callee( body )
char* body;
{
  gen_callee( body);
}

%}


%start program

%token CONST UNSIGNED EXTERNC
%token TCLFUNC
%token CHAR SHORT INT LONG FLOAT DOUBLE VSTR
%token IDENT BODY SERVICE ASYNC VOID STATIC
%token DATATYPE TYPE OUTPUT

%%

program:
                | program STATIC IDENT {temp1 = strdup(yytext);} IDENT {temp2 = strdup(yytext);} ';'
                    {fprintf (fout, "static %s %s;", temp1, temp2);}
                | program DATATYPE IDENT 
					{maketype (yytext,0);typename_print_converters(yytext);} ';'
				| program '@' decl 
                | program TCLFUNC Tcldecl
                | SERVICE IDENT {set_service_name(yytext);
                                 fprintf (fout, "static Application* %s_app;\n", get_service_name() );}
		;
Tcldecl:	externtype IDENT {init_args(); set_args_name(yytext); inc_argcount();}
                   body {y_gen_Tcl($4);}
                ;

decl:		externtype {init_args();} type IDENT {set_args_name(yytext); inc_argcount(); } '(' args ')' body {y_gen_caller($9);y_gen_callee($9);}
                   
                ;

body:
                | BODY {$$=strdup(yytext);}
	        | ';' {$$=strdup(";");}
				;

unsigned:	{$$=0;}
		| UNSIGNED {$$=1;} ;

externtype:     {set_exttype("");}
                | EXTERNC {set_exttype("extern \"C\" ");}
                ;

const:          {$$=0;}
                | CONST {$$=1;set_args_isconst();}
                ;

static:         {$$=0;}
                | STATIC {$$=1;set_args_isstatic();}
                ;

output:         {$$=0;}
                | OUTPUT {$$=1;set_args_output();}
                ;

type:		output const static unsigned rawtype 
		;

rawtype:	TYPE {set_args_typename(yytext);} 
		ptrs {typename_fix (get_args());}
		;


ptrs:			
			| ptrs '*' {set_args_ptr();}
			| ptrs '&' {set_args_ref();}
			;		
args:		
                | type ptrs IDENT {set_args_name(yytext);typename_fix(get_args());inc_argcount();}

		| args ',' type ptrs IDENT {set_args_name(yytext);typename_fix(get_args());inc_argcount();} 
		;
%%


yyerror()
{
    fprintf (stderr, "error\n");
    ret_status = 1;
}

yywrap()
{
    IF_eof();
    return 1;
}

void set_in( FILE* fin)
{
  yyin = fin;
}

int  do_parse()
{
  return yyparse();
}

