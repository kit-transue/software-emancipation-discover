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
#include <ctype.h>

#define yywrap() 1

extern int fun_ind;
extern int no_args;

extern char* args[];
extern char* fun_names[];

extern char * fname;

%}
W       [\n\t ]+
name    [a-zA-Z_][a-zA-Z_0-9]+

%start  NEWLINE ENDLINE FNAME ARGS ENDARGS
%%
      BEGIN NEWLINE;
<NEWLINE>void      {BEGIN FNAME;}
<FNAME>{name}    {fun_names[fun_ind] = fname = strdup(yytext); fun_ind++;}
<FNAME>\(        { no_args = 0; BEGIN ARGS;}
<ARGS>{name}     { args[no_args] = strdup(yytext); ++ no_args; BEGIN ENDARGS;}
<ENDARGS>[^,)]*    {}
<ENDARGS>\,       {BEGIN ARGS;}
<ARGS,ENDARGS>\)  {BEGIN ENDLINE;}
<ENDLINE>\;          {BEGIN NEWLINE; return 1;}
{W}              {}
%%
