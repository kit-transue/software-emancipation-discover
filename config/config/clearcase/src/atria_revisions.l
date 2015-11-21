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
static char* savever;
static int flag;
main(argc, argv)
 int argc; char**argv;
 {
   flag = (argc != 1);
   savever = NULL;
   yylex();
   if (savever) free(savever);
   exit(0);
 }

%}

%start VER FROM FROMVER SKIP

%%
<VER>[^ @"]+                       {
                                      if (savever) free(savever); 
                                      savever = (char*)strdup(yytext);
                                      BEGIN FROM;
                                   }
<FROM>." from "                    {BEGIN FROMVER;}
<FROM>.                            {printf("%s", savever); BEGIN SKIP;}
<FROMVER>[^ ]+                     {printf("%s", flag ? savever : yytext); BEGIN SKIP;}
<SKIP>.*                           {BEGIN 0;}
.*"version \""[^ @"]+"@@"          {BEGIN VER;}
.                                  {BEGIN SKIP;}
%%


/*
   Filter used to extract version from output of
   "cleartool describe <file> | head -1" command.

   If there are any arguments, the "from" version field will be ignored.

Sample Input from "cleartool describe -long <file> | head -1":
version "mainPropagatorUI.h.C@@/main/jon/CHECKEDOUT" from /main/jon/4 (reserved)
version "logger.h.C@@/main/1"


Sample Output (w/o any arguments):
/main/jon/4
/main/1

Sample Output (with an argument):
/main/jon/CHECKEDOUT
/main/1

*/
