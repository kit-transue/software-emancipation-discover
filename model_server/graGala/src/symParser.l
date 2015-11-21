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
#ifdef yywrap
#undef yywrap
#endif
#include "symParser.yacc.h"

static   long  * ippp, ** iptr;

int symParser_num;

extern FILE *yyin;

void pdf_load_state();

#ifdef _WIN32
#define input() yyinput()
#define unput(c) yyunput(c)
#endif

int yy_symParser_wrap ()
{
    return 1;
}

#ifdef WIN32
int yy_symparser_wrap ()
{
    return 1;
}
#endif

%}

int ([0-9-]+)
name ([a-zA-Z_]+)
white ([ \t\r])
comment (\/\*.*\*\/)

%s INT CHAR ISIZE CSIZE BSIZE

%%

{int}      { yylval.integer.value = atoi(yytext); return NUMBER; }

"attach"   { return ATTACH; }

"text_mode" { return TEXT_MODE; }

"lineto"    { return LINETO; }

"moveto"    { return MOVETO; }

"ellipse"   { return ELLIPSE; }

"stroke"    { return STROKE; }

"fill"      { return FILL; }

"image"     { return IMAGE; }

"linewidth" { return LINE_WIDTH; }

"color"     { return COLOR; }

"text"      { return TEXT; }

{name}      { yylval.name.address = strdup(yytext);
              return NAME; }

{comment}   {}

{white}     {}

"\n"        {}

.           { return *yytext; }


%%

void grasym_load_defs(fn)
  FILE* fn;
{

#ifdef _WIN32
  YY_INIT;
#endif

  yyin = fn;
  yyparse();
}

#ifdef WIN32
void
#else
int
#endif
yy_symParser_error()
{
   printf("Syntax error %s\n", yytext);
}

#ifdef WIN32
void yy_symparser_error()
{
   printf("Syntax error %s\n", yytext);
}
#endif
