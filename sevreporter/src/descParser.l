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

#include <stdio.h>
#include <cstring.h>
#include <descParser.yacc.h>

#ifdef WIN32
#define input input_descParser
#define unput unput_descParser
#endif

static int linenum = 1;

%}

nl              "\n"

%%

"<TEXTMODE>" {
  return TEXT_MODE;
}

"<CODEMODE>" {
  return CODE_MODE;
}

"<INPUTS>" {
  return INPUTS;
}

"<INVENTORY>" {
  return INVENTORY;
}

"<METRICTABLE>" {
  return METRIC_TABLE;
}

"<QUALITYGRAPH>" {
  return QUALITY_GRAPH;
}

"<QUALITYTABLE>" {
  return QUALITY_TABLE;
}

"<QUALITYRESULT>" {
  return QUALITY_RESULT;
}

"<OVERALLQUALITY>" {
  return OVERALL_QUALITY;
}

"<B>" {
  return LINE_END;
}

"<P>" {
  return PARA_END;
}


{nl} {
  linenum++;
  yylval.ch = '\n';
  return PLAINCHAR;
}

. {
  yylval.ch = yytext[0];
  return PLAINCHAR;
}

%%

int yywrap()
{
  return 1;
}

#ifndef _WIN32
int yyerror(msg)
     char *msg;
{
  return fprintf ( stderr, "Item description parsing error: line %d: %s at '%s'\n", linenum, msg, yytext );
}
#else
void yyerror(msg)
     char *msg;
{
  (void) fprintf ( stderr, "Item description parsing error: line %d: %s at '%s'\n", linenum, msg, yytext );
}
#endif

