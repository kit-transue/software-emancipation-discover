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
#define yywrap() 1
#include <string.h>

char * proj, *pn, *ln, *at=0;
char * ptr;
int depth;
char body[100000];

void print_pdf();
void load_cosp();

static int lineno = 1;
static void update_lineno(text)
char * text;
{
  while(*text){
    if(*text == '\n')
          ++ lineno;
    ++ text;
  }
}

main(argc, argv)
     int argc;
     char**argv;
{
   load_cosp(argv[1]);

   while(yylex()){
      print_pdf(proj, pn, 0, ln, at, body, lineno);
      update_lineno(body);
   }
   exit(0);
}

void start_body()
{
  depth = 0;
  ptr = body;
}

void copy_body(ch)
int ch;
{
  *ptr = ch;
  ++ptr;
}

void end_body()
{
  copy_body('\0');
  ptr = body;
}

%}

w    [ \t\n]
name  [^ \t\n\/]+
dir  [^ \t\n]+
att  ("[["[^\[\]]+"]]")
%s PROJ PN LN BODY 

%%
  BEGIN PROJ;
<PROJ>{dir}    {proj = strdup(yytext); BEGIN PN;}
<PN>{w}*[=:]{w}* {update_lineno(yytext);}
<PN>{dir}      {pn = strdup(yytext); BEGIN LN; at=0;}
<LN>{att}+     {at = strdup(yytext);}
<LN>{w}*"<"?[-=]">"?{w}* {update_lineno(yytext);}
<LN>{dir}      {ln = strdup(yytext); start_body(); BEGIN BODY;}
<BODY>"{"      {++depth; copy_body(yytext[0]);}
<BODY>"}"      {--depth; copy_body(yytext[0]); 
                  if(depth==0) {
                     end_body(); 
                     BEGIN PROJ;
                     return 1;}}
<BODY>.        {copy_body(yytext[0]);}
<BODY>"\n"     {copy_body(yytext[0]);}
<PROJ,PN,LN>{w}*  {update_lineno(yytext);}

#.*$          {/* comment */}
. {printf("<%c>", yytext[0]);}
