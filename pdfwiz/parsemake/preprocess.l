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
// preprocess.l
// 18.nov.97 kit transue
// preprocessor to preprocess makefile (wrapping escaped lines and removing CRs

// developed using -+ option of flex 2.5.4 (c++ support may change in later revs)

// bug 18504.  Couldn't replicate, but this will certainly fix the issue!
static bool wrote_extra_eof = false;
// end bug 18504.

%}

%option noyywrap
%option yylineno

%x ESCAPE
%%

\\		{ BEGIN ESCAPE; }
<*>\015		{ /* ignore CR */ }
<ESCAPE>\\	{ ECHO; /* remain in escape mode */ }
<ESCAPE>\n	{ BEGIN INITIAL; /* continuation line */ }
<ESCAPE>.	{ (*yyout) << '\\' << yytext[0] ; BEGIN INITIAL; }
\n		{ ECHO; return 1; }
<<EOF>>		{ if (wrote_extra_eof) {return 0;} else { (*yyout) << '\n'; wrote_extra_eof = true; return 1;} }
.		{ ECHO; }
%%
