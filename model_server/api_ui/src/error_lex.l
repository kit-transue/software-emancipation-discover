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

#undef YYLMAX
#define YYLMAX 16384

#ifndef yygetc
#undef input
#define input() (*pin ? *pin++ : 0)
#undef unput
#define unput(c) (pin--)
#define YY_INIT
#else
#define yygetc() (*pin ? *pin++ : 0)
#define input() yyinput()
#define unput(c) yyunput(c)
#undef EOF
#define EOF 0
#endif

#define yywrap() 1
#undef output
#define output(c) error_lex_out(c)

extern void error_lex_print();
extern void error_lex_out();
static char* pin;
%}

INCL_START "In file included"
TOP "At top level:\n"
IN_FUN "In "(method|function).*\n
INCL_FROM  " from "  
POS   ([^ \n]+)
POSLAST ([^ \n]+"\n")
%START FUN INC LOCAT

%%
{TOP}                         |
{IN_FUN}                      {BEGIN FUN; error_lex_print (yytext, 0);}
<FUN>{INCL_START}             |
{INCL_START}                  {BEGIN INC; error_lex_print (yytext, 0);}
<INC>{INCL_FROM}              {BEGIN LOCAT; error_lex_print (yytext, 0);}
<LOCAT>{POS}                  {BEGIN INC; error_lex_print (yytext, 1);}
<LOCAT>{POSLAST}              {BEGIN 0; error_lex_print (yytext, 1);}
%%

void error_lex(buffer)
    char* buffer;
{
    pin = buffer;
    YY_INIT;
    yylex();
}
