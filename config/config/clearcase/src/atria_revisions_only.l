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
static char* savedate;
main(argc, argv) 
 int argc; char**argv;
 { savedate = NULL;
   yylex();
   if (savedate) free(savedate);
   exit(0);
 }
%}
word [a-zA-Z0-9_]+
date [^ ]+
d    [0-9]
n    [1-9][0-9]*

%start VERSION REV SKIP

%%
^{date}[ ]*{word}[ ]*	{}
"create version "       {BEGIN VERSION;}
<VERSION>\"[^ ]*"@@"	{BEGIN REV;}
<REV>[^ \"]*  		{ECHO; putchar('\n'); BEGIN SKIP;}
.			{BEGIN SKIP;}
<SKIP>.*$   		{BEGIN 0;}
"\n"                    {}
%%


/*
   Filter used to extract list of versions from output of
   "cleartool lshistory <file>" command

Sample Input from "cleartool lshistory <file>":
28-Apr.11:26   vobadm     checkout version "pset" from /main/4 (unreserved)
26-Apr.18:37   vobadm     create version "pset@@/main/4"
26-Apr.17:36   vobadm     create version "pset@@/main/3"
26-Apr.17:19   vobadm     create version "pset@@/main/2"
26-Apr.17:03   vobadm     create version "pset@@/main/1"
26-Apr.17:02   vobadm     create version "pset@@/main/0"
26-Apr.17:02   vobadm     create branch "pset@@/main"
26-Apr.17:02   vobadm     create file element "pset@@"

Sample Output:
/main/4
/main/3
/main/2
/main/1
/main/0

*/
