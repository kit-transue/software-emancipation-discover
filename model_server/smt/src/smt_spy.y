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
#define YYDEBUG 1        
#include "smtScdTypes.h" 
#ifdef _WIN32
#include "stddef.h"
#endif


#define out smt_out_stmt_c
#define yylex smt_lex_token_c
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



static int els;
%}

%start program

%token OTHER MERR
%token EXTERN DOUBLE_COLON IDENTIFIER SCSPEC TYPESPEC TYPE_QUAL CONSTANT 
%token STRING ELLIPSIS

%token SIZEOF ENUM IF ELSE WHILE DO FOR SWITCH CASE DEFAULT
decltoken BREAK
 CONTINUE RETURN GOTO ASM TYPEOF ALIGNOF ATTRIBUTE
%token AGGR DELETE NEW OVERLOAD PRIVATE PUBLIC PROTECTED THIS OPERATOR
%token DYNAMIC EMPTY

%token RANGE PLUSPLUS MINUSMINUS LSHIFT RSHIFT ARITHCOMPARE EQCOMPARE
%token MIN_MAX ANDAND OROR POINTSAT ASSGN DOT_STAR ARROW_STAR
%nonassoc IF
%nonassoc ELSE
%%

program: 	extdefs 
		;

extdefs:	extdef | extdefs extdef
		;

extdef:	  fndef				{out(2, SMT_fdef, $1, els);} 
	| expr1 ';'			{out(0, SMT_list_decl,  $1, $2);}
	| expr1 '=' '{' mbi '}' ';'	{out(0, SMT_list_decl,  $1, $6);}
	| ';'				{out(0, SMT_empty,  $1, $1);}
        | comp_stmt
        | stmt
        | extdef | stmt
        | error
	;

mbi:
	| ini
	;

ini:	expr1
	| '{' mbi '}'
	| mbi ',' mbi
	;

fndef:	  expr1 comp_stmt
	;

comp_stmt:
	  '{' comp_stmt1 '}'		{out(2, SMT_block, $1, $3);
					 els = $3;}
	| '{' '}'			{out(2, SMT_block, $1, $2);
					 els = $2;}
	;

comp_stmt1:
	  comp_stmt2
	| expr1
	;

comp_stmt2:
	  stmt
	| comp_stmt1 stmt
	;

switch_body:
  	  '{' switch_body_1 '}'		{out(2, SMT_block, $1, $3);
					 els = $3;}

switch_body_1:
	  case_clause			{out(2, SMT_case_clause, $1, els);}
	| switch_body_1 case_clause	{out(2, SMT_case_clause, $2, els);}

case_clause:
	  case_title 			{out(0, SMT_title, $1, els);}
	| case_clause stmt

case_title:
	  case_title1
	| case_title case_title1

case_title1:
	  CASE expr1 ':'		{els = $3;}
	| CASE expr1 RANGE expr1 ':'	{els = $5;}
        | DEFAULT ':'			{els = $2;}



stmt:
	  stmt_non_if
	| stmt_complex_if

stmt_non_if:
	  comp_stmt
  	| switch_body
	| expr1 ';'			{out(0, SMT_stmt, $1, $2);
					 els = $2;}
	| WHILE exprpar stmt		{out(2, SMT_title, $1, $3 - 1);
					 out(2, SMT_nstdloop,  $1, els);}
	| DO stmt WHILE exprpar ';'	{out(2, SMT_title, $1, $1);
					 out(2, SMT_title, $3, $5);
					 out(2, SMT_nstdloop,$1, $5);
					 els = $5;}
	| SWITCH  exprpar stmt		{out(2, SMT_switch,  $1, els);}
	| BREAK ';'			{out(2, SMT_stmt,  $1, $2);
					 els = $2;}
	| CONTINUE ';'			{out(2, SMT_stmt,  $1, $2);
					 els = $2;}
	| RETURN ';'			{out(2, SMT_stmt,  $1, $2);
					 els = $2;}
	| RETURN expr1 ';'		{out(2, SMT_stmt,  $1, $3);
					 els = $3;}
	| GOTO IDENTIFIER ';'		{out(2, SMT_goto,  $1, $3);
					 els = $3;}
	| IDENTIFIER ':'		{out(2, SMT_label,  $1, $2);}
	| ';'				{out(2, SMT_empty,  $1, $1);
					 els = $1;}
	| FOR '(' mbexpr1 ';' mbexpr1 ';' mbexpr1 ')' stmt
					{out(2, SMT_title, $1, $8);
					 out(2, SMT_nstdloop,  $1, els);}
	| expr1 exprpar comp_stmt 	{out(2, SMT_title, $1, $3 - 1);
					 out(2, SMT_nstdloop,$1,els);}
	;

stmt_complex_if:
	  stmt_complex_if1 %prec IF	{out(2, SMT_if, $1, els);}
	| stmt_complex_if1 ELSE stmt_non_if {out(2, SMT_if, $1, els);
					 out(2, SMT_else_clause, $2, els);}

stmt_complex_if1:
	  simple_if			{out(2, SMT_then_clause, $1, els);}
	| stmt_complex_if1 ELSE simple_if 
					{out(2, SMT_else_if_clause, $2, els);}

simple_if:
	  IF exprpar stmt		
	;

mbexpr1:
	| expr1
	;

expr1:	
	exprpar
  	| expr2a 
	;	

expr2:	
	| expr2a
	;

exprpar:
	  '(' expr2 ')'
	| '(' exprpar ')'
	;

expr2a:   expr_el
	| expr1 oper3 expr1
	| expr1 '?' expr1 ':' expr1
	| expr1 expr1
	| mbexpr1 '[' mbexpr1 ']'
	;


expr_el:
	  expr_w1
	| operator
	| oper2
	;

expr_w1:
	  expr_word
	| CONSTANT
	| STRING
	;

expr_word:
	  ENUM
	| EXTERN
	| IDENTIFIER
	| SCSPEC
	| TYPESPEC
	| TYPE_QUAL
	| SIZEOF
	| TYPEOF
	| ALIGNOF
	| ATTRIBUTE
	| OVERLOAD
	| PRIVATE
	| PUBLIC
	| PROTECTED
	| THIS
	| OPERATOR
	| DYNAMIC
	| AGGR
	;

operator:
	  PLUSPLUS
	| MINUSMINUS
	| '+'
	| '-'
	| '*'
	| '/'
	| '%'
	| LSHIFT
	| RSHIFT
	| ARITHCOMPARE
	| EQCOMPARE
	| MIN_MAX
	| '&'
	| '|'
	| '^'
	| ANDAND
	| OROR
	| DOUBLE_COLON
	| '~'
	| '!'
	;

oper2:
	  '.'
	| NEW
	| POINTSAT
	| DELETE
	| ','
	;

oper3:
	  ASSGN
	;
	
%%


yyerror(s)
  char * s;
  {
  return;
  }

int yywrap()
  {
  return 1;
  }
