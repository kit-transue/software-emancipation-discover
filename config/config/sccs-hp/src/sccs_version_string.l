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

#define yywrap() 1

main()
  {
    yylex();
    exit(0);
  }
%}

%start LETTER VERSION SKIP

%% 
[^ \t]*[ \t]*":"             {BEGIN LETTER;}
<LETTER>[ \t]*[^ \t]*[ \t]*  {BEGIN VERSION;}
<VERSION>[0-9\.]*            {ECHO; printf("\n"); BEGIN SKIP;}
<SKIP>.*                     {BEGIN 0;}
\n                           {BEGIN 0;}
.                            {BEGIN SKIP;}
%%

/*
   Filter used to extract the version string from the output of the sccs prt -y
   command.

Sample Input from "sccs prt -y":
SCCS/s.key.c:   D 1.1   94/09/15 14:26:13 jon   1 0     00026/00000/00000 date and time created 94/09/15 14:26:13 by jon
SCCS/s.one.c:   D 1.5   94/09/09 15:13:21 jon   5 4     00003/00001/00000 
SCCS/s.two.c:   D 1.2   94/09/15 12:02:35 jon   2 1     00000/00000/00000 


Sample Output:
1.1
1.5
1.2

*/
