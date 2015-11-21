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
static flag;
main(argc, argv) 
 int argc; char**argv;
 { flag = argc==1; yylex(); exit(0);}
%}
word [a-zA-Z0-9_]+
d    [0-9]
n    [1-9][0-9]*

%start DOTS REV DATE

%% 
^-[ \t-]$* {BEGIN DOTS;}
<DOTS>^[Rr]evision[ ]   {BEGIN REV;}
<REV>({n}\.)+{n}     {ECHO; putchar(' ');}
<REV>^date":"        {BEGIN DATE;}
<DATE>[0-9/]+        {char*p =yytext; 
                       if(flag) 
                              printf("%5s/%c%c", p+5, p[2], p[3]);
                       else
                              ECHO; 
                       putchar(' ');
                     }
<DATE>[0-9:]+        {ECHO; putchar('\n'); BEGIN 0;}
.    {}
"\n" {}
%%
