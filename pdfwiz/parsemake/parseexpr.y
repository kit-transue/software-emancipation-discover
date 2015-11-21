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
/* parseexpr.y */
/* 21.Oct.97 Kit Transue */
/* to evaluate conditional expresssions in !IF clauses of MS extensions to makefiles */

%{
#include <malloc.h>
#include <stdio.h>
#include <string.h>

int parseexpr_y_retval;
%}

%union {
	int intval;
	char * stringval;
}
%token NL
%token <intval> NUMBER
%token <stringval> STRING
%token EQ
%token NE
%token LT
%token GT
%token LE
%token GE
%token AND
%token OR

%left '-' '+'
%left '*' '/'
%left EQ NE LT GT LE GE
%left AND OR
%nonassoc UMINUS

%type <intval> expression
%type <intval> boolean

%%
statement : expression {
		parseexpr_y_retval = $1;
	}
	;

expression: expression '+' expression	{ $$ = $1 + $3; }
	|	expression '-' expression	{ $$ = $1 - $3; }
	|	expression '*' expression	{ $$ = $1 * $3; }
	|	expression '/' expression	{ $$ = $1 / $3; }
	|	'-' expression %prec UMINUS	{ $$ = - $2; }
	|	'(' expression ')'	{ $$ = $2; }
	|	NUMBER
	|	boolean
	|	expression AND expression { $$ = $1 && $3; }
	|	expression OR expression { $$ = $1 || $3; }
	;

boolean: expression EQ expression
	|	expression NE expression { $$ = $1 != $3; }
	|	expression LT expression { $$ = $1 < $3; }
	|	expression GT expression { $$ = $1 > $3; }
	|	expression LE expression { $$ = $1 <= $3; }
	|	expression GE expression { $$ = $1 >= $3; }
	|	STRING EQ STRING {
			$$ = !strcmp($1, $3);
			free($1);
			free($3);
		}
	|	STRING NE STRING {
			$$ = strcmp($1, $3);
			free($1);
			free($3);
		}
	;
