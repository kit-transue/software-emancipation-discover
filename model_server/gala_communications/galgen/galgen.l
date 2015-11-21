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

#include "galgen.h"

#undef YYLMAX
#define YYLMAX 100000

char *types[100];
int num_types;
extern int yyleng;
extern FILE* fout;

void _token() 
{
  new_token( yyleng );
}

%}

%%

#.*$        { new_line(); fprintf (fout, "%s\n", yytext);}
!.*$        { new_line(); fprintf (fout, "%s\n", yytext+1);}
"![^@]*>"	  {_token(); fprintf (fout, "%s\n", yytext+1);}
\{[^@]*\}	  {_token(); return BODY;}
"@@"        {_token(); return TCLFUNC;}
"@"		      {_token(); return '@';}
"("		      {_token(); return '(';}
")"		      {_token(); return ')';}
","		      {_token(); return ',';}
";"		      {_token(); return ';';}
"*"		      {_token(); return '*';}
"&"		      {_token(); return '&';}
extern[ \t]+[\"]C[\"] _token(); return EXTERNC;
output		  {_token(); return OUTPUT;}
static		  {_token(); return STATIC;}
const		    {_token(); return CONST;}
unsigned	  {_token(); return UNSIGNED;}
service		  {_token(); IF_service_keyword(); return SERVICE;}
datatype	  {_token(); return DATATYPE;}
["A-Za-z_][A-Za-z_0-9:]* {_token(); return istype(yytext) ? TYPE : IDENT;}

[\n\r]   { new_line(); }

[ \t]+	_token();
