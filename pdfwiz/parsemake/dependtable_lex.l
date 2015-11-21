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
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <string>
#include <cassert>
#include "dependpath.h"

using namespace std;

static string dependpathpart;      // that funky {$(INCLUDE)}/foo.h that MS provides
static string dependfile;      // we'll build it as we go, concatenating strings w/ quotes

%}

%option noyywrap

%x PATHPART STRING

%%

\{[^}]*		{ // curly braces enclose a dependency path
			BEGIN PATHPART;
			dependpathpart = yytext + 1;
		}

<PATHPART>\}	{
			BEGIN 0;
		}

\"[^"]*		{ // take everything between quotes literally
			BEGIN STRING;
			dependfile += (yytext + 1);
		}

<STRING>\"	{
			BEGIN 0;
		}

[^ \t\"{}\n]+	{ // regular text without any delimiters
			dependfile += yytext;
		}

[ \t\n]+	{ // whitespace means next dependency--add and clear states
			dependpath::globalparser->insert(dependfile, dependpathpart);
			dependpathpart.erase();
			dependfile.erase();
		}

<<EOF>>	{ // end of dependency list
			dependpath::globalparser->insert(dependfile, dependpathpart);
			dependpathpart.erase();
			dependfile.erase();
			return 1;
		}

.		{
			assert(0);
		}
