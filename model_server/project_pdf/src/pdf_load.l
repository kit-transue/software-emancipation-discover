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
#include <pdf_parser.h>

static   long  * ippp, ** iptr;
static YY_CHAR  * cppp, ** cptr;
static long  *size;

static struct flex_state_table * pdf_ptr;

extern FILE *yyin;

void pdf_load_state();

#ifdef _WIN32
#define input() yyinput()
#define unput(c) yyunput(c)
#endif

int yy_pdf_load_wrap()
{
    return 1;
}
%}

int [0-9]+

%s INT CHAR ISIZE CSIZE BSIZE

%%

^"#define YY_END_OF_BUFFER"    { BEGIN  BSIZE;}
<BSIZE>{int}                   {pdf_ptr->yy_END_OF_BUFFER = atoi(yytext);
                                BEGIN 0;} 

^"static const "[a-z]*" int yy_acclist["  {
           iptr = & pdf_ptr->yy_acclist;
           size = & pdf_ptr->yy_acclist_len;
           BEGIN ISIZE;}

^"static const "[a-z]*" int yy_accept["  {
           iptr = & pdf_ptr->yy_accept;
           size = & pdf_ptr->yy_accept_len;
           BEGIN ISIZE;}

^"static const YY_CHAR yy_ec["        {
           cptr = & pdf_ptr->yy_ec;
           size = & pdf_ptr->yy_ec_len;
           BEGIN CSIZE;}


^"static const YY_CHAR yy_meta["        {
           cptr = & pdf_ptr->yy_meta;
           size = & pdf_ptr->yy_meta_len;
           BEGIN CSIZE;}

^"static const "[a-z]*" int yy_base["  {
           iptr = & pdf_ptr->yy_base;
           size = & pdf_ptr->yy_base_len;
           BEGIN ISIZE;}

^"static const "[a-z]*" int yy_def["  {
           iptr = & pdf_ptr->yy_def;
           size = & pdf_ptr->yy_def_len;
           BEGIN ISIZE;}

^"static const "[a-z]*" int yy_nxt["  {
           iptr = & pdf_ptr->yy_nxt;
           size = & pdf_ptr->yy_nxt_len;
           BEGIN ISIZE;}

^"static const "[a-z]*" int yy_chk["  {
           iptr = & pdf_ptr->yy_chk;
           size = & pdf_ptr->yy_chk_len;
           BEGIN ISIZE;}



<ISIZE>{int}  { ippp=*iptr = (long*)malloc(sizeof(int)*(*size=atoi(yytext)));
                  BEGIN INT;}
<CSIZE>{int}  { cppp=*cptr = (YY_CHAR*)malloc(sizeof(YY_CHAR)*(*size=atoi(yytext)));
                  BEGIN CHAR;}

<INT>{int}    {*ippp++ = atoi(yytext);}
<CHAR>{int}   {*cppp++ = atoi(yytext);}

<INT,CHAR>"}" {BEGIN 0;}
. {}
"\n" {}

%%

void pdf_load_state(pdf_p, fn)
  struct flex_state_table * pdf_p;
  FILE* fn;
{
  pdf_ptr = pdf_p;

  yyin = fn;
  yylex();

  pdf_ptr->yy_NO_STATES = pdf_ptr->yy_accept_len - 2;
}
